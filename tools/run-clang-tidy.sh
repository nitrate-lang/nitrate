#!/bin/bash

SOURCE_FOLDERS=(
                "pipeline/libnitrate-core"
                "pipeline/libnitrate-lexer"
                "pipeline/libnitrate-seq/include"
                "pipeline/libnitrate-seq/src"
                "pipeline/libnitrate-parser"
                "pipeline/libnitrate-ir"
                "pipeline/libnitrate-emit"
                "pipeline/libnitrate/src"
                "no3/src"
                )

function check_file {
    clang-tidy -header-filter=. -extra-arg=-std=c++20 "$1"
    if [ $? -ne 0 ]; then
        echo "clang-tidy failed for $1"
        exit 1
    fi
}

for folder in "${SOURCE_FOLDERS[@]}"; do
    find "$folder" -type f -a \( -name '*.cc' -o -name '*.hh' \)  -print0 | while IFS= read -r -d '' file; do
        echo "Checking $file"
        check_file "$file"
    done
done
