#!/bin/bash

ROOT=$(pwd)
BIN=$ROOT/build/src/main.bin
INPUT=/tmp/test.md

rm -f $INPUT

function runtest {
  echo $1 > $INPUT
  RESULT=$($BIN $INPUT &2> /dev/null)
  echo $RESULT "'$2'"
}

s="paragraph\n> blockquoted paragraph\n"
runtest "$s"

