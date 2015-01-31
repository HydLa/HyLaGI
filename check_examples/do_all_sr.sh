#!/bin/sh

function timeOutExec(){
  timeout=20

  touch run.lock;
  ($*; rm run.lock) &

  cnt=0
  while true
  do
    if [ ! -f run.lock ]; then
      break;
   fi
    if [ $cnt -ge $timeout ]; then
      break;
    fi
    sleep 1;
    cnt=`expr $cnt + 1`
  done

  if [ -f run.lock ]; then
    echo "$cnt second time out"
    pkill -TERM hylagi
    rm run.lock
  fi
  
  return 0
}

files="./*.hydla"
log="./log_all.txt"
rm ${log}
for filepath in ${files}
do
  echo ${filepath} >> ${log}
  timeOutExec ../hylagi -s r -t 5 ${filepath} --nd -f t >> ${log} 2>&1
done
read wait
