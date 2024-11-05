#!/usr/bin/env python3

# Count lines of code in this repo

import os
from sys import exit
from os.path import join, islink
from typing import Tuple, List

valid_extensions = [".c", ".h", ".cpp", ".cc", ".hpp", ".py", ".q", ".qh"]
exclude_dirs = ['./build', './.build', './.git', 'third_party', 'binary_data']
exclude_files = ["package-lock.json"]


def get_sloc(path) -> Tuple[List, int, int]:
    """
    Returns the number of source lines of code in the given path recursively.
    """

    def is_source_line(line) -> bool:
        comment_prefixes = ["//", "/*", "*",
                            "#", "*/", "'''", '"""', '--', '~>']

        line = line.decode().strip()
        if not line:
            return False

        if any([line.startswith(prefix) for prefix in comment_prefixes]):
            return False

        return True

    total_sloc = 0

    file_sloc_map = []
    for dirpath, _, filenames in os.walk(path):
        # Check if the current directory should be excluded by prefix
        if any([dirpath.startswith(exclude_dir) or exclude_dir in dirpath for exclude_dir in exclude_dirs]):
            continue

        for file in filenames:
            # Check if the file should be excluded by name or extension
            if file in exclude_files or not file.endswith(tuple(valid_extensions)):
                continue

            # Check if is symlink
            if islink(join(dirpath, file)):
                continue

            try:
                with open(join(dirpath, file), "rb") as f:
                    sloc_in_file = 0
                    for line in f:
                        if is_source_line(line):
                            sloc_in_file += 1

                    file_sloc_map.append(
                        (join(dirpath, file), sloc_in_file))
                    total_sloc += sloc_in_file
            except UnicodeDecodeError:
                print(f"UnicodeDecodeError: {join(dirpath, file)}")
                exit(1)

    # Sort by lines of code
    file_sloc_map.sort(key=lambda x: x[1], reverse=False)

    return file_sloc_map, total_sloc


def main():
    sloc_map, sloc = get_sloc(".")

    for file in sloc_map:
        print(f"\x1b[36;49m{file[0]}\x1b[0m: {file[1]}")

    # Format number
    sloc = str(sloc)
    sloc = sloc[::-1]
    sloc = ",".join(sloc[i:i+3] for i in range(0, len(sloc), 3))
    sloc = sloc[::-1]

    print()
    print(f"\x1b[34;49;1mLines of code:\x1b[0m {
          sloc} ({len((sloc_map))} code files)")


if __name__ == "__main__":
    main()
