#!/bin/sh

no_good=0

for file_name in ./check_examples/*.sample
do
    command_name=`head -n 1 $file_name`
    command_result=`$command_name`
    sample=`tail -n +2 $file_name`
#    echo $command_result
#    echo $sample
    if test "$command_result" != "$sample"
	then
	echo "$file_name NG"
	no_good=1
    else
        echo "$file_name OK"
    fi
done

if test $no_good -eq 0
    then
    echo "OK!"
else
    echo "NG!"
fi