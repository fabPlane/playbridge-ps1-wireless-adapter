#!/bin/sh
set -eu
cd "$(dirname "$0")"
tmp_dir=$(mktemp -d)
trap 'rm -f "$tmp_dir/pad" "$tmp_dir/pad-pcb" "$tmp_dir/transport"; rmdir "$tmp_dir"' EXIT
"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -I . test.cpp -o "$tmp_dir/pad"
"$tmp_dir/pad"
"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -DPLAYBRIDGE_PROFILE=2 -I . test.cpp -o "$tmp_dir/pad-pcb"
"$tmp_dir/pad-pcb"
"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -Wno-misleading-indentation -fsanitize=address,undefined transport_test.cpp -o "$tmp_dir/transport"
"$tmp_dir/transport"
node web-ui-test.cjs
