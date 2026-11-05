#!/usr/bin/env python3
"""Install, verify and package UniConv with only Python's standard library."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shlex
import shutil
import subprocess
import tarfile
import tempfile


ROOT = Path(__file__).resolve().parents[2]


def run(*args, **kwargs):
    print("+ " + shlex.join(str(arg) for arg in args), flush=True)
    subprocess.run([str(arg) for arg in args], check=True, **kwargs)


def git(path, *args):
    return subprocess.check_output(
        ["git", "-C", str(path), *args], text=True
    ).strip()


def cache_values(build):
    values = {}
    for line in (build / "CMakeCache.txt").read_text(encoding="utf-8").splitlines():
        if line and not line.startswith(("#", "//")) and "=" in line:
            key, value = line.split("=", 1)
            values[key.split(":", 1)[0]] = value
    if Path(values["CMAKE_HOME_DIRECTORY"]).resolve() != ROOT:
        raise ValueError("The build directory belongs to a different source tree")
    return values


def enabled(values, key):
    return values.get(key, "").upper() in {"ON", "YES", "TRUE", "1"}


def package_version(values, requested):
    version = requested or values["CMAKE_PROJECT_VERSION"]
    if not re.fullmatch(r"\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?", version):
        raise ValueError("Invalid package version")
    if version.split("-", 1)[0] != values["CMAKE_PROJECT_VERSION"]:
        raise ValueError("Package version must match the CMake project version")
    return version


def dependency_sources(build, values):
    # FetchContent overrides also allow the same script to verify offline builds.
    sources = {}
    names = ["libiconv", "phmap", "googletest"]
    if enabled(values, "UNICONV_USE_SIMDUTF"):
        names.append("simdutf")
    for name in names:
        override = values.get("FETCHCONTENT_SOURCE_DIR_" + name.upper())
        default = Path(values.get("FETCHCONTENT_BASE_DIR", build / "_deps")) / (name + "-src")
        path = Path(override) if override else default
        if name == "phmap" and values.get("PHMAP_INCLUDE_DIR"):
            candidate = Path(values["PHMAP_INCLUDE_DIR"])
            if (candidate / "parallel_hashmap/phmap.h").is_file():
                path = candidate
        if not path.is_dir():
            raise ValueError("Dependency sources are missing: " + str(path))
        sources[name] = path.resolve()
    return sources


def metadata(build, values, version, sources):
    return {
        "version": version,
        "commit": git(ROOT, "rev-parse", "HEAD"),
        "configuration": "Release",
        "system": platform.system(),
        "architecture": platform.machine(),
        "compiler": Path(values["CMAKE_CXX_COMPILER"]).name,
        "shared": enabled(values, "UNICONV_BUILD_SHARED"),
        "simdutf": enabled(values, "UNICONV_USE_SIMDUTF"),
        "bundled_static_dependencies": enabled(values, "UNICONV_BUNDLE_DEPS"),
        "msvc_runtime": values.get("CMAKE_MSVC_RUNTIME_LIBRARY", ""),
        "macos_deployment_target": values.get("CMAKE_OSX_DEPLOYMENT_TARGET", ""),
        "dependencies": {name: git(path, "rev-parse", "HEAD") for name, path in sources.items()},
    }


def checksum(archive):
    digest = hashlib.sha256()
    with archive.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    archive.with_name(archive.name + ".sha256").write_text(
        digest.hexdigest() + "  " + archive.name + "\n", encoding="utf-8"
    )
    print("Created " + str(archive), flush=True)


def add_notices(prefix, sources):
    notices = prefix / "licenses"
    notices.mkdir()
    for name in ["LICENSE", "COPYING.LIB", "THIRD_PARTY_LICENSES.md"]:
        shutil.copy2(ROOT / name, notices / name)
    licenses = {"libiconv": ["COPYING", "COPYING.LIB"], "phmap": ["LICENSE"],
                "simdutf": ["LICENSE-APACHE", "LICENSE-MIT"]}
    for name, files in licenses.items():
        if name in sources:
            folder = notices / name
            folder.mkdir()
            for filename in files:
                shutil.copy2(sources[name] / filename, folder / filename)
    shutil.copy2(ROOT / "doc/CI_and_Releases.md", prefix / "README.md")


def package(args, build, values, version, sources):
    systems = {"Linux": "linux", "Windows": "windows", "Darwin": "macos"}
    arches = {"x86_64": "x64", "AMD64": "x64", "arm64": "arm64", "aarch64": "arm64"}
    actual = systems[platform.system()] + "-" + arches[platform.machine()]
    if args.platform != actual:
        raise ValueError("Package platform disagrees with the build host: " + actual)
    shared = enabled(values, "UNICONV_BUILD_SHARED")
    if values["CMAKE_BUILD_TYPE"] != "Release":
        raise ValueError("Release packages require CMAKE_BUILD_TYPE=Release")
    if not shared and not enabled(values, "UNICONV_BUNDLE_DEPS"):
        raise ValueError("Release static packages must bundle dependencies")
    variant = ("shared" if shared else "static") + ("-simd" if enabled(values, "UNICONV_USE_SIMDUTF") else "-scalar")
    name = "UniConv-" + version + "-" + actual + "-" + variant
    args.output_dir.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="uniconv-package-", dir=build) as temp:
        work = Path(temp)
        installed = work / "original-prefix"
        prefix = work / "relocated package" / name
        run("cmake", "--install", build, "--config", "Release", "--prefix", installed)
        prefix.parent.mkdir()
        installed.rename(prefix)
        env = os.environ.copy()
        runtime_dirs = [prefix / "bin/Release", prefix / "bin", prefix / "lib/Release", prefix / "lib"]
        for variable in ["PATH", "LD_LIBRARY_PATH", "DYLD_LIBRARY_PATH"]:
            env[variable] = os.pathsep.join(str(p) for p in runtime_dirs) + os.pathsep + env.get(variable, "")
        for standard in [17, 20]:
            consumer = work / ("consumer-" + str(standard))
            run("cmake", "-S", ROOT / "tests/install_consumer", "-B", consumer, "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_CXX_STANDARD=" + str(standard),
                "-DCMAKE_CXX_COMPILER=" + values["CMAKE_CXX_COMPILER"],
                "-DCMAKE_MSVC_RUNTIME_LIBRARY=" + values.get("CMAKE_MSVC_RUNTIME_LIBRARY", "MultiThreadedDLL"),
                "-DCMAKE_PREFIX_PATH=" + str(prefix),
                "-DUniConv_DIR=" + str(prefix / "lib/cmake/UniConv"),
                "-DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF")
            run("cmake", "--build", consumer, "--config", "Release", "--parallel", "2")
            run("ctest", "--test-dir", consumer, "-C", "Release", "--output-on-failure",
                "--timeout", "30", "--no-tests=error", env=env)

        # Upstream .pc files embed the original prefix. Keep those relocatable too.
        for pc in prefix.rglob("*.pc"):
            text = pc.read_text(encoding="utf-8")
            text = re.sub(r"^prefix=.*$", "prefix=${pcfiledir}/../..", text, flags=re.MULTILINE)
            pc.write_text(text, encoding="utf-8")
        add_notices(prefix, sources)
        (prefix / "build-info.json").write_text(
            json.dumps(metadata(build, values, version, sources), indent=2) + "\n", encoding="utf-8"
        )
        if platform.system() == "Windows":
            archive = Path(shutil.make_archive(str(args.output_dir / name), "zip", prefix.parent, name))
        else:
            archive = args.output_dir / (name + ".tar.gz")
            with tarfile.open(archive, "w:gz") as tar:
                tar.add(prefix, arcname=name)
        checksum(archive)


def source_bundle(args, build, values, version, sources):
    args.output_dir.mkdir(parents=True, exist_ok=True)
    name = "UniConv-" + version + "-sources"
    archive = args.output_dir / (name + ".tar.gz")
    with tempfile.TemporaryDirectory(prefix="uniconv-sources-", dir=build) as temp:
        root = Path(temp) / name
        root.mkdir()
        for label, path in {"": ROOT, **{"deps/" + key: value for key, value in sources.items()}}.items():
            files = subprocess.check_output(["git", "-C", str(path), "ls-files", "-z"]).decode("utf-8").split("\0")
            for filename in filter(None, files):
                source = path / filename
                if source.is_file():
                    target = root / label / filename
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(source, target)
        (root / "build-info.json").write_text(
            json.dumps(metadata(build, values, version, sources), indent=2) + "\n", encoding="utf-8"
        )
        (root / "BUILD_FROM_SOURCE.md").write_text(
            "# Offline source build\n\nRun from this directory (CMake, Ninja and a C++17 compiler required):\n\n"
            "```sh\ncmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release "
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON -DUNICONV_BUILD_TESTS=ON "
            "-DUNICONV_BUNDLE_DEPS=ON -DUNICONV_USE_SIMDUTF=ON "
            "-DFETCHCONTENT_SOURCE_DIR_LIBICONV=\"$PWD/deps/libiconv\" "
            "-DPHMAP_INCLUDE_DIR=\"$PWD/deps/phmap\" "
            "-DFETCHCONTENT_SOURCE_DIR_SIMDUTF=\"$PWD/deps/simdutf\" "
            "-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=\"$PWD/deps/googletest\"\n"
            "cmake --build build --parallel 2\nctest --test-dir build --output-on-failure\n```\n\n"
            "On Windows, use a VS Developer Bash or substitute absolute paths in PowerShell. "
            "Dependency licenses are included alongside their sources. "
            "Build shared libraries by setting UNICONV_BUILD_SHARED=ON and UNICONV_BUNDLE_DEPS=OFF.\n",
            encoding="utf-8"
        )
        with tarfile.open(archive, "w:gz") as tar:
            tar.add(root, arcname=name)
    checksum(archive)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["package", "sources"])
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--version", default="")
    parser.add_argument("--platform", choices=["linux-x64", "windows-x64", "macos-x64", "macos-arm64"])
    args = parser.parse_args()
    args.output_dir = args.output_dir.resolve()
    build = args.build_dir.resolve()
    values = cache_values(build)
    version = package_version(values, args.version)
    sources = dependency_sources(build, values)
    if args.command == "package":
        package(args, build, values, version, sources)
    else:
        source_bundle(args, build, values, version, sources)


if __name__ == "__main__":
    main()
