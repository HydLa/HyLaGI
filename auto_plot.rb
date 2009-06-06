data = ARGF.read

OUT_DATA  = "out.data"
OUT_PLOT  = "out.plt"
OUT_IMAGE = "out.png"

open("out.data", "w") do |f|
  f.puts data
end
puts data

open(OUT_PLOT, "w") do |f|
  f.puts "set terminal png"
  f.puts "set output '#{OUT_IMAGE}'"
  f.print "plot "
  data.scan(/^#(.*)$/) do |line|
    vars = line[0].split
    vars.shift
    index = 2
    vars.each do |v|
      f.print "'#{OUT_DATA}' u 1:#{index} title '#{v}' w l"
      f.print ", " if vars.size != index-1
      index += 1
    end
  end
end

system("gnuplot #{OUT_PLOT}")
#system("eog #{OUT_IMAGE}")
