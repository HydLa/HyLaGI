#!/usr/bin/ruby

def dump(cases)
  i = 1
  cases.each do |oneCase|
    puts "case: " + i.to_s

    puts "  traj:" if oneCase[:traj]
    oneCase[:traj].each do |elem|
      puts "    phase: " + elem[:phase] + " " + elem[:id].to_s
      puts "      ms: " + elem[:ms]
      puts "      vm: "
      elem[:vm].each do |key, val|
        puts "        " + key + "\t: " + val
      end
    end

    puts "  pm:" if oneCase[:param] != {}
    oneCase[:param].each do |param, range|
      puts "      " + param + "\t: " + range
    end

    i += 1
  end
end

def diffVm(lhsCases, rhsCases)
  lhsCase, rhsCase = lhsCases.each, rhsCases.each
  loop do
    lhsTraj, rhsTraj = lhsCase.next[:traj].each, rhsCase.next[:traj].each
    loop do
      lhsVm, rhsVm = lhsTraj.next[:vm].sort.each, rhsTraj.next[:vm].sort.each
      loop do
        lhsVar, lhsVal = lhsVm.next
        rhsVar, rhsVal = rhsVm.next
        raise "unequal variable name" unless lhsVar == rhsVar
        puts lhsVar + "\t: (" + lhsVal + ")-(" + rhsVal + ")"
      end
    end
  end
end

def extract(str)
  return "" unless str
  str.gsub(/(#|---)/, "").chomp
end

def getData(enum)
  line = enum.next
  line = enum.next if extract(line) == ""
  if extract(line) == "parameter condition" # unuse
    line = enum.next
    while line.index("parameter") == 0
      line = enum.next
    end
  end

  cases = []
  loop do
    while extract(line) =~ /Case (.*)/
      oneCase = {}
      # caseNum = $1.to_i # unuse
      line = enum.next
      oneCase[:traj] = []
      while extract(line) =~ /\d/ # #---1--
        # stepNum = extract(line).to_i # unuse
        line = enum.next
        2.times do
          elem = {}
          extract(line) =~ /(.*) (.*)/
          elem[:phase], elem[:id] = $1, $2.to_i
          elem[:ms] = enum.next.chomp
          elem[:vm] = []
          line = enum.next
          while line.chomp == extract(line) and line.chomp !=""
            if line =~ /->(.*)/
              elem[:vm].push ["time", $1]
            else
              line =~ /(.*)\t: (.*)/
              elem[:vm].push [$1, $2]
            end
            line = enum.next
          end
          oneCase[:traj].push elem
        end
      end

      line = enum.next if extract(line) == ""

      oneCase[:param] = {} 
      if extract(line) == "parameter condition" # unuse
        line = enum.next
        while (line =~ /^parameter(.*)\t: (.*)/) == 0
          oneCase[:param]["parameter"+$1] = $2
          line = enum.next
        end
      end

      cases.push oneCase
      line = enum.next if line.index("#") == 0
      line = enum.next if extract(line) == ""
    end
  end

  cases.sort! do |lhs, rhs|
    lhs[:traj].collect{ |it| it[:ms] } <=> rhs[:traj].collect{ |it| it[:ms] }
  end

  return cases
end

def toEnum(arg)
  if arg.index ".hydla"
    `#{arg}`.each
  else
    File.open(arg).each
  end
end

if ARGV.size == 1
  cases = getData(toEnum(ARGV[0]))
  dump(cases)
elsif ARGV.size == 2
  lhsCases = getData(toEnum(ARGV[0]))
  rhsCases = getData(toEnum(ARGV[1]))
  diffVm(lhsCases, rhsCases)
end
