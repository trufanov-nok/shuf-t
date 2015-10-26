#include "shuf-t.h"
#include "utils.h"
#include <cstdio>

//global variables initialization

size_t _param_buffer_size = 1024*1024*1024; //1 Gib
size_t _param_start_line = 0;
size_t _param_end_line = 0;
size_t _param_header = 0;
size_t _param_output_limit = 0;
bool _param_verbose = true;
bool _is_terminal = false;

io_buf source_file;
io_buf destination_file;
string source_string;
vector< Block > metadata;

bool is_temporary_file = false;
bool remove_trailing_empty_line = true;

size_t readMetadata(io_buf& src_file, const size_t source_length)
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
        metadata.reserve(_param_end_line - std::max(_param_start_line, (size_t) 1) + 1);
    }


    double progress = 0;

    // scan file for line offsets
    size_t pos_start = 0;
    size_t lines_processed = 0;

    size_t skip_first_lines = std::max(_param_header+1,_param_start_line);

    char *line = NULL;
    size_t len;
    while ((len = readto(src_file, line, '\n')))
    {
        if (skip_first_lines <= ++lines_processed)
            metadata.push_back( Block(pos_start, len) );

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
        if (metadata.size() > 0 && (metadata.end()-1)->length == 0)
            metadata.pop_back();


    return 0;
}

//// Fisher–Yates shuffle
//// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm

void shuffleMetadata()
{
    double blocks_processed = 1; // n-1 loops
    size_t n = metadata.size();
    double progress_step = (double) n/10;
    double progress = 0;

    if (n > 0)
        for (size_t i = n-1; i > 0; i--)
        {
            size_t j = rand() % (i+1);
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


int writeData(io_buf& in_file, io_buf& out_file)
{
    int result = -1;
    char * int_buffer = (char*) malloc(_param_buffer_size);

    if(int_buffer == NULL)
    {
        fprintf(stderr, "can't allocate enough memory");
        return result;
    }

    in_file.seek(0);
    char *line = NULL;

    // copy header from the beginning of input
    size_t pos_input = 0;
    while (_param_header--)
    {
        size_t len = readto(in_file, line, '\n');
        pos_input += len;
        bin_write_fixed(out_file, line, len);
    }

    // copy data
    size_t total_blocks = metadata.size();
    size_t blocks_left;
    if (_param_output_limit > 0)
        blocks_left = min(_param_output_limit, total_blocks);
    else
        blocks_left = total_blocks;

    double blocks_processed = 0;
    double progress_step = (double) blocks_left/10.;
    double progress = 0;
    result = 0;

    while (result == 0 && blocks_left > 0)
    {
        vector<Block2Buf> blocks_to_read;
        size_t buffer_size_left = _param_buffer_size;

        size_t blocks_marked = 0;


        // mark as many blocks as could fit our buffer

        while (blocks_marked < blocks_left)
        {
            Block& next_block = metadata[blocks_marked];
            size_t block_len = next_block.length;

            if (block_len <= buffer_size_left)
            {   // block still can fit the buffer
                blocks_to_read.push_back(Block2Buf(next_block.offset, _param_buffer_size-buffer_size_left));
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
            metadata.erase(metadata.begin(), metadata.begin()+blocks_marked);
            blocks_left -= blocks_marked;
        }

        // sort blocks by offset toread them from input with one scan
        std::sort(blocks_to_read.begin(), blocks_to_read.end());

        // read blocks from input to buffer

        for(size_t i = 0; i < blocks_to_read.size(); ++i)
        {
            Block2Buf& block = blocks_to_read[i];
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
        }


        // save buffer to output
        size_t pos_buffer = 0;
        size_t buffer_size_used = _param_buffer_size - buffer_size_left;


        while (pos_buffer < buffer_size_used)
        {
            int write_len =  min(QTEXTSTREAM_BUFFERSIZE, (int) (buffer_size_used - pos_buffer));
            char*  buf = int_buffer+pos_buffer;
            bin_write_fixed(out_file, buf, write_len);
            pos_buffer += write_len;
        }

        // progress by blocks
        if (_param_output_limit > 0)
            blocks_processed = min(_param_output_limit,total_blocks) - blocks_left;
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


int openFileSource(io_buf &ts, const string filename)
{
    ts.close_file();
    int result = -1;
    if ( (result = ts.open_file(filename.data())) != -1)
    {
        ts.reset();
        result = readMetadata(ts, ts.size());
    }

    return result;
}

std::FILE* readStdinToTmpFile()
{
    std::FILE* f = std::tmpfile();
    if (f != NULL)
    {
        for (std::string line; std::getline(std::cin, line);)
         {
            std::fputs(line.data(), f);
         }
        std::rewind(f);

    } else {
        fprintf(stderr, "ERROR: Can't create a temporary file.");
    }

    is_temporary_file = true;
    return f;
}

void closeFileSource()
{

    source_file.close_file();
    if (is_temporary_file)
    {
//        source_file.remove(); TODO
    }
}

int openFileDestination(io_buf& ts, const string filename)
{
    ts.close_file();
    int result = -1;
    if ( (result = ts.open_file(filename.data(), io_buf::WRITE)) != -1)
    {
        ts.reset();
    }

    return result;
}

size_t openInputRangeSource(io_buf& ts, size_t range_min, size_t range_max)
{

    source_string.clear();
    for (size_t i = range_min; i <= range_max; i++)
    {
        if (!source_string.empty())
            source_string.push_back('\n');
        source_string.append(to_string(i));
    }
//TODO
    //ts.setString(&source_string, QIODevice::ReadOnly);
//    return readMetadata(ts, source_string.size());
}

void closeInputRangeSource()
{
    source_string.clear();
}

int openStdOutDestination(io_buf& ts)
{
    return ts.open_file(NULL, io_buf::WRITE);
}

void closeFileDestination()
{
    destination_file.close_file();
}




