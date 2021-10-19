#!/bin/sh

check_list_name="_check_list"
touch ${check_list_name}
for FILE in `find ./examples/ -maxdepth 1 \( -type f -and -name '*.hydla' \)`;
do
    if grep ${FILE} ${check_list_name}
    then # すでにチェック済みならスキップ
       continue
    fi
    echo ${FILE}
    ./bin/hylagi ${FILE}
    if [ "$?" != "0" ];
    then
        echo "Something is wrong!"
        while :
        do
          echo "Continue? Input y or n."
          read ans < /dev/tty
          if [ ${ans} == "y" ] || [ ${ans} == "n" ];
          then
              break
          fi
        done
        if [ ${ans} == "y" ];
        then
            continue
        else
            exit 1;
        fi
    else
        echo ${FILE} >>  ${check_list_name}
    fi
done

rm ${FILE}  ${check_list_name}
