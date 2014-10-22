#!/bin/bash

if test $# -ne 2 ; then 
  echo "usage: replace_all.sh [before] [after]"
else
  find .. \( -type d -and -name '.git' -and -prune \) -or \( -type f -and \( -name \*.cpp -or -name \*.h \) -and -print \) | xargs sed -i "s/${1}/${2}/g"
fi
