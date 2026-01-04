# Third-party licenses and included components

This project (`UniConv`) includes third-party source code. This file documents
what is included and how to comply with the third-party licenses when you
distribute the project or binaries built from it.

## Included components

- **GNU libiconv** (sources included under `src/`, e.g. `src/iconv.c`,
  `src/localcharset.c`, and related files): licensed under the GNU Lesser
  General Public License (LGPL) version 2.1 or later (see `COPYING.LIB`).

## What this means for you

- The project original code (author `hesphoros`) is released under the MIT
  license (see `LICENSE`). That license applies to files authored by the
  project owner and does not supersede third-party licenses.
- The libiconv sources remain under the LGPL. You must preserve copyright
  and license headers in the libiconv files.
- If you distribute binaries that include the libiconv code (for example,
  statically linked into a single executable), you must comply with the
  LGPL obligations for libiconv:
  - Provide a copy of the LGPL license text (this repository contains
    `COPYING.LIB` as a pointer and guidance).
  - Preserve the original libiconv copyright and license headers in the
    included files.
  - If you modified the libiconv sources, you must provide the modified
    libiconv sources under the LGPL when distributing the binaries.
  - For static linking, you must provide a way for recipients to relink the
    application with a modified version of libiconv (for example, provide
    object files or build scripts), or choose dynamic linking instead.

## Practical options to reduce obligations

1. Use system libiconv at runtime (dynamic linking) instead of embedding the
   code into your binary. Most systems provide libiconv packaged separately.
2. Keep libiconv as a separate build target in your build system so that it
   can be rebuilt independently by downstream users.
3. Remove bundled libiconv and rely on a permissively licensed alternative
   if one exists for your supported platforms.

## Contacts and references

- GNU libiconv: https://www.gnu.org/software/libiconv/
- LGPL v2.1: https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html

If you are preparing redistributions for third-parties or commercial
distributions, consider consulting your legal counsel to ensure full
compliance with the LGPL requirements.
