#!/usr/bin/ruby

require 'optparse'

def dump(cases)
  cases.each do |one_case|
    puts "case: " + one_case[:id].to_s

    puts " " * 2 + "traj:" if one_case[:traj]
    one_case[:traj].each do |traj|
      puts " " * 4 + "phase: " + traj[:phase] + " " + traj[:id].to_s
      puts " " * 4 + "ms: " + traj[:ms]
      puts " " * 4 + "vm: "
      traj[:vm].each do |key, val|
        puts " " * 6 + key + "\t: " + val
      end
    end

    puts " " * 2 + "pm:" if one_case[:param] != {}
    one_case[:param].each do |param, range|
      puts " " * 4 + param + "\t: " + range
    end
  end
end

def diff_vm(lhs_cases, rhs_cases)
  msg = "{\n"
  lhs_case, rhs_case = lhs_cases.each, rhs_cases.each
  loop do
    lhs_one_case = lhs_case.next
    lhs_trajs, rhs_trajs = lhs_one_case[:traj].each, rhs_case.next[:traj].each
    msg +=  " " * 2 + "{\"case " + lhs_one_case[:id].to_s + "\", \n"
    loop do
      lhs_traj = lhs_trajs.next
      lhs_vm, rhs_vm = lhs_traj[:vm].sort.each, rhs_trajs.next[:vm].sort.each
      msg +=  " " * 4 + "{\"" + lhs_traj[:phase] + " " + lhs_traj[:id].to_s + "\", \n" + " " * 6
      loop do
        lhs_var, lhs_val = lhs_vm.next
        rhs_var, rhs_val = rhs_vm.next
        raise "unequal variable name" unless lhs_var == rhs_var
        msg += "{\"" + lhs_var + "\", Simplify[(" + lhs_val + ")-(" + rhs_val + ")] == 0},"
      end
      msg = msg[0, msg.length-1]
      msg += "},\n"
    end
    msg = msg[0, msg.length-2]
    msg += "\n},\n"
  end

  msg = msg[0, msg.length-2]
  msg += "}"
  puts msg
end

def diff_ms(lhs_cases, rhs_cases)
  lhs_case, rhs_case = lhs_cases.each, rhs_cases.each
  loop do
    lhs_one_case = lhs_case.next
    lhs_trajs, rhs_trajs = lhs_one_case[:traj].each, rhs_case.next[:traj].each
    puts "case: " + lhs_one_case[:id].to_s
    loop do
      lhs_traj = lhs_trajs.next 
      lhs_ms, rhs_ms = lhs_traj[:ms], rhs_trajs.next[:ms]
      puts " " * 2 +  "phase: " + lhs_traj[:phase] + " " + lhs_traj[:id].to_s
      if lhs_ms == rhs_ms
        puts " " * 4 + "lhs MS equals rhs MS"
      else
        puts " " * 4 + "lhs MS unequals rhs MS"
      end
      puts " " * 6 + "lhs MS: " + lhs_ms
      puts " " * 6 + "rhs MS: " + rhs_ms
    end
  end
end

def extract(str)
  return "" unless str
  str.gsub(/(#|---)/, "").chomp
end

def get_data(enum)
  line = enum.next
  line = enum.next while extract(line) == ""
  if extract(line) == "parameter condition" # unuse
    line = enum.next
    while line.index("parameter") == 0
      line = enum.next
    end
  end

  cases = []
  loop do
    while extract(line) =~ /Case (.*)/
      one_case = {}
      # case_num = $1.to_i # unuse
      line = enum.next
      one_case[:traj] = []
      while extract(line) =~ /\d/ # #---1--
        # step_num = extract(line).to_i # unuse
        line = enum.next
        2.times do
          traj = {}
          extract(line) =~ /(.*) (.*)/
          traj[:phase], traj[:id] = $1, $2.to_i
          traj[:ms] = enum.next.chomp
          traj[:vm] = []
          line = enum.next
          while line.chomp == extract(line) and line.chomp !=""
            if line =~ /->(.*)/
              traj[:vm].push ["time", $1]
            else
              line =~ /(.*)\t: (.*)/
              traj[:vm].push [$1, $2]
            end
            line = enum.next
          end
          one_case[:traj].push traj
        end
      end

      line = enum.next while extract(line) == ""

      one_case[:param] = {} 
      if extract(line) == "parameter condition" # unuse
        line = enum.next
        while (line =~ /^parameter(.*)\t: (.*)/) == 0
          one_case[:param]["parameter"+$1] = $2
          line = enum.next
        end
      end

      cases.push one_case
      line = enum.next if line.index("#") == 0
      line = enum.next while extract(line) == ""
    end
  end

  cases.sort! do |lhs, rhs|
    lhs[:traj].collect{ |it| it[:ms] } <=> rhs[:traj].collect{ |it| it[:ms] }
  end
  i = 1
  cases.each do |one_case|
    one_case[:id] = i
    i += 1
  end

  return cases
end

def to_enum(arg)
  if arg.index ".hydla" # treat as a command
    `#{arg}`.each
  else                  # treat as a file
    File.open(arg).each
  end
end

# Main process
OptionParser.new do |opt|
  mode = 'NORMAL'
  opt.on('-n', '--normal'){  mode = 'NORMAL' }
  opt.on('-v', '--vcs')   {  mode = 'VCS'    }
  opt.on('-m', '--ms')    {  mode = 'MS'     }

  arr = opt.parse(ARGV != [] ? ARGV : '-h')

  case mode
  when 'VCS' then
    arr.each do |cmd|
      lhs_cases = get_data(to_enum(cmd + ' -sr'))
      rhs_cases = get_data(to_enum(cmd + ' -sm'))
      diff_vm(lhs_cases, rhs_cases)
    end
  when 'MS' then
    if arr.size == 1
      cases = get_data(to_enum(arr[0]))
      dump(cases)
    elsif arr.size == 2
      lhs_cases = get_data(to_enum(arr[0]))
      rhs_cases = get_data(to_enum(arr[1]))
      diff_ms(lhs_cases, rhs_cases)
    end
  else # when 'NORMAL'
    if arr.size == 1
      cases = get_data(to_enum(arr[0]))
      dump(cases)
    elsif arr.size == 2
      lhs_cases = get_data(to_enum(arr[0]))
      rhs_cases = get_data(to_enum(arr[1]))
      diff_vm(lhs_cases, rhs_cases)
    end
  end
end

