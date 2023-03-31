#!/usr/bin/env bash

set -e

echo -n "testing system_test"
if [ -z "$fnum" ]; then
  # fnumが未定義ならファイル数
  fnum=`ls ../examples/*.hydla | grep -c ''`
fi
echo " in $fnum parallel..."

ls ../examples/*.hydla | xargs -L 1 -P $fnum -I {} bash -c 'hylagi {} &> /dev/null && printf "hylagi %s \033[32m%s\033[m\n" {} "finished" || printf "hylagi %s \033[31m%s\033[m\n" {} "failed"'
printf "%s \033[32m%s\033[m\n" "exec examples" "succeeded"
ls ../check_examples/*.hydla | xargs -L 1 -P $fnum -I {} bash -c 'hylagi {} &> /dev/null && printf "hylagi %s \033[32m%s\033[m\n" {} "finished" || printf "hylagi %s \033[31m%s\033[m\n" {} "failed"'
printf "%s \033[32m%s\033[m\n" "exec check_examples" "succeeded"

ls hydat/*.hydat | xargs -L 1 -P $fnum -I {} python3 compare_hydat.py {} {}.master
printf "%s \033[32m%s\033[m\n" "system_test" "succeeded"
