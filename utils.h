#ifndef UTILS_H
#define UTILS_H


#include "./qforkedtextstream.h"
#include <math.h>
#include <QCommandLineParser>


static const int QTEXTSTREAM_BUFFERSIZE = 16384;

extern bool _param_verbose;
inline void print(const char* s)
{
    if (_param_verbose)
    {
        fprintf(stdout, "%s",  s);
        fflush(stdout);
    }

}

//inline void print(const char s)
//{
//    char c[2]; //some problems on release bulds
//    c[0] = s; c[1] = 0x0;
//    print(&c[0]);
//}

inline void print(const QString s)
{
    print( s.toStdString().c_str() );
}

inline void swapIfNeeded(uint& i1, uint& i2)
{
    if (i1 > i2)
    {
        uint t = i2;
        i2 = i1;
        i1 = t;
        print("WARNING: values of argument '" + QString::number(i2)+'-'+QString::number(i1)+" were swapped.");
    }
}

void printTime(const int msc);
void initCommandOptionsParser(QCommandLineParser &parser);
int getLineDelimiterLength(QForkedTextStream &ts);

#endif // UTILS_H
