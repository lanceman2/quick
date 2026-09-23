#!/bin/bash

set -ex
shopt -s nullglob

#Usage: $0 [DIR]

dir=""

[ $# -lt 1 ] || dir="${1}/"

echo ${dir}*.c

