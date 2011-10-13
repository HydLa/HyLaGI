#!/bin/sh
files="./*.hydla"
for filepath in ${files}
do
  echo ${filepath}
  ../hydla -m s -t 1 ${filepath} -f t
done
read wait