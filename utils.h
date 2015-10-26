#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <string>
#include <sstream>
#include <cstdio>
#include <iostream>
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

static const int QTEXTSTREAM_BUFFERSIZE = 16384;

vector<string> split(const string &s, char delim, bool skip_empty_parts = true);


template < typename T > string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }


extern bool _param_verbose;
extern bool _is_terminal;
inline void print(const char* s)
{
    if (_param_verbose)
    {
        fprintf(stderr, "%s",  s);
        fflush(stderr);
    }
}

inline void print(const string& s) { print(s.data()); }

inline void swapIfNeeded(size_t& i1, size_t& i2)
{
    if (i1 > i2)
    {
        size_t t = i2;
        i2 = i1;
        i1 = t;
        stringstream ss;
        ss << "WARNING: values of argument '" << i2 << '-' << i1 << " were swapped.";
        print(ss.str());
    }
}

void printTime(const int msc);
bool initCommandLineOptions(boost::program_options::variables_map &vm, int argc, char *argv[]);

#endif // UTILS_H
