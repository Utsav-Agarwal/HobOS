#!/bin/bash

CHECKPATCH_DIR=$1

for i in $(find . -type f \( -name "*.c" -o -name "*.h" \)); 
	do ${CHECKPATCH_DIR}/checkpatch.pl --strict --no-tree -f $i 2> /dev/null
done
