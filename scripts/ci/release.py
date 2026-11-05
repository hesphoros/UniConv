#!/usr/bin/env python3
"""Validate release tags and the complete, checksummed release asset set."""

import argparse
import hashlib
import os
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def validate(tag):
    match = re.fullmatch(r"v((?:0|[1-9]\d*)\.(?:0|[1-9]\d*)\.(?:0|[1-9]\d*))(?:-([0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*))?", tag)
    if not match or any(part.isdigit() and len(part) > 1 and part.startswith("0")
                        for part in (match.group(2) or "").split(".")):
        raise ValueError("Expected vMAJOR.MINOR.PATCH or vMAJOR.MINOR.PATCH-prerelease")
    project = re.search(r"project\(UniConv\s+VERSION\s+(\d+\.\d+\.\d+)",
                        (ROOT / "CMakeLists.txt").read_text(encoding="utf-8"))
    if not project or match.group(1) != project.group(1):
        raise ValueError("Tag version disagrees with project(UniConv VERSION ...)")
    commit = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip()
    tag_commit = subprocess.check_output(
        ["git", "rev-parse", "--verify", "refs/tags/" + tag + "^{commit}"], cwd=ROOT, text=True
    ).strip()
    if tag_commit != commit:
        raise ValueError("Checkout must point to the exact commit referenced by the tag")
    values = {"tag": tag, "version": tag[1:], "commit": commit,
              "prerelease": "true" if match.group(2) else "false"}
    output = "".join(key + "=" + value + "\n" for key, value in values.items())
    print(output, end="")
    if os.environ.get("GITHUB_OUTPUT"):
        with open(os.environ["GITHUB_OUTPUT"], "a", encoding="utf-8") as stream:
            stream.write(output)


def checksums(directory, version):
    if not re.fullmatch(r"\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?", version):
        raise ValueError("Invalid version")
    prefix = "UniConv-" + version + "-"
    expected = {prefix + system + "-" + linkage + "-" + variant + extension
                for system, extension in [("linux-x64", ".tar.gz"), ("windows-x64", ".zip"),
                                          ("macos-x64", ".tar.gz"), ("macos-arm64", ".tar.gz")]
                for linkage in ["static", "shared"] for variant in ["scalar", "simd"]}
    expected.add(prefix + "sources.tar.gz")
    actual = {file.name for file in directory.iterdir() if file.name.endswith((".tar.gz", ".zip"))}
    if actual != expected:
        raise ValueError("Incomplete asset set; missing=" + str(sorted(expected - actual)) +
                         ", unexpected=" + str(sorted(actual - expected)))
    lines = []
    for name in sorted(expected):
        digest = hashlib.sha256()
        with (directory / name).open("rb") as stream:
            for block in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(block)
        line = digest.hexdigest() + "  " + name + "\n"
        if (directory / (name + ".sha256")).read_text(encoding="utf-8") != line:
            raise ValueError("SHA-256 mismatch for " + name)
        lines.append(line)
    (directory / "SHA256SUMS").write_text("".join(lines), encoding="utf-8")
    print("Verified 16 binary packages and one complete source archive")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    tag = sub.add_parser("validate")
    tag.add_argument("--tag", required=True)
    sums = sub.add_parser("checksums")
    sums.add_argument("--directory", type=Path, required=True)
    sums.add_argument("--version", required=True)
    args = parser.parse_args()
    if args.command == "validate":
        validate(args.tag)
    else:
        checksums(args.directory, args.version)


if __name__ == "__main__":
    main()
