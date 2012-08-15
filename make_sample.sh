#!/bin/sh

output(){
    output_file=$1
    shift
    echo "./hyrose $@" > $output_file
    ./hyrose $@ >> $output_file
    cat $output_file
}


if [ $# -lt 1 ]; then
  echo "usege: $0 <hydla_program> <options>..." 1>&2
  exit 1
fi

hydla=`echo $1 | sed -e  "s/.*\/\([^/]*\)\..*/\1/g"`

output ./samples/$hydla.sample "$@"
#output ./samples_reduce/$hydla.sample "$@ -s r"