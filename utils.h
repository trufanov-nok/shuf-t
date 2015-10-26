#ifndef UTILS_H
#define UTILS_H

#include "settings.h"
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


inline void print(const char* s)
{
    if (settings.verbose)
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

inline bool getRangeArgument(const string arg, size_t& i1, size_t& i2)
{
    vector<string> s = split(arg, '-');
    if (!(s.size() != 2) && (s[1].size() <= 0))
        return false;

    try
    {
        i1 = atol( s[0].data() );
    }
    catch(...) { return false; }

    try
    {
        i2 = atol( s[1].data() );
    }
    catch(...) { return false; }

    return true;
}

void printTime(const double msc);
bool initCommandLineOptions(boost::program_options::variables_map &vm, int argc, char *argv[]);

#endif // UTILS_H
