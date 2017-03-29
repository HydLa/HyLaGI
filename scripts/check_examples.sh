#!/bin/sh

find ./examples/ -maxdepth 1 \( -type f -and -name '*.hydla' \) | while read FILE
do
    echo ${FILE}
    ./bin/hylagi ${FILE}
    if [ "$?" != "0" ];
    then
        echo "Something is wrong!"
        while :
        do
          echo "Continue? Input y or n."
          read ans < /dev/tty
          if [ ${ans} = "y" ] || [ ${ans} = "n" ];
          then
              break
          fi
        done
        if [ ${ans} = "y" ];
        then
            continue
        else
            break
        fi
    fi
done
