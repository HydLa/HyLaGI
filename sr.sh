#!/bin/sh

[REDUCE_PATH]/reduce -w -F- -L log_reducef.txt > /dev/null 2>&1 &
./hydla $*
killall reduce

