#ifndef SHUFFT_H
#define SHUFFT_H


#include "metadata.h"
#include "./qforkedtextstream.h"
#include <QTextStream>
#include <QFile>
#include <QVector>
#include <QTemporaryFile>

// shuffle settings
extern qint64 _param_buffer_size;
extern uint _param_start_line;
extern uint _param_end_line;
extern uint _param_header;
extern uint _param_output_limit;
extern bool _param_verbose;

// internal data
extern QFile source_file;
extern QFile destination_file;
extern QString source_string;
extern QVector< Block > metadata;




uint readMetadata(QForkedTextStream& source_stream, const qint64 source_length = 0);
int  writeData(QForkedTextStream& in_stream, QTextStream& out_stream);
void shuffleMetadata();

uint openFileSource(QForkedTextStream& ts, const QString filename);
uint openFileDestination(QTextStream& ts, const QString filename);
uint openInputRangeSource(QForkedTextStream& ts, uint range_min = 0, uint range_max = 0);
QString readStdinToTmpFile();
uint openStdOutDestination(QTextStream& ts);

void closeFileDestination();
void closeFileSource();
void closeInputRangeSource();


#endif // SHUFFT_H
