#!/bin/sh

repo_root=$(realpath $(dirname "$0")/..)
clang_format_exe="${CLANG_FORMAT:-clang-format-22}"
num_threads=$(nproc)

# Format everything except the `external` folder
find $repo_root -path "$repo_root/external" -prune -o \
  -regex ".*\.\(h\|cpp\)" -print \
  | xargs -n 1 -P $num_threads $clang_format_exe -i
