find .. | egrep "\.cpp$|\.h$|\.m$|\.red$" | xargs ./remove_comment_blank_lines.pl | wc -l
