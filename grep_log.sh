#!/bin/bash

if test $# -ne 2 ; then 
  echo "usage: grep_log.sh [input_file] [word_to_grep]"
else
cat $1 | awk 'BEGIN{RS="@";ORS=""} /'$2'/{print "@";print $0;print "\n"}'
fi
