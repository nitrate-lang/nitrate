#!/bin/bash

SOURCE_FOLDERS=("pipeline/libnitrate-core/src")

function check_file {
    clang-tidy -header-filter=. -extra-arg=-std=c++20 "$1"
    if [ $? -ne 0 ]; then
        echo "clang-tidy failed for $1"
        exit 1
    fi
}

for folder in "${SOURCE_FOLDERS[@]}"; do
    find "$folder" -type f -name "*.cc" -print0 | while IFS= read -r -d '' file; do
        check_file "$file"
    done
done