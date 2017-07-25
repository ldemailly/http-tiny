#!/bin/bash
find . -type f -a \( -name "*.h" -o -name "*.c" \) | xargs clang-format -i
