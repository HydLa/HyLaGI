#!/bin/bash

cd ../
timeout -s SIGKILL 1.5 ./scripts/test_math_source.sh &> log
stty sane
ln=$(grep -c '' log)
rm -f log
if [ $ln -ne 4 ] ; then
  echo "load_math_source failed..."
  exit 1
fi
echo "load_math_source succeeded"