#!/bin/sh
env_dir=$(readlink -f $(dirname "$0"))/environment
mkdir -p "$env_dir/code/extensions" "$env_dir/code/data"
executable=/usr/share/codium/codium
if test -f /usr/local/share/code-oss/code-oss; then
  executable=/usr/local/share/code-oss/code-oss
elif test -f /usr/share/code/code; then
  executable=/usr/share/code/code
fi
$executable --ms-enable-electron-run-as-node .
#  --disable-gpu --extensions-dir "$env_dir/code/extensions" --user-data-dir "$env_dir/code/data"
