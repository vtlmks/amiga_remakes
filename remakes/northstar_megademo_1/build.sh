#!/bin/bash

# Set the project name here
PROJECT_NAME="remake"  # Change this when creating a new remake

GIT_ROOT=$(git rev-parse --show-toplevel)

# Add tools to PATH (bmp2ugg built by bootstrap.sh, at end so local version takes precedence)
export PATH="$PATH:$GIT_ROOT/tools/bin"

# Compiler
CC=gcc
WINCC=x86_64-w64-mingw32-gcc

# Base configuration common to all builds
CFLAGS="-std=gnu99 "
CFLAGS+="-mtune=generic "

CFLAGS+="-fno-math-errno "
CFLAGS+="-fno-non-call-exceptions "
CFLAGS+="-fno-pic "
CFLAGS+="-fno-signaling-nans "
CFLAGS+="-fno-stack-protector "
CFLAGS+="-fno-trapping-math "
CFLAGS+="-fno-unwind-tables "

CFLAGS+="-fcf-protection=none "
CFLAGS+="-ffast-math "
CFLAGS+="-fstrict-aliasing "
CFLAGS+="-fvisibility=hidden "
CFLAGS+="-fwrapv "

CFLAGS+="-no-pie "

CFLAGS+="-Wall "
CFLAGS+="-Wextra "

# CFLAGS+="-Wconversion "
CFLAGS+="-Wshift-overflow=2 "
CFLAGS+="-Wstrict-aliasing=3 "
CFLAGS+="-Wstrict-overflow=5 "
CFLAGS+="-Wstringop-overflow=4 "
CFLAGS+="-Wstringop-truncation "

# CFLAGS+="-Wno-sign-conversion "

CFLAGS+="-Wno-missing-field-initializers "
CFLAGS+="-Wno-trigraphs "
CFLAGS+="-Wno-unused-const-variable "
CFLAGS+="-Wno-unused-function "
CFLAGS+="-Wno-unused-parameter "
CFLAGS+="-Wno-unused-variable "
CFLAGS+="-Wno-write-strings "
CFLAGS+="-Wvla "

CFLAGS+="-U_FORTIFY_SOURCE "


# Enable occationally to check for errors..
# CFLAGS+="-fopt-info-vec-optimized "
# CFLAGS+="-fopt-info-optimized "
# CFLAGS+="-fopt-info-missed "
# CFLAGS+="-fopt-info-loop "
# CFLAGS+="-fanalyzer "

LDFLAGS="-Wl,--gc-sections -Wl,--as-needed "

# Base include paths
INCLUDE_PATHS="-I. -I$GIT_ROOT -I$GIT_ROOT/include -I$GIT_ROOT/platform -I$GIT_ROOT/platform/mkfw"

# Linux-specific includes and libraries
LINUX_CFLAGS="-ffunction-sections -fdata-sections"
LINUX_LDFLAGS="$LDFLAGS"
LINUX_INCLUDE=""
LINUX_LIBS="-lasound -lXi -lX11 -lGL -lm -ldl -pthread "

# Windows-specific includes and libraries
WINDOWS_CFLAGS="-ffunction-sections -D__USE_MINGW_ANSI_STDIO=0 "
WINDOWS_LDFLAGS="$LDFLAGS "
WINDOWS_INCLUDE=""
WINDOWS_LIBS="-lwinmm -lole32 -lmmdevapi -lavrt -lgdi32 -lopengl32 -luuid -mwindows "

# Determine build type-specific flags
BUILD_TYPE=$1

if [ -z "$BUILD_TYPE" ]; then
	BUILD_TYPE="normal"
fi

case "$BUILD_TYPE" in
	"normal")
		CFLAGS+="-ggdb -fno-omit-frame-pointer -O2 "
#  -pg # for gprof
		;;
	"release")
		CFLAGS+="-s -Wl,--strip-all -O2 "
		;;
	"profile")
		CFLAGS+="-O2 -fprofile-generate "
#  -ftest-coverage
		;;
	"profile_release")
		CFLAGS+="-s -Wl,--strip-all -O2 -fprofile-use "
		# CFLAGS+="-O2 -fprofile-use "
		;;
	"debug")
		CFLAGS+="-g -O0 -DDEBUGPRINT "
		# CFLAGS+="-DMKFW_TIMER_DEBUG "
		# tripwires for UB; non-recovering so failures are loud
#		CFLAGS+="-fsanitize=undefined,alignment,shift,signed-integer-overflow,pointer-overflow,object-size,unreachable "
#		CFLAGS+="-fno-sanitize-recover=undefined,alignment,shift,signed-integer-overflow,pointer-overflow,object-size,unreachable "
		LDFLAGS+="-fno-pie -no-pie "
		;;
	"coverage")
		gcov -b -c *.c
		exit 0
		;;

	"clean")
		rm -f *.gcda *.gcno *.gcov perf.data* *.alias *.o config.o ${PROJECT_NAME} ${PROJECT_NAME}.exe viewer viewer.exe
		rm -f data/*.ugg data/*/*.ugg data/*.dat
		exit 0
		;;
	*)
		echo "Unknown build type: $BUILD_TYPE"
		exit 1
		;;
esac

# Make sure compilation stops if any error happens.
set -e

# Process assets
(
	cd data
	# Convert BMPs to UGG format
	[ -n "$(ls *.bmp 2>/dev/null)" ] && bmp2ugg -o . *.bmp || true

	# Process scroller texts
	tr '\r\n' ' ' < ns-md1-p5-scroller1.txt > p5_scrolltext1.dat
	tr '\r\n' ' ' < ns-md1-p5-scroller2.txt > p5_scrolltext2.dat
)

# Build Linux version
(
	$CC $CFLAGS $LINUX_CFLAGS remake.c -o ${PROJECT_NAME} $INCLUDE_PATHS $LINUX_INCLUDE $LINUX_LDFLAGS $LINUX_LIBS
) &

# Build Windows version
(
	$WINCC $CFLAGS $WINDOWS_CFLAGS remake.c -o ${PROJECT_NAME}.exe $INCLUDE_PATHS $WINDOWS_INCLUDE $WINDOWS_LDFLAGS $WINDOWS_LIBS
) &


wait
