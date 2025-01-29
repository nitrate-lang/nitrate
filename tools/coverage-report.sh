#!/bin/sh

if [ -z "$LLVM_PROFILE_FILE" ]; then
    LLVM_PROFILE_FILE=default.profraw
fi

if [ -z "$1" ]; then
    echo "Usage: $0 <path-to-binary>"
    exit 1
fi

PATH_TO_OBJECT=$1

llvm-profdata-18 merge -sparse $LLVM_PROFILE_FILE -o default.profdata || exit 1
llvm-cov-18 report $PATH_TO_OBJECT -instr-profile=default.profdata --use-color || (rm -f default.profdata && exit 1)

rm -f default.profdata
