#!/bin/bash

if [ "$#" -ne 1 ] ; then
	echo "Usage $0 test_dir"
	exit 1
fi

tests=($(find $1 -name "*.c" ))

for t in "${tests[@]}" ; do
	./alex $t 1>"$t.out" 2>"$t.err"
done
