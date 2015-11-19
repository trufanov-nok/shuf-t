/*
Copyright (c) by Alexander Trufanov. Released under a Simplified BSD
license as described in the file LICENSE.
 */
#ifndef SETTINGS_H
#define SETTINGS_H
#include <vector>
#ifdef WIN32
#include <io.h>
#endif
#include <cstdio>
#include "io_buf.h"
#include "metadata.h"

enum ShuftSources
{
    SOURCE_STDIN,
    SOURCE_FILE,
    SOURCE_INPUT_RANGE
};

enum SuftDestinations
{
    DEST_STDOUT,
    DEST_FILE
};

struct FileData
{
private:
    string _filename;
public:
    io_buf file_stream;
    string filename() {return _filename;}
    int setFilename(string fn, int flag = io_buf::READ)
    {
        _filename = fn;
        return file_stream.open_file(fn.data(), flag);
    }
};

struct TempFileData
{
private:
    std::FILE* _ptr;
public:
    io_buf file_stream;
    std::FILE* ptr() {return _ptr;}
    void setPtr(std::FILE* f)
    {
       _ptr = f;
       if (f) file_stream.file = fileno(_ptr);
    }

};

struct IRData
{
    size_t max;
    size_t min;
    TempFileData tempFile;
};

struct ShuftSettings
{
    size_t buffer_size;
    size_t start_line;
    size_t end_line;
    size_t header;
    size_t output_limit;
    bool verbose;
    ShuftSources src;
    void* src_data;
    SuftDestinations dst;
    void* dst_data;
    vector< Block > metadata;

    ShuftSettings(): src_data(NULL), dst_data(NULL)
    {
    }

    ~ShuftSettings()
    {
        if (dst_data)
        {
          if (dst == DEST_FILE) delete (FileData*)dst_data;
          else delete (io_buf*)dst_data;
        }

        if (src_data)
        {
            if (src == SOURCE_FILE)
                delete (FileData*)src_data;
            else if (src == SOURCE_STDIN)
                delete (TempFileData*)src_data;
            else  delete (IRData*)src_data;

        }
    }
};


extern ShuftSettings settings;
#endif // SETTINGS_H

