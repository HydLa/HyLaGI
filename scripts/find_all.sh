#!/bin/bash

if test $# -ne 1 ; then 
  echo "usage: find_all.sh [word_to_find]"
else
  find .. \( -type d -and -name '.git' -and -prune \) -or \( -type f -and \( -name \*.cpp -or -name \*.h \) -and -print \) | xargs grep $1
fi
