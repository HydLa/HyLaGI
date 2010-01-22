#set terminal  postscript eps monochrome dashed dashlength 5 14
#set output 'out.eps'

set terminal png giant
set output 'out.png'

set xrange [ 0 : 6 ]
set yrange [ 0 : 21 ]

set xlabel "bx"
set ylabel "by"

set object 1 rect from 5,6 to 6,14
set object 2 rect from 1,4.5 to 4,15

plot 'out.data' u 3:4 title 'trajectory' w l
