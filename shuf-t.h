#ifndef SHUFFT_H
#define SHUFFT_H


#include "metadata.h"
#include <QVector>
#include <QTemporaryFile>
#include "io_buf.h"

// shuffle settings
extern qint64 _param_buffer_size;
extern uint _param_start_line;
extern uint _param_end_line;
extern uint _param_header;
extern uint _param_output_limit;
extern bool _param_verbose;
extern bool _is_terminal;

// internal data
extern io_buf source_file;
extern io_buf destination_file;
extern QString source_string;
extern QVector< Block > metadata;




uint readMetadata(io_buf& src_file, const qint64 source_length = 0);
int  writeData(io_buf& in_file, io_buf &out_file);
void shuffleMetadata();

int openFileSource(io_buf& ts, const QString filename);
int openFileDestination(io_buf& ts, const QString filename);
uint openInputRangeSource(io_buf& ts, uint range_min = 0, uint range_max = 0);
QString readStdinToTmpFile();
int openStdOutDestination(io_buf& ts);

void closeFileDestination();
void closeFileSource();
void closeInputRangeSource();


#endif // SHUFFT_H
