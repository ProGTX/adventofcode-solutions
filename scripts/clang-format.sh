#!/bin/sh

repo_root=$(realpath $(dirname "$0")/..)
clang_format_exe="${CLANG_FORMAT:-clang-format-17}"
num_threads=$(nproc)

find $repo_root -regex ".*\.\(h\|cpp\)" \
  | xargs -n 1 -P $num_threads $clang_format_exe -i
