#!/bin/sh
# Runs the orbit compiler (orbitc) on every file in a directory

FILES=`find $1 -type f -name '*.orbit' -print`
RESULT=0

for i in $FILES; do
    echo "$2 $i"
    $2 $i
    if [ $? != 0 ]; then
        RESULT=1
    fi
done

exit $RESULT

#return find $1 -type f -name '*.orbit' -exec $2 {} \;
