#!/bin/sh
files="./*.hydla"
log="./log_all.txt"
rm ${log}
for filepath in ${files}
do
  echo ${filepath} >> ${log}
  ../bin/hyrose -t5 ${filepath} --nd >> ${log}
done
read wait