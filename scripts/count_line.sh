find .. | egrep "\.cpp$|\.h$|\.m$|\.red$" | xargs grep -v "^$" | wc -l
