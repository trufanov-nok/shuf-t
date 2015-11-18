/*
Copyright (c) by Alexander Trufanov. Released under a Simplified BSD
license as described in the file LICENSE.
 */
#include "shuf-t.h"
#include "utils.h"

bool remove_trailing_empty_line = true;

size_t readMetadata(io_buf& src_file, const size_t source_length)
{

    // selecting progress step

    double progress_step = 0;
    bool progress_by_lines = false;

    switch (settings.src)
    {
    case SOURCE_INPUT_RANGE:
        progress_by_lines  = true;
        IRData* ir = (IRData*) settings.src_data;
        double lines_to_generate = fmin(ir->max - ir->min, settings.end_line);
        progress_step = lines_to_generate / 10;
        double lines_to_shuffle = lines_to_generate - settings.header- std::max(settings.start_line, (size_t) 1) + 1;
        if (lines_to_shuffle > 0)
        settings.metadata.reserve( (size_t)lines_to_shuffle );
        break;
    case SOURCE_FILE:
        if (settings.end_line > 0)
        {
            progress_by_lines = true;
            progress_step = (double) settings.end_line / 10;  //progress by lines read
            settings.metadata.reserve(settings.end_line - std::max(settings.start_line, (size_t) 1) + 1);
        }
        else
            progress_step = (double) source_length   / 10;  //progress by bytes read

        break;
    case SOURCE_STDIN:

    }

    if (settings.src == SOURCE_INPUT_RANGE)
    {

    } else
        if (settings.src == SOURCE_FILE && )

    if (progress_step <= 0) progress_step = 1; //file length 0 or 1

    settings.metadata.clear();


    double progress = 0;

    // scan file for line offsets
    size_t pos_start = 0;
    size_t lines_processed = 0;

    size_t skip_first_lines = std::max(settings.header+1,settings.start_line);

    char *line = NULL;
    size_t len;
    bool first_line_to_read_found = false;

    io_buf* dest_file = NULL;
    if (settings.src != SOURCE_FILE) dest_file = &src_file;

    while ((len = readto(src_file, line, '\n')))
    {
        if (skip_first_lines <= ++lines_processed)
        {
            settings.metadata.push_back( Block(pos_start, len) );
            if (!first_line_to_read_found)
            {
                settings.first_line_to_read = pos_start;
                first_line_to_read_found = true;
            }
        }

        pos_start += len;

        double current_progress;
        if (settings.end_line > 0)
            current_progress = lines_processed; // progress by lines read
        else
            current_progress = pos_start;       // progress by bytes read

        while (current_progress - progress > 1e-5)
        {
            print(".");
            progress += progress_step;

        }


        if (settings.end_line > 0 && lines_processed >= settings.end_line) break;

        if (dest_file) // used for stdin and inut_range
            bin_write(*dest_file, line, len);
    }

    settings.end_line = lines_processed;

    print("\n");

    if (remove_trailing_empty_line)
        if (settings.metadata.size() > 0 && (settings.metadata.end()-1)->length == 0)
        {
            settings.metadata.pop_back();
            settings.end_line--;
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
        for (size_t i = n-1; i > 0; --i)
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

int splitMetaData()
{
    // copy data
    if ((settings.output_limit > 0) && (settings.output_limit < settings.metadata.size()))
    {
        settings.metadata.erase(settings.metadata.begin()+settings.output_limit, settings.metadata.end());
        settings.end_line = settings.output_limit;
        settings.output_limit = 0;
    }

    size_t blocks_left = settings.metadata.size();

    size_t block_idx = 0;
    size_t rolling_offset = 0;

    while (block_idx < blocks_left)
    {
        size_t buffer_size_left = settings.buffer_size;

        // mark as many blocks as could fit our buffer
        while (block_idx < blocks_left)
        {
            Block& b = settings.metadata[block_idx];
            size_t block_len = b.length;

            if (block_len <= buffer_size_left)
            {   // block still can fit the buffer
                b.offset_write = rolling_offset;
                buffer_size_left -= block_len;
                block_idx++;
                rolling_offset += block_len;
            } else
            {   // buffer is filled
                settings.metadata_split.push_back(rolling_offset);
                break;
            }
        }


        if (buffer_size_left == settings.buffer_size)
        {   // nothing was marked
            if (block_idx < blocks_left)
            {   // but there was data to write
                print("error: block is bigger than buffer"); //need to return -1
                return -1;
            }
            break;
        }


    }

    return 0;
}

inline int get_split_index(const size_t offset)
{
    assert(!settings.metadata_split.empty());
    for (int i = 0; i < (int)settings.metadata_split.size(); ++i)
        if (offset < settings.metadata_split[i]) return i-1;

    return settings.metadata_split.size()-1;
}

int splitData(io_buf& in_file, io_buf& out_file)
{

    // sort metadata by offset_read
    std::sort(settings.metadata.begin(), settings.metadata.end());

    // first buffer can be assembled from input file
    char* int_buffer = (char*) malloc(settings.buffer_size);

    if(int_buffer == NULL)
    {
        fprintf(stderr, "can't allocate enough memory");
        return -1;
    } else settings.buffer = int_buffer;


    // prepare temp files
    const bool no_splits = settings.metadata_split.empty();
    if (!no_splits)
        for (size_t i = 0; i < settings.metadata_split.size(); ++i)
        {
            std::FILE* f = openTmpFile();
            if (!f) return -1;
            io_buf* io = new io_buf();
            if (!io) return -1;
            io->file = fileno(f);
            settings.temp_files.push_back(io);
        }


    in_file.reset();

    // copy header from the beginning of input
    char *line = NULL;
    size_t lines_processed = settings.header;
    for (size_t i = 0; i < lines_processed; ++i)
    {
        size_t len = readto(in_file, line, '\n');
        bin_write_fixed(out_file, line, len);
    }

    // copy data
    double progress_step = (double) settings.end_line / 10;  //progress by lines read

    if (progress_step <= 0) progress_step = 1; //file length 0 or 1

    lines_processed = std::max(settings.header, settings.start_line);
    double progress = lines_processed;

    in_file.endloaded = in_file.space.begin; // reset io buffer
    in_file.space.end = in_file.space.begin; // reset io buffer
    in_file.seek(settings.first_line_to_read);

    size_t metadata_idx = 0;
    while (buf_read_b(in_file, line, settings.metadata[metadata_idx].length))
    {

        Block& b = settings.metadata[metadata_idx++];

        int split_index;
        if (no_splits || ((split_index = get_split_index(b.offset_write)) == -1))
        { // can write to the memory buf
            assert(b.offset_write < settings.buffer_size);
            memcpy( settings.buffer+b.offset_write, line, b.length);
            settings.buffer_used = max(settings.buffer_used, b.offset_write + b.length );
        } else {
            // have to write to a temp file
            io_buf& sf = *settings.temp_files[split_index];
            bin_write_fixed(sf, line, b.length);
        }

        lines_processed++;

        while (lines_processed - progress > 1e-5)
        {
            print(".");
            progress += progress_step;

        }

        if (lines_processed >= settings.end_line)
            break;

    }


    for (size_t i = 0; i < settings.temp_files.size(); ++i)
        settings.temp_files[i]->flush();


    print("\n");

    return 0;

}

std::vector<Block>::iterator getSubrangeEnd(size_t offset)
{    
    std::vector<Block>::iterator it;
    for (it = settings.metadata.begin(); it < settings.metadata.end(); ++ it)
        if (it->offset_write >= offset) return it;

    return settings.metadata.end();
}

void DumpBuffer(io_buf& out_file)
{
    size_t pos_buffer = 0;
    while (pos_buffer < settings.buffer_used)
    {
        size_t write_len =  min(QTEXTSTREAM_BUFFERSIZE, settings.buffer_used - pos_buffer);
        char*  buf = settings.buffer + pos_buffer;
        bin_write_fixed(out_file, buf, write_len);
        pos_buffer += write_len;
    }

    out_file.flush();
}

int writeData(io_buf& out_file)
{
    int result = -1;

    // write first buffer that already assembled in memory
    DumpBuffer(out_file);

    // assemble rest chunks from slit and write them
    
    if (!settings.temp_files.empty())
    {
        // to group metadata to blocks
        std::sort(settings.metadata.begin(), settings.metadata.end(), Block::sort_by_write_offset);


        // erase first block as it's already stored
        std::vector<Block>::iterator block_end;
        block_end = getSubrangeEnd(settings.metadata_split[0]);
        settings.metadata.erase(settings.metadata.begin(), block_end);
        assert(settings.metadata_split[0] == settings.metadata[0].offset_write);
        settings.metadata_split.erase(settings.metadata_split.begin());



        size_t total_blocks = settings.metadata.size();
        double progress_step = total_blocks / 10.;
        size_t blocks_left = total_blocks;
        size_t blocks_processed= 0; double progress = 0;

        while (!settings.temp_files.empty())
        {
            io_buf& split = *settings.temp_files[0];

            if (settings.metadata_split.empty()) // last bufer
                block_end = settings.metadata.end();
            else
                block_end = getSubrangeEnd(settings.metadata_split[0]);

            const size_t offset_shift = settings.metadata[0].offset_write;
            settings.buffer_used = (block_end-1)->offset_write+(block_end-1)->length;
            settings.buffer_used -= offset_shift;
            assert(settings.buffer_used <= settings.buffer_size);
            // sort block lines as they were written to chunk
            std::sort(settings.metadata.begin(), block_end);

            split.reset();

            size_t metadata_idx = 0;
            char *line = NULL;

            while (metadata_idx < settings.metadata.size() && buf_read_b(split, line, settings.metadata[metadata_idx].length))
            {
                Block& b = settings.metadata[metadata_idx++];
                if (!settings.metadata_split.empty())
                    assert(get_split_index(b.offset_write) == -1);
                b.offset_write -= offset_shift;
                assert(b.offset_write < settings.buffer_size);
                memcpy( settings.buffer+b.offset_write, line, b.length);

                blocks_left--;

                // progress by blocks
                blocks_processed = total_blocks - blocks_left;

                while (blocks_processed - progress > 1e-5)
                {
                    progress += progress_step;
                    print(".");
                }
            }

            DumpBuffer(out_file);


            //erase block's metadata
            split.close_file();
            delete settings.temp_files[0];
            settings.temp_files.erase(settings.temp_files.begin());
            if (!settings.metadata_split.empty())
                settings.metadata_split.erase(settings.metadata_split.begin());
            settings.metadata.erase(settings.metadata.begin(), block_end);

        }

        assert(settings.temp_files.empty());
        assert(settings.metadata_split.empty());
        assert(settings.metadata.empty());
    }

    print("\n");
    
    result = 0;
    return result;

}


std::FILE* openTmpFile()
{
    std::FILE* f = std::tmpfile();
    if (f == NULL)
        fprintf(stderr, "ERROR: Can't create a temporary file.");
    return f;
}

//std::FILE* readStdinToTmpFile()
//{
//    std::FILE* f = openTmpFile();
//    if (f)
//    {
//        for (std::string line; std::getline(std::cin, line);)
//        {
//            std::fputs(line.data(), f);
//        }
//        std::rewind(f);
//    }

//    return f;
//}


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





