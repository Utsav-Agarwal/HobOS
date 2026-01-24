#!/bin/bash

CHECKPATCH_DIR=$1
CHECKPATCH_EXTRA_FLAGS=$2	#just for sugar, doesnt really matter here

IGNORE_SPACING=${CHECKPATCH_DIR}/ignore_spacing.checkpatch
IGNORE_COMPLEX_MACRO=${CHECKPATCH_DIR}/ignore_complex_macro.checkpatch

# $1: name
# $2: name list file
# $3: ignore flag type
ignore_if_valid() {
	file_name="$1"
	file_name="${file_name#./}"

	if grep -Fxq "$file_name" $2; then
		CHECKPATCH_EXTRA_FLAGS+=" --ignore $3"
	fi
}

for i in $(find . -type f \( -name "*.c" -o -name "*.h" \) -o \( -path *tools/* \) -prune ); do
	
	CHECKPATCH_EXTRA_FLAGS=$2
	
	ignore_if_valid "$i" "$IGNORE_SPACING" "SPACING"
	ignore_if_valid "$i" "$IGNORE_COMPLEX_MACRO" "COMPLEX_MACRO"

	${CHECKPATCH_DIR}/checkpatch.pl --strict --no-tree ${CHECKPATCH_EXTRA_FLAGS} -f $i 2> /dev/null

done
