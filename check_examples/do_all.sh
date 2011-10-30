#!/bin/sh
files="./*.hydla"
log="./log_all.txt"
rm ${log}
for filepath in ${files}
do
  echo ${filepath} >> ${log}
  ../hydla -m s -t 5 ${filepath} --nd -f t >> ${log}
done
read wait