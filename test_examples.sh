#!/bin/bash

# *** Test Script for Examples ***
# This script executes every program under examples/ 
# with the options specified in comment lines.

# If your hylagi is not in PATH, 
# set the absolute path to hylagi here
# e.g., hylagi="/path/to/hylagi"
hylagi="hylagi"

# stdout is in CSV style:
#   '<example_file_path>', <exitcode>

# About <exitcode>:
#   0     = success
#   1     = failure
#   124   = timeout (>60s)
#   128+N = Unix SIGNAL N
#     e.g., 134: SIGABRT (128+6)
#           139: SIGSEGV (128+11)

# If there are no comment lines containing "#hylagi",
# <exitcode> = -1.

# If there are multiple options,
# it chooses the first one.

# Log files are generated in logs/ folder

cd "$(dirname $0)/examples"

for f in $(find . -name "*.hydla"); do
  opts="$(grep -c "#hylagi" $f)"
  if [[ $opts -eq 0 ]]; then
    # echo "[$f] $(printf '\033[31m%s\033[m\n' 'Error'): $opts options found (expected: 1)"
    echo "'$f', -1"
    continue
  fi
  # echo $f
  args=$(grep "#hylagi" $f | head -1 | sed -e "s/^\s*\/\///" | sed -e 's/#hylagi//' | sed -e 's/\r//g')
  cmd="$hylagi $args $f"
  # echo $cmd
  mkdir -p "../logs/$(dirname "$f")"
  timeout 60 $cmd &> ../logs/$f.txt
  ec=$?
  echo "'$f', $ec"
  # if [[ $ec -eq 124 ]]; then
  #   echo "[$f] $(printf '\033[31m%s\033[m\n' 'Timeout') (10s): $cmd"
  # elif [[ $ec -eq 134 ]]; then
  #   echo "[$f] $(printf '\033[32m%s\033[m\n' 'Success') (134): $cmd"
  # elif [[ $ec -eq 0 ]]; then
  #   echo "[$f] $(printf '\033[32m%s\033[m\n' 'Success'): $cmd"
  # else
  #   echo "[$f] $(printf '\033[31m%s\033[m\n' 'Failed') ($ec): $cmd"
  # fi
done
