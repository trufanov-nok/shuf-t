#include "shuf-t.h"
#include "utils.h"
#include <QDebug>
#include <QTextCodec>

//global variables initialization

qint64 _param_buffer_size = 1024*1024*1024; //1 Gib
uint _param_start_line = 0;
uint _param_end_line = 0;
uint _param_header = 0;
uint _param_output_limit = 0;
bool _param_verbose = true;
bool _is_terminal = false;

io_buf source_file;
io_buf destination_file;
QString source_string;
QVector< Block > metadata;

bool is_temporary_file = false;
bool remove_trailing_empty_line = true;

uint readMetadata(io_buf& src_file, const qint64 source_length)
{
    double progress_step = 0;

    if (_param_end_line > 0)
        progress_step = (double) _param_end_line / 10;  //progress by lines read
    else
        progress_step = (double) source_length   / 10;  //progress by bytes read

    if (progress_step <= 0) progress_step = 1; //file length 0 or 1

    metadata.clear();

    if (_param_end_line > 0)
    {
        metadata.reserve(_param_end_line - std::max(_param_start_line, (uint) 1) + 1);
    }


    double progress = 0;

    // scan file for line offsets
    qint64 pos_start = 0;
    uint lines_processed = 0;

    uint skip_first_lines = std::max(_param_header+1,_param_start_line);

    char *line = NULL;
    size_t len;
    while (len = readto(src_file, line, '\n'))
    {
        if (skip_first_lines <= ++lines_processed)
            metadata.append( Block(pos_start, len) );

        pos_start += len;

        double current_progress;
        if (_param_end_line > 0)
            current_progress = lines_processed; // progress by lines read
        else
            current_progress = pos_start;       // progress by bytes read

        while (current_progress - progress > 1e-5)
        {
            print(".");
            progress += progress_step;

        }


        if(_param_end_line > 0 && lines_processed >= _param_end_line) break;

    }

    print("\n");

    if (remove_trailing_empty_line)
        if (metadata.count() > 0 && metadata.last().length == 0)
            metadata.removeLast();


    return 0;
}

//// Fisher–Yates shuffle
//// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm

void shuffleMetadata()
{
    double blocks_processed = 1; // n-1 loops
    uint n = metadata.count();
    double progress_step = (double) n/10;
    double progress = 0;

    if (n > 0)
        for (uint i = n-1; i > 0; i--)
        {
            uint j = rand() % (i+1);
            if (j != i)
            {
                Block b     = metadata[i];
                metadata[i] = metadata[j];
                metadata[j] = b;
            }

            blocks_processed++;
            while( blocks_processed - progress > 1e-5)
            {
                print(".");
                progress += progress_step;
            }
        }

    print("\n");
}

//const char* eoln= "\n";

