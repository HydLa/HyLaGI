#!/bin/sh

cd `dirname $0`

ruby check_diff.rb -c "../hyrose red_data/bouncing_particle.hydla    -sr -t100 -p3 --nd" red_data/bouncing_particle.data | math
ruby check_diff.rb -c "../hyrose red_data/impulse_by_impulse.hydla   -sr -t100 -p3 --nd" red_data/impulse_by_impulse.data | math
ruby check_diff.rb -c "../hyrose red_data/impulse_function.hydla     -sr -t100 -p3 --nd" red_data/impulse_function.data | math
ruby check_diff.rb -c "../hyrose red_data/sawtooth_wave.hydla        -sr -t100 -p3 --nd" red_data/sawtooth_wave.data | math
ruby check_diff.rb -c "../hyrose red_data/square_wave.hydla          -sr -t100 -p3 --nd" red_data/square_wave.data | math
ruby check_diff.rb -c "../hyrose red_data/step_function.hydla        -sr -t100 -p3 --nd" red_data/step_function.data | math
ruby check_diff.rb -c "../hyrose red_data/triangle_wave.hydla        -sr -t100 -p3 --nd" red_data/triangle_wave.data | math
ruby check_diff.rb -c "../hyrose red_data/bouncing_particle_rp.hydla -sr -t100 -p3 --nd" red_data/bouncing_particle_rp.data | math
ruby check_diff.rb -c "../hyrose red_data/box1.hydla                 -sr -t100 -p3 --nd" red_data/box1.data | math
ruby check_diff.rb -c "../hyrose red_data/circle.hydla               -sr -t100 -p3 --nd" red_data/circle.data | math
ruby check_diff.rb -c "../hyrose red_data/dansa.hydla                -sr -t100 -p3 --nd" red_data/dansa.data | math
ruby check_diff.rb -c "../hyrose red_data/roof_bouncing.hydla        -sr -t100 -p3 --nd" red_data/roof_bouncing.data | math

ruby check_diff.rb -c "../hyrose red_data/magnet_ball.hydla          -sr -t100 -p3 --nd" red_data/magnet_ball.data | math
ruby check_diff.rb -c "../hyrose red_data/balloon_tank_subset.hydla  -sr -t100 -p3 --nd" red_data/balloon_tank_subset.data | math
ruby check_diff.rb -c "../hyrose red_data/n_waves.hydla              -sr -t100 -p3 --nd" red_data/n_waves.data  | math
