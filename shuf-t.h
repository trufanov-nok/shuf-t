#ifndef SHUFFT_H
#define SHUFFT_H


#include "metadata.h"
#include <string>
#include <vector>
#include "io_buf.h"

// shuffle settings
extern size_t _param_buffer_size;
extern size_t _param_start_line;
extern size_t _param_end_line;
extern size_t _param_header;
extern size_t _param_output_limit;
extern bool _param_verbose;
extern bool _is_terminal;

// internal data
extern io_buf source_file;
extern io_buf destination_file;
extern string source_string;
extern vector< Block > metadata;




size_t readMetadata(io_buf& src_file, const long long source_length = 0);
int  writeData(io_buf& in_file, io_buf &out_file);
void shuffleMetadata();

int openFileSource(io_buf& ts, const string filename);
int openFileDestination(io_buf& ts, const string filename);
size_t openInputRangeSource(io_buf& ts, size_t range_min = 0, size_t range_max = 0);
FILE *readStdinToTmpFile();
int openStdOutDestination(io_buf& ts);

void closeFileDestination();
void closeFileSource();
void closeInputRangeSource();


#endif // SHUFFT_H
