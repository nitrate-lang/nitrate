#!/usr/bin/env python3

# Count lines of code in this repo

import os

valid_extensions = [".c", ".h", ".cpp", ".cc", ".hpp", ".py", ".q", ".qh", ".java", ".js", ".ts", ".html", ".tsx", ".jsx", ".json",
                    ".xml", ".yml", ".yaml", ".sh", ".bat", ".cmd", ".ps1", ".css", ".scss", ".sass", ".php", ".go", ".rs", ".cs", ".swift", ".m", ".mm", "go"]
exclude_dirs = ['./docs', './build', './.git', 'node_modules', 'test', "deps"]
exclude_files = ["package-lock.json"]


def get_loc(path) -> (int, int):
    """
    Returns the number of lines of code in the given path.
    """
    loc = 0
    files_count = 0
    file_loc = []
    for root, dirs, files in os.walk(path):
        # Check if the current directory should be excluded by prefix
        skip = False
        for exclude_dir in exclude_dirs:
            if root.startswith(exclude_dir) or exclude_dir in root:
                skip = True
                break
        if skip:
            continue

        for file in files:
            if file in exclude_files:
                continue
            if not file.endswith(tuple(valid_extensions)):
                continue

            # Check if is symlink
            if os.path.islink(os.path.join(root, file)):
                continue
            try:
                with open(os.path.join(root, file), "r") as f:
                    files_count += 1
                    tmp = loc
                    for line in f:
                        tline = line.strip()
                        # Check if line is a comment or empty
                        if tline == "" or tline.startswith("//") or tline.startswith("/*") or tline.startswith("*") or tline.startswith("#"):
                            continue
                        loc += 1
                    file_loc.append((os.path.join(root, file), loc - tmp))
            except UnicodeDecodeError:
                print(f"UnicodeDecodeError: {os.path.join(root, file)}")
                continue

    # Sort by lines of code
    file_loc.sort(key=lambda x: x[1], reverse=False)
    for file in file_loc:
        print(f"\x1b[36;49m{file[0]}\x1b[0m: {file[1]}")

    return loc, files_count


def main():
    loc, count = get_loc(".")
    loc = str(loc)
    # Format with commas
    loc = loc[::-1]
    loc = ",".join(loc[i:i+3] for i in range(0, len(loc), 3))
    loc = loc[::-1]

    print()
    print(f"Lines of code: {loc} ({count} code files)")


if __name__ == "__main__":
    main()
