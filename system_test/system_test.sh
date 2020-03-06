ls ../examples/*.hydla | xargs -L 1 -P 40 hylagi
ls hydat/*.hydat | xargs -L 1 -P 40 -I {} python3 compare_hydat.py {} {}.master