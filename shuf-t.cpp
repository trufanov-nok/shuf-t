/*
Copyright (c) by Alexander Trufanov. Released under a Simplified BSD
license as described in the file LICENSE.
 */
#include "shuf-t.h"
#include "utils.h"
#include <algorithm> // std::sort

bool remove_trailing_empty_line = true;

int readMetadata(io_buf& src_file, const size_t source_length)
{
    double progress_step = 0;

    if (settings.end_line > 0)
        progress_step = (double) settings.end_line / 10;  //progress by lines read
    else
        progress_step = (double) source_length   / 10;  //progress by bytes read

    if (progress_step <= 0) progress_step = 1; //file length 0 or 1

    settings.metadata.clear();

    if (settings.end_line > 0)
    {
        settings.metadata.reserve(settings.end_line - max(settings.start_line, (size_t) 1) + 1);
    }


    double progress = 0;

    // scan file for line offsets
    size_t pos_start = 0;
    size_t lines_processed = 0;

    size_t skip_first_lines = max(settings.header+1,settings.start_line);

    char *line = NULL;
    size_t len;
    bool last_line_teminator_found = false;

    while ((len = readto(src_file, line, '\n')))
    {
        if (skip_first_lines <= ++lines_processed)
            settings.metadata.push_back( Block(pos_start, len) );

        last_line_teminator_found = *(line+len-1) == '\n';

        pos_start += len;

        double current_progress;
        if (settings.end_line > 0)
            current_progress = (double)lines_processed; // progress by lines read
        else
            current_progress = (double)pos_start;       // progress by bytes read

        while (current_progress - progress > 1e-5)
        {
            print(".");
            progress += progress_step;

        }


        if(settings.end_line > 0 && lines_processed >= settings.end_line) break;

    }

    print("\n");


    if (settings.metadata.size() > 0)
    {
        Block& last_block = *(settings.metadata.end()-1);
        if (last_block.length == 0)
        {
            if (remove_trailing_empty_line)
                settings.metadata.pop_back();
            else
                last_block.length++;
        } else
            if (!last_line_teminator_found) last_block.length++; //reserve a space for missing last line terminator
    }


    return 0;
}

//// Fisher–Yates shuffle
//// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm

void shuffleMetadata()
{
    double blocks_processed = 1; // n-1 loops
    size_t n = settings.metadata.size();
    double progress_step = (double) n/10;
    double progress = 0;

    if (n > 0)
        for (size_t i = n-1; i > 0; i--)
        {
            size_t j = rand() % (i+1);
            if (j != i)
            {
                Block b     = settings.metadata[i];
                settings.metadata[i] = settings.metadata[j];
                settings.metadata[j] = b;
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
    char * int_buffer = (char*) malloc(settings.buffer_size);

    if(int_buffer == NULL)
    {
        fprintf(stderr, "can't allocate enough memory");
        return result;
    }

    in_file.seek(0);
    char *line = NULL;

    // copy header from the beginning of input
    size_t pos_input = 0;
    while (settings.header--)
    {
        size_t len = readto(in_file, line, '\n');
        pos_input += len;
        bin_write_fixed(out_file, line, (uint32_t)len);
    }

    // copy data
    size_t total_blocks = settings.metadata.size();
    size_t blocks_left;
    if (settings.output_limit > 0)
        blocks_left = min(settings.output_limit, total_blocks);
    else
        blocks_left = total_blocks;

    double blocks_processed = 0;
    double progress_step = (double) blocks_left/10.;
    double progress = 0;
    result = 0;

    while (result == 0 && blocks_left > 0)
    {
        vector<Block2Buf> blocks_to_read;
        size_t buffer_size_left = settings.buffer_size;

        size_t blocks_marked = 0;


        // mark as many blocks as could fit our buffer

        while (blocks_marked < blocks_left)
        {
            Block& next_block = settings.metadata[blocks_marked];
            size_t block_len = next_block.length;

            if (block_len <= buffer_size_left)
            {   // block still can fit the buffer
                blocks_to_read.push_back(Block2Buf(next_block.offset, settings.buffer_size-buffer_size_left));
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
            settings.metadata.erase(settings.metadata.begin(), settings.metadata.begin()+blocks_marked);
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
                if(in_file.seek((long)block.offset_read) == -1) //always forward
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

            if (blocks_left == 0 && i == blocks_to_read.size()-1) //last line processing
                if (len == 0 || *(line+len-1) != '\n') //last line has no trailing line terminator (space already reserved)
                    *(int_buffer+block.offset_write+len) = '\n';
        }


        // save buffer to output
        size_t pos_buffer = 0;
        size_t buffer_size_used = settings.buffer_size - buffer_size_left;


        while (pos_buffer < buffer_size_used)
        {
            int write_len =  min(QTEXTSTREAM_BUFFERSIZE, (int) (buffer_size_used - pos_buffer));
            char*  buf = int_buffer+pos_buffer;
            bin_write_fixed(out_file, buf, write_len);
            pos_buffer += write_len;
        }

        // progress by blocks
        if (settings.output_limit > 0)
            blocks_processed = (double) min(settings.output_limit,total_blocks) - blocks_left;
        else
            blocks_processed = (double) total_blocks - blocks_left;

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


std::FILE* openTmpFile()
{
    std::FILE* f = std::tmpfile();
    if (f == NULL)
        fprintf(stderr, "ERROR: Can't create a temporary file.");
    return f;
}

std::FILE* readStdinToTmpFile()
{
    std::FILE* f = openTmpFile();
    if (f)
    {
        for (std::string line; std::getline(std::cin, line);)
        {
            std::fputs(line.data(), f);
        }
        std::rewind(f);
    }

    return f;
}


void storeInputRangeToFile(FILE *f, size_t min, size_t max)
{    

    stringstream ss;
    for (size_t i = min; i <= max; i++)
    {
        ss.str(std::string());
        ss << i << std::endl;
        std::fputs(ss.str().data(), f);
    }

    std::rewind(f);
}





