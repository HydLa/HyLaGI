echo "testing system_test..."
fnum=`ls ../examples/*.hydla | grep -c ''`

ls ../examples/*.hydla | xargs -L 1 -P $fnum -I {} bash -c 'hylagi {} 1> /dev/null && printf "hylagi %s finished\n" {}'
if [ $? -ne 0 ] ; then
  printf "%s \033[31m%s\033[m\n" "hylagi" "failed"
  exit 1
fi
printf "%s \033[32m%s\033[m\n" "hylagi" "succeeded"

ls hydat/*.hydat | xargs -L 1 -P $fnum -I {} python3 compare_hydat.py {} {}.master
if [ $? -ne 0 ] ; then
  printf "%s \033[31m%s\033[m\n" "system_test" "failed"
  exit 1
fi
printf "%s \033[32m%s\033[m\n" "system_test" "succeeded"