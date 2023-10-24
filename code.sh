#!/bin/sh
env_dir=$(readlink -f $(dirname "$0"))/environment
mkdir -p "$env_dir/code/extensions" "$env_dir/code/data"
/usr/local/share/code-oss/code-oss --extensions-dir "$env_dir/code/extensions" --disable-gpu --ms-enable-electron-run-as-node .
# --user-data-dir "$env_dir/code/data"
