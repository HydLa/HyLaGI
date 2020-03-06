git checkout master
cd ../
make clean
make -j
cd system_test
ls ../example/*.hydla | xargs -L 1 -P 40 hylagi
ls hydat/*.hydat | xargs -I {} mv {} {}.master