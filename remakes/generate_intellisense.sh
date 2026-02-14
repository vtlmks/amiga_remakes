#!/bin/bash

# Generates per-remake-directory compile_commands.json and .vscode/c_cpp_properties.json
# for IntelliSense when opening each remake directory as a VSCode workspace.
#
# For unity builds, sub-files (remake_part*.c etc.) get IntelliSense context
# via -include platform.c with the correct BUFFER_WIDTH/HEIGHT defines.
#
# Uses the "arguments" array format (not "command" string) in compile_commands.json
# so IntelliSense correctly parses flags like -include.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GIT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

PLATFORM_C="$GIT_ROOT/platform/platform.c"

# Process each remake directory
for dir in "$SCRIPT_DIR"/*/; do
	if [ ! -f "$dir/build.sh" ]; then
		continue
	fi

	# Skip template
	if [ "$(basename "$dir")" = "template" ]; then
		continue
	fi

	DIR_NAME=$(basename "$dir")
	dir="$(cd "$dir" && pwd)"

	echo "Processing: $DIR_NAME"

	# Extract BUFFER_WIDTH and BUFFER_HEIGHT from remake.c
	BW=$(grep -m1 '#define BUFFER_WIDTH' "$dir/remake.c" | sed 's/#define BUFFER_WIDTH[[:space:]]*//' | tr -d '[:space:]')
	BH=$(grep -m1 '#define BUFFER_HEIGHT' "$dir/remake.c" | sed 's/#define BUFFER_HEIGHT[[:space:]]*//' | tr -d '[:space:]')

	if [ -z "$BW" ]; then BW="346"; fi
	if [ -z "$BH" ]; then BH="270"; fi

	OUTPUT="$dir/compile_commands.json"

	# Common arguments shared by all entries
	COMMON_ARGS="
		\"gcc\",
		\"-std=gnu99\",
		\"-mtune=generic\",
		\"-fno-math-errno\",
		\"-fno-non-call-exceptions\",
		\"-fno-pic\",
		\"-fno-signaling-nans\",
		\"-fno-stack-protector\",
		\"-fno-trapping-math\",
		\"-fno-unwind-tables\",
		\"-fcf-protection=none\",
		\"-ffast-math\",
		\"-fstrict-aliasing\",
		\"-fvisibility=hidden\",
		\"-fwrapv\",
		\"-no-pie\",
		\"-Wall\",
		\"-Wextra\",
		\"-Wshift-overflow=2\",
		\"-Wstrict-aliasing=3\",
		\"-Wstrict-overflow=5\",
		\"-Wstringop-overflow=4\",
		\"-Wstringop-truncation\",
		\"-Wno-missing-field-initializers\",
		\"-Wno-trigraphs\",
		\"-Wno-unused-const-variable\",
		\"-Wno-unused-function\",
		\"-Wno-unused-parameter\",
		\"-Wno-unused-variable\",
		\"-Wno-write-strings\",
		\"-Wvla\",
		\"-U_FORTIFY_SOURCE\",
		\"-ggdb\",
		\"-fno-omit-frame-pointer\",
		\"-O2\",
		\"-ffunction-sections\",
		\"-fdata-sections\",
		\"-I.\",
		\"-I$GIT_ROOT\",
		\"-I$GIT_ROOT/include\",
		\"-I$GIT_ROOT/platform\",
		\"-I$GIT_ROOT/platform/mkfw\""

	FIRST_ENTRY=1

	# Start the JSON array
	echo "[" > "$OUTPUT"

	add_comma() {
		if [ $FIRST_ENTRY -eq 0 ]; then
			echo "," >> "$OUTPUT"
		fi
		FIRST_ENTRY=0
	}

	# Entry for remake.c itself (normal compile, no -include needed)
	add_comma
	cat >> "$OUTPUT" << EOF
	{
		"directory": "$dir",
		"arguments": [
$COMMON_ARGS,
			"remake.c"
		],
		"file": "$dir/remake.c"
	}
EOF

	# Find all other .c files in the directory (sub-files included by remake.c)
	for cfile in "$dir"/*.c; do
		[ "$(basename "$cfile")" = "remake.c" ] && continue
		[ ! -f "$cfile" ] && continue

		BASENAME="$(basename "$cfile")"

		# Sub-files need the framework context via -include
		add_comma
		cat >> "$OUTPUT" << EOF
	{
		"directory": "$dir",
		"arguments": [
$COMMON_ARGS,
			"-DBUFFER_WIDTH=$BW",
			"-DBUFFER_HEIGHT=$BH",
			"-include",
			"$PLATFORM_C",
			"$BASENAME"
		],
		"file": "$dir/$BASENAME"
	}
EOF
	done

	# Close the JSON array
	echo "" >> "$OUTPUT"
	echo "]" >> "$OUTPUT"

	# Generate .vscode/c_cpp_properties.json (only if it doesn't exist)
	VSCODE_DIR="$dir/.vscode"
	CPP_PROPS="$VSCODE_DIR/c_cpp_properties.json"
	if [ ! -f "$CPP_PROPS" ]; then
		mkdir -p "$VSCODE_DIR"
		cat > "$CPP_PROPS" << EOF
{
	"configurations": [
		{
			"name": "Linux",
			"includePath": [
				"\${workspaceFolder}",
				"\${workspaceFolder}/../../include",
				"\${workspaceFolder}/../../platform",
				"\${workspaceFolder}/../../platform/mkfw"
			],
			"defines": [],
			"compilerPath": "/usr/bin/gcc",
			"cStandard": "gnu99",
			"intelliSenseMode": "linux-gcc-x64",
			"compileCommands": "\${workspaceFolder}/compile_commands.json",
			"browse": {
				"path": [
					"\${workspaceFolder}",
					"\${workspaceFolder}/../../include",
					"\${workspaceFolder}/../../platform",
					"\${workspaceFolder}/../../platform/mkfw"
				],
				"limitSymbolsToIncludedHeaders": false
			}
		}
	],
	"version": 4
}
EOF
		echo "  Created: $CPP_PROPS"
	fi

	# Generate .vscode/settings.json (only if it doesn't exist)
	SETTINGS="$VSCODE_DIR/settings.json"
	if [ ! -f "$SETTINGS" ]; then
		mkdir -p "$VSCODE_DIR"
		cat > "$SETTINGS" << EOF
{
	"files.associations": {
		"*.s": "m68k",
		"*.rh": "c"
	},
	"files.exclude": {
		"**/.profile_viewer*": true,
		"**/.remake_*": true,
		"**/remake": true,
		"**/viewer": true,
		"**/*.exe": true
	}
}
EOF
		echo "  Created: $SETTINGS"
	fi
done

echo ""
echo "Done! Generated compile_commands.json in each remake directory."
