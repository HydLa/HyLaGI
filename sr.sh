#!/bin/sh

if [ -f reduce.pid ]
then
	# ����hydla�R�}���h�ɉ������Ȃ�
	kill `cat reduce.pid`
	rm reduce.pid
fi
[REDUCE_PATH]/reduce -w -F- -L log_reducef.txt > /dev/null 2>&1 &
echo $! > reduce.pid

./hydla $*

kill `cat reduce.pid`
rm reduce.pid
