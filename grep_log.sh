#!/bin/bash

if test $# -lt 2 ; then 
  echo "usage: grep_log.sh [input_file] [word_to_grep]..."
else
words=$2
for word in ${@:3:($#-2)}
do
words=$words"|$word"
done
cat $1 | awk 'BEGIN{RS="@";ORS=""} /'$words'/{print "@";print $0;print "\n"}'
fi
