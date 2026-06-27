#!/bin/bash

#FIXME: THIS is not portable.  Uses UNIX path separators.
# Can meson do this shit?

set -ex

if [ $# -lt 2 ] ; then
    echo "Usage: $0 INPUT_FILE LIBDIR"
    exit 1
fi


helper="./lib/quickplot/misc/quickplotHelp"
output="share/bash-completion/completions/quickplot"

#echo "PWD=$PWD"

echo -e "# This is a generated file\n" > $output

sed $1\
    -e "s/@OPTS@/$(${helper} -O)/g"\
    -e "s!@REL_LIBDIR@!$2!g"\
    -e "s/@OPTS_NOARG@/$(${helper} -w)/g" >> $output

