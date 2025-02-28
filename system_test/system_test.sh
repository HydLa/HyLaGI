#!/usr/bin/env bash

set -e
cd $(dirname $0)

rm -rf hydat/*.hydat
ls hydat/*.hydat.master | sed -e 's/.master$//' | xargs -I {} bash -c 'touch {}'

echo -n "testing system_test"
if [ -z "$fnum" ]; then
  # fnumが未定義なら1
  fnum=1
  # fnum=`ls ../examples/*.hydla | grep -c ''`
fi
echo " in $fnum parallel..."

function test(){
  ../bin/hylagi $1 &> /dev/null \
    && printf "../bin/hylagi %s \033[32m%s\033[m\n" $1 "finished" \
    || (printf "../bin/hylagi %s \033[31m%s\033[m\n" $1 "failed"; \
        exit 255)
}
export -f test

ls ../examples/*.hydla | xargs -P $fnum -I {} bash -c "test {}"
printf "%s \033[32m%s\033[m\n" "exec examples" "succeeded"

ls ../check_examples/*.hydla | xargs -P $fnum -I {} bash -c "test {}"
printf "%s \033[32m%s\033[m\n" "exec check_examples" "succeeded"

ls hydat/*.hydat | xargs -P $fnum -I {} python3 compare_hydat.py {} {}.master
printf "%s \033[32m%s\033[m\n" "system_test" "succeeded"
