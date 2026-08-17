#!/bin/sh

repo_root=$(realpath $(dirname "$0")/..)

# rust/Cargo.toml is a virtual workspace,
# so --all is required to pick up the per-year crates and utils
cargo fmt --all --manifest-path "$repo_root"/rust/Cargo.toml "$@"
