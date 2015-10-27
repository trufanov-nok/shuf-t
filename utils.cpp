/*
Copyright (c) by Alexander Trufanov. Released under a Simplified BSD
license as described in the file LICENSE.
 */
#include "utils.h"
#include <boost/format.hpp>

const char* SHUF_T_VERSION = "1.2";

void printTime(double msc)
{
    int hours = msc/3600;
    int minutes = ((int)msc)/60 % 60;
    float seconds = fmod(msc, 60.0);


    string str;
    if (hours > 0) str = (boost::format("%ih ") % hours).str();
    if (minutes > 0) str.append((boost::format("%i min ") % minutes).str());
    str.append((boost::format("%i sec\n") % seconds).str());

    print(str);
}

bool initCommandLineOptions(po::variables_map& vm, int argc, char *argv[])
{
    try {
        po::options_description desc;
        desc.add_options()
                ("version,v", "Display version of application.")
                ("help,h", "Display this help.")
                ("input_range,i", po::value< string > (), "Acts as if input came from a file containing the range of unsigned decimal integers LO-HI, one per line. Example: -i 10-100 ")
                ("output,o",  po::value< string > (), "Write output to OUTPUT-FILE instead of standard output.")
                ("head-count,n", po::value< int > ()->default_value(0), "Output at most COUNT lines. By default, all input lines are output.")
                ("buffer,b", po::value< float > ()->default_value(1024), "Buffer size in MiB for read operations. Default is 1 GiB.")
                ("quiet,q", "Don't output shuffling progress.")
                ("top,t", po::value< int > ()->default_value(0), "Copy top COUNT lines to output without shuffling. Top is taken from the beggining of file before -l is applied.")
                ("lines,l", po::value< string > (), "Shuffle and output only lines in range FROM-TO.")
                ("seed,s", "Initialize rand function with given seed. Overwise system time is used.");

        po::options_description pos;
        pos.add_options()
                ("input-file",  po::value< vector<string> >(), "");

        po::options_description all;

        all.add(desc);
        all.add(pos);

        po::positional_options_description p;
        p.add("input-file", -1);

        try
        {
            po::store(po::command_line_parser(argc, argv).
                      options(all).positional(p).run(), vm);

            /** --help option
           */
            if ( vm.count("help")  )
            {
                std::cout << "This application shuffles the input file lines skipping (optionaly) the header. It's optimized for files bigger than available RAM. Shuffling performed in 3 steps:" << std::endl
                          << "1. The file is scanned and offsets of all lines are stored in RAM." << std::endl
                          << "2. Offsets are shuffled (Fisher–Yates algorithm)." << std::endl
                          << "3. Iteratively shuf-t takes the offsets of first N shuffled lines and sort their offsets ascending. Lines are read from input file (only seeking forward is used as offsets are sorted) and written to memory allocated buffer in shuffled order. Then buffer is written to output file. Buffer size can be adjusted by user based on available amount of free RAM. Value N is chosen based on lines length and buffer size at the beginning of each iteration." << std::endl << std::endl
                          << "Command line parameters:" << std::endl
                          << desc << std::endl;
                return false;
            } else if ( vm.count("version")  )
            {
                std::cout << "Program version: " << SHUF_T_VERSION << std::endl << "Boost library version: " << BOOST_LIB_VERSION << std::endl;
                return false;
            }

            po::notify(vm); // throws on error, so do after help in case
            // there are any problems
        }
        catch(po::error& e)
        {
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
//            std::cerr << desc << std::endl;
            return false;
        }

    }
    catch(std::exception& e)
    {
        std::cerr << "Unhandled Exception: "
                  << e.what() << ", application will now exit" << std::endl;
        return false;

    }

  return true;
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
