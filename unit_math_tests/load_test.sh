#!/bin/bash
echo "testing loat_test..."
if [ $(./load.m | grep -c '') -ne 0 ] ; then
  printf "%s \033[31m%s\033[m\n" "load_math_source" "failed"
  exit 1
fi
printf "%s \033[32m%s\033[m\n" "load_math_source" "succeeded"