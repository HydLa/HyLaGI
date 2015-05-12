#!/bin/sh

find ./examples/ \( -type f -and -name '*.hydla' \) | while read FILE
do
    echo ${FILE}
    ./bin/hylagi ${FILE}
    if [ "$?" != "0" ];
    then
        echo "Something is wrong!"
        break
    fi
done

