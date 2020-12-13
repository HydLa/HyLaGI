set -eu

ls ../examples/*.hydla | xargs -L 1 -P 40 hylagi
ls ../check_examples/*.hydla | xargs -L 1 -P 40 hylagi
ls hydat/*.hydat | xargs -I {} mv {} {}.master
