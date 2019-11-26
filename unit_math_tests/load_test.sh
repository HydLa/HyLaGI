#!/bin/bash

if [ $(./load.m | grep -c '') -ne 0 ] ; then
  echo "load_math_source failed..."
  exit 1
fi
echo "load_math_source succeeded"