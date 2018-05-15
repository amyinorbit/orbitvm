#!/bin/sh
# Runs the orbit compiler (orbitc) on every file in a directory

FILES=`find $1 -type f -name '*.orbit' -print`
RESULT=0

for i in $FILES; do
    $2 $i
    if [ $? != 0 ]; then
        RESULT=1
    fi
done

[ $RESULT -ne 0 ] && exit 1
exit 0
