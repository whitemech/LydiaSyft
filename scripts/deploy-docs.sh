#!/usr/bin/env bash

set -x # un-comment to see what's going on when you run the script

GITHUB_REPONAME="whitemech/LydiaSyft"

# Create a temporary directory and store its name in a variable.
TEMPD=$(mktemp -d)

# Exit if the temp directory wasn't created successfully.
if [ ! -e "$TEMPD" ]; then
    >&2 echo "Failed to create temp directory"
    exit 1
fi

# generate documentation
doxygen Doxyfile

# copy generated HTML documentation in the temporary folder
cp -r ./docs/html/* "$TEMPD"

# move to temporary directory
CURDIR=$(pwd)
cd $TEMPD

# set up Git in temporary directory
git init
git checkout -b gh-pages
git add .
git commit -m "Deploy documentation"

git remote add origin "git@github.com:${GITHUB_REPONAME}.git"

# Push files
git push origin gh-pages --force

# return to previous directory
cd $CURDIR

# Make sure the temp directory gets removed on script exit.
trap "exit 1"           HUP INT PIPE QUIT TERM
trap 'rm -rf "$TEMPD"'  EXIT