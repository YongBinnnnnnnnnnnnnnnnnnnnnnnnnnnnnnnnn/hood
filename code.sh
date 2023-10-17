#!/bin/sh
env_dir=$(readlink -f $(dirname "$0"))/environment
mkdir -p "$env_dir/code/extensions" "$env_dir/code/data"
/usr/local/share/code-oss/bin/code-oss --extensions-dir "$env_dir/code/extensions" --user-data-dir "$env_dir/code/data"
