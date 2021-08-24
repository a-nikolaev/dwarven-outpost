#! /usr/bin/env ruby

require 'tempfile'
require 'fileutils'

def append(ht, k, v)
  if ht[k] == nil
    ht[k] = [v]
  else
    ht[k] << v
  end
end

def mean(v)
  v.map{|x| x.to_f}.reduce(:+) / v.size.to_f
end

def run_test(executable) 

  exe_dir = File.dirname(executable)
  exe_basename = File.basename(executable)

  arr_dw = Array.new
  arr_st = Array.new
  arr_lm = Array.new
  arr_ap = Array.new
  arr_pu = Array.new

  srand 123789465

  count_timeouts = 0
  count_crashes = 0

  seed_index = 0
  seed_array = Array.new
  500.times {
    seed_array << rand(1000000)
  }

  [6, 7, 8].each{|num|
    [26, 30, 34].each{|rows|
      [26, 30, 34].each{|cols|
        [0, 1, 2].each{|seed_iteration|
          seed = seed_array[seed_index]
          seed_index += 1
          # seed = rand(10000000)
          
          rows1 = rows + (rand(3) - 1);
          cols1 = cols + (rand(3) - 1);

          num1 = num

          tmp_file = Tempfile.create('dwarves-tmpfile')

=begin
          full_cmd = "#{cmd} #{rows1} #{cols1} #{num1} #{debris1} #{mpr1} #{seed} fast #{tmp_file.path}"
          `#{full_cmd}`
=end
          timeout_time = 20

          full_cmd = "./#{exe_basename} c #{rows1} #{cols1} #{num1} #{seed} fast #{tmp_file.path}"
          status = `(cd #{exe_dir}; timeout #{timeout_time} #{full_cmd} > /dev/null 2> /dev/null ); echo $?`.to_i

          raw_dw = 0.0
          raw_st = 0.0
          raw_lm = 0.0
          raw_ap = 0.0
          raw_pu = 0.0

          # if stopped due to time-out
          if status == 124

            count_timeouts += 1
            rate = 0

            if count_timeouts > 7
              return [false, [0.0, 0.0, 0.0, 0.0, 0.0], count_timeouts, count_crashes]
            end

          elsif status != 0
            
            count_crashes += 1

          else

            res = IO.read(tmp_file.path).split("\n")
            File.unlink(tmp_file)

            succ = false 
            if res[0] == "success"
              succ = true
            end

            time = res[1].split()[1].to_i
            dwarves = res[2].split()[1].to_i
            structures = res[3].split()[1].to_i
            lumber = res[4].split()[1].to_i
            apples = res[5].split()[1].to_i
            pumpkins = res[6].split()[1].to_i

            puts "rows: #{rows1} cols: #{cols1} num: #{num1} seed: #{seed} dwarves: #{dwarves} structures: #{structures} lumber: #{lumber} apples: #{apples} pumpkins: #{pumpkins}"

            raw_dw = dwarves.to_f / num1.to_f
            raw_st = structures.to_f
            raw_lm = lumber.to_f
            raw_ap = apples.to_f 
            raw_pu = pumpkins.to_f

          end
            
          arr_dw << raw_dw
          arr_st << raw_st
          arr_lm << raw_lm
          arr_ap << raw_ap
          arr_pu << raw_pu

        }
      }
    }
  }

  mean_raw_dw = mean(arr_dw)
  mean_raw_st = mean(arr_st)
  mean_raw_lm = mean(arr_lm)
  mean_raw_ap = mean(arr_ap)
  mean_raw_pu = mean(arr_pu)

  return [true, [mean_raw_dw, mean_raw_st, mean_raw_lm, mean_raw_ap, mean_raw_pu], count_timeouts, count_crashes]
  
end

def extra_credit(x)
  extra = 0.3
  if x < 0 then
    0
  elsif x < 1 then
    x
  else
    1.0 + extra * (1.0 - Math.exp(-(x-1.0)))
  end
end

###
### Checker
###

# Returns [Performance, Leaderboard]
def check_correctness(score_factor, executable)

  if ! File.exist?(executable)
    $stderr.puts "Check-correctness error"
    $stderr.puts "Executable '#{executable}' is not found"
    exit(1)
  end

  arr = run_test(executable)
  status_good = arr[0]
  avgs = arr[1]

  avg_dw = avgs[0]
  avg_st = avgs[1]
  avg_lm = avgs[2]
  avg_ap = avgs[3]
  avg_pu = avgs[4]

  timeouts = arr[2]
  crashes = arr[3]

  # Reporting
  
  output = ""

  if timeouts > 0
    output += "Timeouts: #{timeouts}\n\t(possibly, infinite loops).\n\n"
  end
  if crashes > 0
    output += "Crashes: #{crashes}\n\t(possibly, segmentation faults and access outside the bounds of an array or string).\n\n"
  end

  score = 0.0

  if status_good then

    res = (avg_dw*100.0).round(1)
    res_s = sprintf("%7.1f", res)
    sc_dw = 25.0 * extra_credit(avg_dw)
    output += "Dwarves survived (on average):  #{res_s}%  (#{sc_dw.round(1)} / #{25} pts)\n"
   
    res = (avg_st).round(1)
    res_s = sprintf("%7.1f", res)
    sc_st = 25.0 * extra_credit(avg_st / 30.0)
    output += "Largest structure (on average): #{res_s}   (#{sc_st.round(1)} / #{25} pts)\n"

    res = (avg_ap).round(1)
    res_s = sprintf("%7.1f", res)
    sc_ap = 25.0 * extra_credit(avg_ap / 1000.0)
    output += "Apples (on average):            #{res_s}   (#{sc_ap.round(1)} / #{25} pts)\n"
    
    res = (avg_pu).round(1)
    res_s = sprintf("%7.1f", res)
    sc_pu = 25.0 * extra_credit(avg_pu / 30.0)
    output += "Pumpkins (on average):          #{res_s}   (#{sc_pu.round(1)} / #{25} pts)\n"

    output.strip!

    score = sc_dw + sc_st + sc_ap + sc_pu
  end

  rate = score / 100.0

  one_star = '🌟'
  stars = ''
  if rate < 0.5
    stars = ''
  elsif rate < 0.6
    stars = one_star * 1
  elsif rate < 0.7
    stars = one_star * 2
  elsif rate < 0.8
    stars = one_star * 3
  elsif rate < 0.9
    stars = one_star * 4
  elsif rate < 1.0
    stars = one_star * 5
  elsif rate < 1.1
    stars = one_star * 6
  elsif rate < 1.2
    stars = one_star * 7
  else
    stars = one_star * 8
  end

  puts
  puts('---------------------------------------------------------')
  puts("Total Score: #{(score * score_factor).round(1)} out of 100.")
  puts
  puts("Stars: #{stars}")
  puts
  puts(output)
  puts('---------------------------------------------------------')
  puts

end

if ARGV.size < 1
  puts("\nUsage:\n\t ./score-stage-c.rb PATH-TO-DWARVES-EXECUTABLE")
  puts("\nExample:\n\t ./score-stage-c.rb ./code/dwarves\n\n")
  exit(1)
end

check_correctness(1.0, ARGV[0])

