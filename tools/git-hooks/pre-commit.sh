#!/bin/sh
git stash -q --keep-index
REPO_DIR=$(pwd)
${REPO_DIR}/tools/git-hooks/run-tests.sh "$REPO_DIR"
RESULT=$?

git stash pop -q
[ $RESULT -ne 0 ] && exit 1
exit 0
