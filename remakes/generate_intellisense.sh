#!/bin/bash

# Script to generate compile_commands.json and c_cpp_properties.json for all remake projects

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Find all directories with build.sh
for dir in "$SCRIPT_DIR"/*/; do
    # Skip if no build.sh exists
    if [ ! -f "$dir/build.sh" ]; then
        continue
    fi

    # Extract project name from build.sh
    PROJECT_NAME=$(grep '^PROJECT_NAME=' "$dir/build.sh" | sed 's/PROJECT_NAME="\(.*\)".*/\1/')

    if [ -z "$PROJECT_NAME" ]; then
        echo "Warning: Could not find PROJECT_NAME in $dir/build.sh"
        continue
    fi

    DIR_NAME=$(basename "$dir")
    echo "Generating IntelliSense files for: $DIR_NAME (project: $PROJECT_NAME)"

    # Create .vscode directory if it doesn't exist
    mkdir -p "$dir/.vscode"

    # Generate compile_commands.json
    cat > "$dir/compile_commands.json" << 'EOF'
[
	{
		"directory": "DIRECTORY_PLACEHOLDER",
		"command": "gcc -std=gnu11 -mtune=generic -march=x86-64-v3 -fno-argument-alias -mfunction-return=keep -mindirect-branch=keep -fwrapv -ffast-math -fno-trapping-math -fvisibility=hidden -fno-stack-protector -fno-PIE -no-pie -fcf-protection=none -fno-non-call-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -Wall -Wextra -Wstrict-aliasing=3 -Wno-unused-parameter -Wno-sign-compare -Wno-trigraphs -Wno-maybe-uninitialized -Wno-unused-variable -Wno-unused-const-variable -Wno-unused-function -Wno-write-strings -Wno-missing-field-initializers -U_FORTIFY_SOURCE -fno-pic -fno-ident -fno-strict-overflow -fno-delete-null-pointer-checks -Wstrict-overflow=5 -Warray-bounds -Wshift-overflow=2 -Woverflow -Wstringop-overflow=4 -Wstringop-truncation -Wvla -ggdb -fno-omit-frame-pointer -O2 -ffunction-sections -fdata-sections -I. -I/work/current/amiga_remakes -I/work/current/amiga_remakes/include -I/work/current/amiga_remakes/platform -I/work/current/amiga_remakes/platform/mkfw remake.c -o PROJECT_NAME_PLACEHOLDER -Wl,--gc-sections -Wl,--as-needed -lasound -lXi -lX11 -lGL -lm -ldl -pthread",
		"file": "remake.c"
	}
]
EOF

    # Replace placeholders
    sed -i "s|DIRECTORY_PLACEHOLDER|$dir|g" "$dir/compile_commands.json"
    sed -i "s|PROJECT_NAME_PLACEHOLDER|$PROJECT_NAME|g" "$dir/compile_commands.json"

    # Generate c_cpp_properties.json
    cat > "$dir/.vscode/c_cpp_properties.json" << 'EOF'
{
	"configurations": [
		{
			"name": "Linux",
			"includePath": [
				"${workspaceFolder}",
				"${workspaceFolder}/../../include",
				"${workspaceFolder}/../../platform",
				"${workspaceFolder}/../../platform/mkfw"
			],
			"defines": [],
			"compilerPath": "/usr/bin/gcc",
			"cStandard": "gnu11",
			"cppStandard": "gnu++17",
			"intelliSenseMode": "linux-gcc-x64",
			"compileCommands": "${workspaceFolder}/compile_commands.json",
			"browse": {
				"path": [
					"${workspaceFolder}",
					"${workspaceFolder}/../../include",
					"${workspaceFolder}/../../platform",
					"${workspaceFolder}/../../platform/mkfw"
				],
				"limitSymbolsToIncludedHeaders": false
			}
		}
	],
	"version": 4
}
EOF

    echo "  ✓ Created compile_commands.json"
    echo "  ✓ Created .vscode/c_cpp_properties.json"
    echo ""
done

echo "Done! All remake projects now have IntelliSense configuration."
