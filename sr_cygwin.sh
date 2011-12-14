#!/bin/sh

killall reduce
#cd /home/yysaki/HydLa_20111212
./reduce.exe -w -F- -L log_reducef.txt &
./hydLa.exe -m s -s r -f t examples/bouncing_particle.hydla -t 2 -d
