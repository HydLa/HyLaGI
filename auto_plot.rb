require 'optparse'

OUT_DATA  = "out.data"
OUT_PLOT  = "out.plt"

terminal_type = "png"
output_ext    = "png"

OptionParser.new do |opt|
  opt.on('-t', 
         '--type=TYPE', 
         'output image type (png, gif, eps, ...) default is "png"') do |v| 
    case v
      when "eps" 
      terminal_type = "postscript enhanced eps"
      output_ext    = "eps"
      else
      terminal_type = v
      output_ext    = v
    end
  end
  opt.parse!(ARGV)
end

data = ARGF.read
open(OUT_DATA, "w") do |f|
  f.puts data
end
puts data

data =~ /^#(.+)$/
vars = $1.split
vars.shift

open(OUT_PLOT, "w") do |f|
  f.puts <<EOS
set terminal #{terminal_type}
set output "out.#{output_ext}"
set xlabel "time"
set ylabel "#{vars.join(", ")}"
EOS

  f.print "plot "
  index = 2
  vars.each do |v|
    f.print "\"#{OUT_DATA}\" u 1:#{index} title \"#{v}\" w l"
    f.print ", \\\n     " if vars.size != index-1
    index += 1
  end
end

system("gnuplot #{OUT_PLOT}")
