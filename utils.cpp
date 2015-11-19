/*
Copyright (c) by Alexander Trufanov. Released under a Simplified BSD
license as described in the file LICENSE.
 */
#include "utils.h"

void printTime(double msc)
{
    int hours = (int) msc/3600;
    int minutes = ((int)msc)/60 % 60;
    double seconds = fmod(msc, 60.0);


    char buf[30];
    int len = 0;
    if (hours > 0) len += sprintf(buf+len, "%.2dh ", hours);
    if (minutes > 0) len += sprintf(buf+len, "%.2d min ", minutes);
    sprintf(buf+len, "%.2d sec\n", (int)seconds);

    print(buf);
}

CSimpleOpt::SOption cmdOptions[] = {
    { OPT_VER, _T("-v"),     SO_NONE    },
    { OPT_VER, _T("--version"),     SO_NONE    },
    { OPT_HELP, _T("-h"),     SO_NONE    },
    { OPT_HELP, _T("--help"),     SO_NONE    },
    { OPT_RANGE,  _T("-i"),     SO_REQ_SEP },
    { OPT_RANGE,  _T("--input_range"),     SO_REQ_SEP },
    { OPT_OUT,  _T("-o"),     SO_REQ_SEP },
    { OPT_OUT,  _T("--output"),     SO_REQ_SEP },
    { OPT_HEAD,  _T("-n"),     SO_REQ_SEP },
    { OPT_HEAD,  _T("--head-count"),     SO_REQ_SEP },
    { OPT_BUF,  _T("-b"),     SO_REQ_SEP },
    { OPT_BUF,  _T("--buffer"),     SO_REQ_SEP },
    { OPT_TOP,  _T("-t"),     SO_REQ_SEP },
    { OPT_TOP,  _T("--top"),     SO_REQ_SEP },
    { OPT_QUIET, _T("-q"),     SO_NONE    },
    { OPT_QUIET, _T("--quiet"),     SO_NONE    },
    { OPT_LINES,  _T("-l"),     SO_REQ_SEP },
    { OPT_LINES,  _T("--lines"),     SO_REQ_SEP },
    { OPT_SEED,  _T("-s"),     SO_REQ_SEP },
    { OPT_SEED,  _T("--seed"),     SO_REQ_SEP },
    SO_END_OF_OPTIONS                       // END
};

CSimpleOpt* initCommandLineOptions(int argc, char *argv[])
{
    CSimpleOpt* opt = new CSimpleOpt();
    if (opt->Init(argc, argv, cmdOptions))
        return opt;
    else {
        delete opt;
        return NULL;
    }
}

void showHelp()
{
 const char* help =
"This application shuffles the input file lines skipping (optionaly) the header. It's optimized for files bigger than available RAM. Shuffling performed in 3 steps:\n\
1. The file is scanned and offsets of all lines are stored in RAM.\n\
2. Offsets are shuffled (Fisher–Yates algorithm).\n\
3. Iteratively shuf-t takes the offsets of first N shuffled lines and sort their offsets ascending. Lines are read from input file (only seeking forward is used as offsets are sorted) and written to memory allocated buffer in shuffled order. Then buffer is written to output file. Buffer size can be adjusted by user based on available amount of free RAM. Value N is chosen based on lines length and buffer size at the beginning of each iteration.\n\
\n\
USAGE: shuf-t [-v] [-h] [-q] [-i LO-HI] [-o FILE_OUT] [-n CNT] [-t CNT] [-l BEG-END] [-s SEED] [-b SIZE] file_in\n\
\n\
Command line parameters:\n\
  -v [ --version ]             Display version of application.\n\
  -h [ --help ]                Display this help.\n\
  -i [ --input_range ] arg     Acts as if input came from a file containing the\n\
                               range of unsigned decimal integers LO-HI, one \n\
                               per line. Example: -i 10-100 \n\
  -o [ --output ] arg          Write output to OUTPUT-FILE instead of standard \n\
                               output.\n\
  -n [ --head-count ] arg (=0) Output at most COUNT lines. By default, all \n\
                               input lines are output.\n\
  -b [ --buffer ] arg (=1024)  Buffer size in MiB for read operations. Default \n\
                               is 1 GiB.\n\
  -q [ --quiet ]               Don't output shuffling progress.\n\
  -t [ --top ] arg (=0)        Copy top COUNT lines to output without \n\
                               shuffling. Top is taken from the beggining of \n\
                               file before -l is applied.\n\
  -l [ --lines ] arg           Shuffle and output only lines in range FROM-TO.\n\
  -s [ --seed ]                Initialize rand function with given seed. \n\
                               Overwise system time is used.\n";
 std::printf("%s", help);
}

vector<string> split(const string &s, char delim, bool skip_empty_parts)
{
    stringstream ss(s);
    string item;
    vector<string> elems;
    while (getline(ss, item, delim))
        if (!(skip_empty_parts && item.empty()))
            elems.push_back(item);

    return elems;
}
