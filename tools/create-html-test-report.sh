#!/bin/sh

# Check dependencies
if ! command -v ansi2html > /dev/null; then
    echo "Error: ansi2html is not installed. Please install it."
    exit 1
fi

# Ensure that the user has provided the path to the no3 executable
if [ -z "$1" ]; then
    echo "Usage: $0 <no3-executable>"
    exit 1
fi
EXECUTABLE=$1

$EXECUTABLE dev test --opt="gtest_color=yes" --opt="gtest_brief=0" --opt="gtest_repeat=5"|ansi2html