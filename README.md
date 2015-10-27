shuf-t 1.2
======
This application shuffles the input file lines skipping (optionaly) the header. It's optimized for files bigger than available RAM. Shuffling performed in 3 steps:  
1. The file is scanned and offsets of all lines are stored in RAM.  
2. Offsets are shuffled (Fisher–Yates algorithm).  
3. Iteratively shuf-t takes the offsets of first N shuffled lines and sort their offsets ascending. Lines are read from input file (only seeking forward is used as offsets are sorted) and written to memory allocated buffer in shuffled order. Then buffer is written to output file. Buffer size can be adjusted by user based on available amount of free RAM. Value N is chosen based on lines length and buffer size at the beginning of each iteration.  
  
Options:  
```
  -v, --version               Displays version information.  
  -h, --help                  Displays this help.  
  -i, --input_range <LO-HI>   Act as if input came from a file containing the  
                              range of unsigned decimal integers LO...HI, one  
                              per line.  
  -o, --output <OUTPUT-FILE>  Write output to OUTPUT-FILE instead of standard  
                              output.  
  -n, --head-count <COUNT>    Output at most COUNT lines. By default, all input  
                              lines are output.  
  -b, --buffer <SIZE>         Buffer size in MiB for read operations. Default  
                              is 1 GiB.  
  -q, --quiet <BOOL>          Don't output shuffling progress.  
  -t, --top <COUNT>           Copy top COUNT lines to output without shuffling.  
                              Top is taken from the beggining of file before -l  
                              is applied.  
  -l, --lines <FROM-TO>       Shuffle and output only lines in range FROM..TO.  
  -s, --seed <VALUE>          Initialize rand function with given seed.  
                              Overwise system time is used. 
```


Changelog:  
1.2 - Qt dependencies were removed (Qt Creator and qmake are still used for building the project). Now app depends only on boost_program_options library.  
1.1 - critical bug fixed in IO stream  
      use stderr instead of stdout to print notifications  
1.0 - initial release.  
