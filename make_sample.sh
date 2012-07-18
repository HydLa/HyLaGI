#!/bin/sh


# 実行時に指定された引数の数、つまり変数 $# の値が 3 でなければエラー終了。
if [ $# -lt 2 ]; then
  echo "usege: $0 <output_file> <commands>..." 1>&2
  exit 1
fi

output_file=$1

shift

echo "$@" > $output_file
$@ >> $output_file
cat $output_file