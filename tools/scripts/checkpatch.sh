#!/bin/bash

CHECKPATCH_DIR=$1
CHECKPATCH_EXTRA_FLAGS=$2

for i in $(find . -type f \( -name "*.c" -o -name "*.h" \) -o \( -path */tools/* \) -prune ); do 
	${CHECKPATCH_DIR}/checkpatch.pl --strict --no-tree ${CHECKPATCH_EXTRA_FLAGS} -f $i 2> /dev/null
done
