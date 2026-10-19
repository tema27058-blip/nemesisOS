#!/usr/bin/env sh
set -eu

make clean
make

printf '\nBuild complete.\n'
printf 'Run with: make run\n'
