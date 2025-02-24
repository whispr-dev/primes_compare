[int. to 5k]

PS C:\Users\phine\desktop\primes_ben\dist> .\cs_exe_bm
Starting benchmarks with 10 runs, 10 repeats per run...

Progress: Run 10/10 - Testing: r-primes-fast.exeexe

Program                  Avg (ms)       Min (ms)       Max (ms)       StdDev (ms)
----------------------------------------------------------------------
p-primes.exe             684.284        657.214        695.213        11.940
p-primes-fast.exe        1314.270       1293.219       1341.405       11.912
p-primes-faster.exe      3109.509       3003.473       3189.906       56.416
p-primes-fastest.exe     4676.581       4591.894       4764.591       56.634
c-primes.exe             39.626         22.757         58.340         10.709
c-primes-fast.exe        67.921         22.967         94.494         17.360
r-primes.exe             67.767         19.425         85.183         16.706
r-primes-fast.exe        73.421         33.670         83.421         13.701
PS C:\Users\phine\desktop\primes_ben\dist>



[int. to 500k]

PS C:\users\phine\desktop\primes_ben\dist> .\cs_exe_bm
Starting benchmarks with 10 runs, 10 repeats per run...

Progress: Run 6/10 - Testing: p-primes-fastest.exe
Error running p-primes-fastest.exe: Process execution timed out
Progress: Run 7/10 - Testing: p-primes-fastest.exe
Error running p-primes-fastest.exe: Process execution timed out
Progress: Run 8/10 - Testing: p-primes-fastest.exe
Error running p-primes-fastest.exe: Process execution timed out
Progress: Run 9/10 - Testing: p-primes-fastest.exe
Error running p-primes-fastest.exe: Process execution timed out
Progress: Run 10/10 - Testing: p-primes-fastest.exe
Error running p-primes-fastest.exe: Process execution timed out
Progress: Run 10/10 - Testing: r-primes-fast.exe

Program                  Avg (ms)       Min (ms)       Max (ms)       StdDev (ms)
----------------------------------------------------------------------
p-primes.exe             840.554        756.605        1051.996       110.031
p-primes-fast.exe        1637.786       1318.095       2240.099       341.258
p-primes-faster.exe      3828.754       3184.501       4886.964       673.056
p-primes-fastest.exe     4760.595       4709.470       4851.866       48.255
c-primes.exe             73.561         50.547         97.935         13.174
c-primes-fast.exe        64.386         43.185         83.442         14.913
r-primes.exe             77.463         43.815         108.258        23.856
r-primes-fast.exe        75.135         45.075         104.839        21.021
PS C:\users\phine\desktop\primes_ben\dist>



[int. to 500k]

PS C:\users\phine\desktop\primes_ben\dist> .\cs_exe_bm_mkii
Starting benchmarks with 10 runs, 10 repeats per run...

Progress: Run 10/10 - Testing: r-primes-fast.exe

Program                  Avg (ms)       Min (ms)       Max (ms)       StdDev (ms)
----------------------------------------------------------------------
p-primes.exe             6946.705       4958.079       8422.626       1112.013
c-primes.exe             59.454         45.215         70.105         7.867
c-primes-fast.exe        62.849         39.741         141.576        29.584
r-primes.exe             50.891         32.207         65.914         9.336
r-primes-fast.exe        53.568         43.618         74.686         8.874


[int. to 500k/py@5k]

Progress: Run 10/10 - Testing: r-primes-fast.exe

Program                  Avg (ms)       Min (ms)       Max (ms)       StdDev (ms)
----------------------------------------------------------------------
p-primes.exe             3828.827       3628.089       4210.447       150.006
c-primes.exe             51.316         45.491         58.746         4.705
c-primes-fast.exe        48.279         40.205         57.437         6.325
r-primes.exe             51.428         40.018         70.869         11.184
r-primes-fast.exe        56.623         42.510         94.171         14.569
PS C:\users\phine\desktop\primes_ben\dist>