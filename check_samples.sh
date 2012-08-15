#!/bin/sh

no_good=0


judge(){
    command_name=`head -n 1 $1`
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
}

for file_name in ./samples/*.sample
do
    judge $file_name
done

#for file_name in ./samples_reduce/*.sample
#do
#    judge $file_name
#done

if test $no_good -eq 0
    then
    echo "OK!"
else
    echo "NG!"
fi