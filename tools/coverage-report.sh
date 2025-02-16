#!/bin/sh

EXECUTABLES=`find ./build/bin -type f -executable`
SHARED_LIBS=`find ./build/lib -type f -name "*.so"`
OBJECTS="$EXECUTABLES $SHARED_LIBS"

if [ -z "$LLVM_PROFILE_FILE" ]; then
    LLVM_PROFILE_FILE=default.profraw
fi

llvm-profdata-18 merge -sparse $LLVM_PROFILE_FILE -o default.profdata
if [ $? -ne 0 ]; then
    echo "Failed to merge profile data"
    rm -f default.profdata
    exit 1
fi

mkdir -p coverage

for OBJECT in $OBJECTS; do
  FILENAME=`basename $OBJECT`
  llvm-cov-18 export $OBJECT --format=lcov -instr-profile=default.profdata > coverage/$FILENAME.dat
  if [ $? -ne 0 ]; then
    echo "Failed to generate coverage report"
    rm -f default.profdata
    exit 1
  fi
done

rm -f default.profdata

echo "Complete"