int writeData(io_buf& in_file, io_buf& out_file)
{
    int result = -1;
    char * int_buffer = (char*) malloc(_param_buffer_size);

    if(int_buffer == NULL)
    {
        qCritical() << "can't allocate enough memory";
        return result;
    }

    in_file.seek(0);
    char *line = NULL;

    // copy header from the beginning of input
    qint64 pos_input = 0;
    while (_param_header--)
    {
        size_t len = readto(in_file, line, '\n');
        pos_input += len;
        bin_write_fixed(out_file, line, len);
//        bin_write_fixed(out_file, eoln, 1);
    }

    // copy data
    uint total_blocks = metadata.count();
    uint blocks_left;
    if (_param_output_limit > 0)
        blocks_left = std::min(_param_output_limit, total_blocks);
    else
        blocks_left = total_blocks;

    double blocks_processed = 0;
    double progress_step = (double) blocks_left/10.;
    double progress = 0;
    result = 0;

    while (result == 0 && blocks_left > 0)
    {
        QVector<Block2Buf> blocks_to_read;
        qint64 buffer_size_left = _param_buffer_size;

        qint64 blocks_marked = 0;


        // mark as many blocks as could fit our buffer

        while (blocks_marked < blocks_left)
        {
            Block& next_block = metadata[blocks_marked];
            qint64 block_len = next_block.length;

            if (block_len <= buffer_size_left)
            {   // block still can fit the buffer
                blocks_to_read.append(Block2Buf(next_block.offset, _param_buffer_size-buffer_size_left));
                buffer_size_left -= block_len;

                blocks_marked++;
            } else
            {   // buffer is filled
                break;
            }
        }


        if(blocks_marked == 0)
        {   // nothing was marked
            if (blocks_left > 0)
            {   // but there was data to write
                print("error: block is bigger than buffer"); //need to return -1
                result = -1;
            }
            break;
        } else {
            metadata.remove(0, blocks_marked);
            blocks_left -= blocks_marked;
        }

        // sort blocks by offset toread them from input with one scan
        std::sort(blocks_to_read.begin(), blocks_to_read.end());

        // read blocks from input to buffer

        foreach (Block2Buf block, blocks_to_read)
        {
            if (pos_input != block.offset_read)
            {
                if(in_file.seek(block.offset_read) == -1) //always forward
                {
                    result = -1;
                    print("internal error\n");
                    break;
                }

                pos_input = block.offset_read;
            }

            size_t len = readto(in_file, line, '\n');
            pos_input += len;

            memcpy( int_buffer+block.offset_write, line, len);
//            int_buffer[block.offset_write+len] = '\n';
        }


        // save buffer to output
        size_t pos_buffer = 0;
        size_t buffer_size_used = _param_buffer_size - buffer_size_left;


        while (pos_buffer < buffer_size_used)
        {
            int read_len =  std::min(QTEXTSTREAM_BUFFERSIZE, (int) (buffer_size_used - pos_buffer));
            char*  buf = int_buffer+pos_buffer;
            bin_write_fixed(out_file, buf, read_len);
            pos_buffer += read_len;
        }

        // progress by blocks
        if (_param_output_limit > 0)
            blocks_processed = std::min(_param_output_limit,total_blocks) - blocks_left;
        else
            blocks_processed = total_blocks - blocks_left;

        while (blocks_processed - progress > 1e-5)
        {
            progress += progress_step;
            print(".");
        }


    }

    out_file.flush();

    print("\n");

    free(int_buffer);

    return result;

}


int openFileSource(io_buf &ts, const QString filename)
{
    ts.close_file();
    int result = -1;
    if ( (result = ts.open_file(filename.toStdString().c_str(), false)) != -1)
    {
        ts.reset();

        result = readMetadata(ts, ts.size());
    }

    return result;
}

QString readStdinToTmpFile()
{
    QString fname;
    QTemporaryFile f;
    f.setAutoRemove(false);
    if (f.open())
    {
        fname = f.fileName();
        QTextStream f_stream(&f);
        QFile f2;
        if (f2.open(stdin, QIODevice::ReadOnly))
        {
            QTextStream f2_stream(&f2);
            while(!f2_stream.atEnd())
            {
                f_stream << f2_stream.readLine() + '\n';
            }
            f2.close();

        } else
        {
            f.close();
            qCritical() << "ERROR: Can't open stdin";
            return QString();
        }

        f.close();
    } else {
        qCritical() << "ERROR: Can't create a temp file " + fname;
        return QString();
    }

    is_temporary_file = true;
    return fname;
}

void closeFileSource()
{

    source_file.close_file();
    if (is_temporary_file)
    {
//        source_file.remove(); TODO
    }
}

int openFileDestination(io_buf& ts, const QString filename)
{
    ts.close_file();
    int result = -1;
    if ( (result = ts.open_file(filename.toStdString().c_str(), false, io_buf::WRITE)) != -1)
    {
        ts.reset();
    }

    return result;
}

uint openInputRangeSource(io_buf& ts, uint range_min, uint range_max)
{
    //TODO
//    source_string.clear();
//    QStringList sl;
//    for (uint i = range_min; i <= range_max; i++)
//        sl.append(QString::number(i));
//    source_string = sl.join("\n");
//    ts.setString(&source_string, QIODevice::ReadOnly);
//    return readMetadata(ts, source_string.data_ptr()->size);
}

void closeInputRangeSource()
{
    source_string.clear();
}

int openStdOutDestination(io_buf& ts)
{
    return ts.open_file(NULL, true);
}

void closeFileDestination()
{
    destination_file.close_file();
}




