#!/bin/bash

# Bootstrap script - run this once after cloning the repository
# This ensures bmp2ugg is available and processes shared platform assets

set -e

GIT_ROOT=$(git rev-parse --show-toplevel)

# Build bmp2ugg
echo "Building bmp2ugg..."
mkdir -p "$GIT_ROOT/tools/bin"
(cd "$GIT_ROOT/tools/bmp2ugg" && gcc -g -O2 -march=native -Wno-unused-result -pthread bmp2ugg.c -I"$GIT_ROOT/include" -o "$GIT_ROOT/tools/bin/bmp2ugg")

# Process shared option selector assets
process_selector() {
	local selector_dir="$GIT_ROOT/platform/$1/data"
	local needs_update=false
	for bmp in "$selector_dir"/*.bmp; do
		[ -f "$bmp" ] || continue
		local ugg="${bmp%.bmp}.ugg"
		[ ! -f "$ugg" ] || [ "$bmp" -nt "$ugg" ] && needs_update=true && break
	done
	if [ "$needs_update" = true ]; then
		echo "Processing selector: $1"
		(cd "$selector_dir" && [ -n "$(ls *.bmp 2>/dev/null)" ] && bmp2ugg -o . *.bmp)
	fi
	return 0
}

process_selector "option_selector_1"
echo "Bootstrap complete. You can now run build_all.sh or individual build scripts."
