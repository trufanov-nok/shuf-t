#include "shuf-t.h"
#include "utils.h"
#include <QDebug>

//global variables initialization

qint64 _param_buffer_size = 1024*1024*1024; //1 Gib
uint _param_start_line = 0;
uint _param_end_line = 0;
uint _param_header = 0;
uint _param_output_limit = 0;
bool _param_verbose = true;

QFile source_file;
QFile destination_file;
QString source_string;
QVector< Block > metadata;

bool is_temporary_file = false;
bool remove_trailing_empty_line = true;

uint readMetadata(QForkedTextStream& source_stream, const qint64 source_length)
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

    uint skip_first_lines = std::max(_param_header,_param_start_line);

    while (!source_stream.atEnd())
    {
        source_stream.readLine();


        qint64 pos_end = source_stream.getFastPos();
        if (skip_first_lines <= ++lines_processed)
        {
            uint strlen = source_stream.getBytesRead();
            metadata.append( Block(pos_start, strlen) );
        }

        pos_start = pos_end;


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

int writeData(QForkedTextStream& in_stream, QTextStream& out_stream)
{
    int result = -1;
    char * mem = (char*) malloc(_param_buffer_size);
    if(mem == NULL)
    {
        qCritical() << "can't allocate enough memory";
        return result;
    }

    QByteArray buffer;
    buffer = buffer.setRawData(mem, _param_buffer_size);

    in_stream.seek(0);

    QTextStream buffer_stream(&buffer, QIODevice::ReadWrite);

    // copy header from the beginning of input
    while (_param_header--)
        out_stream << in_stream.readLine() + "\n";

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
        uint buffer_size_left = _param_buffer_size;

        uint blocks_marked = 0;


        // mark as many blocks as could fit our buffer

        while (blocks_marked < blocks_left)
        {
            Block& next_block = metadata[blocks_marked];
            uint block_len = next_block.length + 1; // +1 for "\n"

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
        buffer_stream.seek(0);
        qint64 pos_buffer = 0;
        qint64 pos_input  = in_stream.getFastPos();
        //QHelperTextStream in_helper(in_stream);

        foreach (Block2Buf block, blocks_to_read)
        {
            if (pos_input != block.offset_read)
            {
                if(!in_stream.seek(block.offset_read)) //always forward
                {
                    result = -1;
                    break;
                }

                pos_input = block.offset_read;
            }

            if (pos_buffer != block.offset_write)
            {
                if (!buffer_stream.seek(block.offset_write)) //any direction
                {
                    result = -1;
                    break;
                }

                pos_buffer = block.offset_write;
            }


            buffer_stream << in_stream.readLine()+"\n";
            qint64 strlen = in_stream.getBytesRead();
            strlen += 1; //for '\n
            pos_buffer += strlen;
            pos_input = in_stream.getFastPos();
        }


        // save buffer to output
        buffer_stream.seek(0);
        pos_buffer = 0;
        qint64 buffer_size_used = _param_buffer_size - buffer_size_left;


        while (pos_buffer < buffer_size_used)
        {
            int read_len =  std::min(QTEXTSTREAM_BUFFERSIZE, (int) (buffer_size_used - pos_buffer));
            out_stream << buffer_stream.read(read_len);
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


    print("\n");

    free(mem);

    return result;

}


uint openFileSource(QForkedTextStream& ts, const QString filename)
{
    source_file.close();
    source_file.setFileName(filename);
    int result = -1;
    if (source_file.open(QIODevice::ReadOnly))
    {
        ts.setDevice(&source_file);
        ts.seek(0);
        result = readMetadata(ts, source_file.size()-1);
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
    if (!is_temporary_file)
            source_file.close();
    else
            source_file.remove();
}

uint openFileDestination(QTextStream& ts, const QString filename)
{
    destination_file.close();
    destination_file.setFileName(filename);
    int result = -1;
    if (destination_file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        ts.setDevice(&destination_file);
        ts.seek(0);
    }

    return result;
}

uint openInputRangeSource(QForkedTextStream& ts, uint range_min, uint range_max)
{
    source_string.clear();
    QStringList sl;
    for (uint i = range_min; i <= range_max; i++)
        sl.append(QString::number(i));
    source_string = sl.join("\n");
    ts.setString(&source_string, QIODevice::ReadOnly);
    return readMetadata(ts, source_string.data_ptr()->size);
}

void closeInputRangeSource()
{
    source_string.clear();
}

uint openStdOutDestination(QTextStream& ts)
{
   if (!destination_file.open(stdout, QIODevice::WriteOnly))
       return -1;

   ts.setDevice(&destination_file);
   return 0;
}

void closeFileDestination()
{
    destination_file.close();
}




