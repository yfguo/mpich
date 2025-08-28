#! /usr/bin/env bash
##
## Copyright (C) by Argonne National Laboratory
##     See COPYRIGHT in top-level directory
##
## Code cleanup script using clang-format instead of GNU Indent
## This script replaces the functionality of maint/code-cleanup.bash

# Check if clang-format is available
if ! hash clang-format 2>/dev/null; then
    echo "clang-format is required but not found in PATH"
    exit 1
fi

# Check clang-format version (require 21 or later)
clang_format_version=$(clang-format --version | grep -oE 'version [0-9]+' | grep -oE '[0-9]+')
if [ -z "$clang_format_version" ]; then
    echo "Could not determine clang-format version"
    exit 1
fi

if [ "$clang_format_version" -lt 21 ]; then
    echo "This script requires clang-format version 21 or later. Found version: $clang_format_version"
    exit 1
fi

# Find the .clang-format file
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd "$script_dir/.." && pwd)"
clang_format_file="$project_root/.clang-format"

if [ ! -f "$clang_format_file" ]; then
    echo "Error: .clang-format file not found at $clang_format_file"
    exit 1
fi

format_code()
{
    file=$1

    # Create backup
    cp "${file}" "${file}.backup"

    # Apply clang-format
    clang-format -i --style=file --fallback-style=none "${file}"

    # Additional post-processing to match GNU Indent behavior
    # These sed commands replicate the post-processing from the original script
    # temp_file=$(mktemp)
    # cat "${file}" | \
    #     sed -e 's/ *$//g' \
    #         -e 's/( */(/g' \
    #         -e 's/ *)/)/g' \
    #         -e 's/if(/if (/g' \
    #         -e 's/while(/while (/g' \
    #         -e 's/do{/do {/g' \
    #         -e 's/}while/} while/g' \
    #     > "$temp_file" && mv "$temp_file" "${file}"
    #
    # # Remove backup file if formatting succeeded
    # if [ $? -eq 0 ]; then
    rm -f "${file}.backup"
    # else
    #     echo "Error formatting $file, restoring backup"
    #     mv "${file}.backup" "${file}"
    #     return 1
    # fi
}

debug=
run_format()
{
    if test -n "$debug" ; then
        echo format_code $1
    else
        format_code $1
    fi
}

usage()
{
    echo "Usage: $1 [filename | --all] {--recursive} {--debug} {--ignore ignore_patterns}"
    echo "Code cleanup script using clang-format (requires version 21 or later)"
}

# Check usage
if [ -z "$1" ]; then
    usage $0
    exit
fi

# File types to format (same as original script)
filetype_list="\.c$|\.h$|\.c\.in$|\.h\.in$|\.cpp$|\.cpp.in$|\.inc$"

# Default ignore list (same as original script)
ignore_list="doc/"
ignore_list="$ignore_list|src/mpid/ch3/doc"
ignore_list="$ignore_list|src/mpid/ch3/include|src/mpid/ch3/src"
ignore_list="$ignore_list|src/mpid/ch3/util"
ignore_list="$ignore_list|src/mpid/ch3/channels/nemesis/include"
ignore_list="$ignore_list|src/mpid/ch3/channels/nemesis/src"
ignore_list="$ignore_list|src/mpid/ch3/channels/nemesis/utils"
ignore_list="$ignore_list|src/mpi/romio/include/mpiof.h.in"
ignore_list="$ignore_list|test/mpi/errors/f77/io/addsize.h.in"
ignore_list="$ignore_list|test/mpi/errors/f77/io/iooffset.h.in"
ignore_list="$ignore_list|test/mpi/f77/attr/attraints.h.in"
ignore_list="$ignore_list|test/mpi/f77/datatype/typeaints.h.in"
ignore_list="$ignore_list|test/mpi/f77/ext/add1size.h.in"
ignore_list="$ignore_list|test/mpi/f77/io/ioaint.h.in"
ignore_list="$ignore_list|test/mpi/f77/io/iodisp.h.in"
ignore_list="$ignore_list|test/mpi/f77/io/iooffset.h.in"
ignore_list="$ignore_list|test/mpi/f77/pt2pt/attr1aints.h.in"
ignore_list="$ignore_list|test/mpi/f77/rma/addsize.h.in"
ignore_list="$ignore_list|test/mpi/f77/spawn/type1aint.h.in"
ignore_list="$ignore_list|src/include/mpi.h.in"
ignore_list="$ignore_list|src/mpi/romio/include/mpio.h.in"
ignore_list="$ignore_list|src/mpi/romio/adio/include/adioi_errmsg.h"
ignore_list="$ignore_list|src/pmi/include/pmix.h"
ignore_list="$ignore_list|src/pmi/include/pmix_abi_support.h"
ignore_list="$ignore_list|src/pmi/include/pmix_abi_support_bottom.h"
ignore_list="$ignore_list|src/pmi/include/pmix_fns.h"
ignore_list="$ignore_list|src/pmi/include/pmix_macros.h"
ignore_list="$ignore_list|src/pmi/include/pmix_types.h"
ignore_list="$ignore_list|src/binding/abi/mpi_abi.h"

filelist=""

# Parse command line arguments (same logic as original script)
all=0
recursive=0
ignore=0
got_file=0
for arg in $@; do
    if [ "$ignore" = "1" ] ; then
	ignore_list="$ignore_list|$arg"
	ignore=0
	continue;
    fi

    if [ "$arg" = "--all" ]; then
	all=1
    elif [ "$arg" = "--recursive" ]; then
	recursive=1
    elif [ "$arg" = "--debug" ]; then
	debug="echo"
    elif [ "$arg" = "--ignore" ] ; then
	ignore=1
    else
	got_file=1
        filelist="$filelist $arg"
    fi
done

# Validation (same as original script)
if [ "$recursive" = "1" -a "$all" = "0" ]; then
    echo "--recursive cannot be used without --all"
    usage $0
    exit
fi
if [ "$got_file" = "1" -a "$all" = "1" ]; then
    echo "--all cannot be used in conjunction with a specific file"
    usage $0
    exit
fi

# Process files (same logic as original script)
if [ "$recursive" = "1" ]; then
    for i in `git ls-files | egrep "($filetype_list)" | egrep -v "($ignore_list)"` ; do
	run_format $i
    done
elif [ "$all" = "1" ]; then
    for i in `git ls-files | cut -d/ -f1 | uniq | egrep "($filetype_list)" | egrep -v "($ignore_list)"` ; do
	run_format $i
    done
else
    for i in $filelist; do
        filename=`echo $i | egrep "($filetype_list)" | egrep -v "($ignore_list)"`
        if [ "$filename" != "" ] ; then
            run_format $filename
        fi
    done
fi
