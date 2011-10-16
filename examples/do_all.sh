#!/bin/sh
files="./*.hydla"
log="./log_all.txt"
rm ${log}
for filepath in ${files}
do
  echo ${filepath} >> ${log}
  ../hydla -m s -t 1 ${filepath} -f t >> ${log}
done
read wait