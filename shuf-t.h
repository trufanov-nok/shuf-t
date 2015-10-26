#ifndef SHUFFT_H
#define SHUFFT_H

#include <ctime>
#include <string>
#include <cstdio>
#include <vector>
#include "io_buf.h"
#include "metadata.h"
#include "settings.h"

size_t readMetadata(io_buf& src_file, const size_t source_length = 0);
int  writeData(io_buf& in_file, io_buf &out_file);
void shuffleMetadata();

void storeInputRangeToFile(FILE* f, size_t min, size_t max);
std::FILE* openTmpFile();
std::FILE* readStdinToTmpFile();
int openStdOutDestination(io_buf& ts);



#endif // SHUFFT_H
