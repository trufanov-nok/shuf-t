#include "utils.h"


void printTime(const  int msc)
{
    int hours = msc/3600000;
    int minutes = msc/60000 % 60;
    float seconds = fmod((double)msc/1000.0,60.0);

    QString str;

    if (hours > 0) str = QString().sprintf("%ih ", hours);
    if (minutes > 0) str += QString().sprintf("%i min ", minutes);
    str +=  QString().sprintf("%2.2f sec\n",seconds);

    print(str);
}

void initCommandOptionsParser(QCommandLineParser &parser)
{
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.setApplicationDescription("This application shuffles the input file lines skipping (optionaly) the header. It's optimized for files bigger than available RAM. Shuffling performed in 3 steps:\n1. The file is scanned and offsets of all lines are stored in RAM.\n2. Offsets are shuffled (Fisher–Yates algorithm).\n3. Iteratively shuf-t takes the offsets of first N shuffled lines and sort their offsets ascending. Lines are read from input file (only seeking forward is used as offsets are sorted) and written to memory allocated buffer in shuffled order. Then buffer is written to output file. Buffer size can be adjusted by user based on available amount of free RAM. Value N is chosen based on lines length and buffer size at the beginning of each iteration.");
    parser.addOption(QCommandLineOption(QStringList() << "i" << "input_range", "Act as if input came from a file containing the range of unsigned decimal integers LO...HI, one per line.", "LO-HI"));
    parser.addOption(QCommandLineOption(QStringList() << "o" << "output", "Write output to OUTPUT-FILE instead of standard output.", "OUTPUT-FILE"));
    parser.addOption(QCommandLineOption(QStringList() << "n" << "head-count", "Output at most COUNT lines. By default, all input lines are output.", "COUNT", "0"));
    parser.addOption(QCommandLineOption(QStringList() << "b" << "buffer", "Buffer size in MiB for read operations. Default is 1 GiB.", "SIZE", "1024"));
    parser.addOption(QCommandLineOption(QStringList() << "q" << "quiet", "Don't output shuffling progress.", "BOOL", "0"));
    parser.addOption(QCommandLineOption(QStringList() << "t" << "top", "Copy top COUNT lines to output without shuffling. Top is taken from the beggining of file before -l is applied.", "COUNT", "0"));
    parser.addOption(QCommandLineOption(QStringList() << "l" << "lines", "Shuffle and output only lines in range FROM..TO.", "FROM-TO"));
    parser.addOption(QCommandLineOption(QStringList() << "s" << "seed", "Initialize rand function with given seed. Overwise system time is used.", "VALUE"));

}
