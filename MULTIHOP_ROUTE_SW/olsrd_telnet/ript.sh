#!/bin/bash
FILES=/home/szabokar/olsrd/*.patch
for f in $FILES
do
  echo "Processing $f file..."
  # take action on each file. $f store current file name
  git apply $f
done
