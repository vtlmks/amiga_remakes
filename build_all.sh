#!/usr/bin/env bash
set -e
ROOT_DIR="$(dirname "$0")"
REMAKES_DIR="$ROOT_DIR/remakes"
BIN_DIR="$ROOT_DIR/bin"

# Create bin directory if it doesn't exist
mkdir -p "$BIN_DIR"

pids=()
for dir in "$REMAKES_DIR"/*/; do
	dirname=$(basename "$dir")

	# Skip template directory
	if [ "$dirname" = "template" ]; then
		echo "Skipping template directory"
		continue
	fi

	if [ -x "$dir/build.sh" ]; then
		echo "Building $dirname..."
		(
			cd "$dir"
			./build.sh "$@"
		) &
		pids+=($!)
	else
		echo "Skipping $dirname (no executable build.sh)"
	fi
done

# wait for all background jobs
fail=0
for pid in "${pids[@]}"; do
	wait "$pid" || fail=1
done

# Copy binaries after all builds complete successfully
if [ $fail -eq 0 ]; then
	echo "Copying binaries to bin directory..."
	for dir in "$REMAKES_DIR"/*/; do
		dirname=$(basename "$dir")

		# Skip template directory
		if [ "$dirname" = "template" ]; then
			continue
		fi

		# Copy Linux binary if it exists
		if [ -f "$dir/remake" ]; then
			cp "$dir/remake" "$BIN_DIR/$dirname"
			echo "Copied $dirname (Linux)"
		fi

		# Copy Windows binary if it exists
		if [ -f "$dir/remake.exe" ]; then
			cp "$dir/remake.exe" "$BIN_DIR/$dirname.exe"
			echo "Copied $dirname.exe (Windows)"
		fi
	done
fi

exit $fail

