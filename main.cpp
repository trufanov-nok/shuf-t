/*
Copyright (c) by Alexander Trufanov. Released under a Simplified BSD
license as described in the file LICENSE.
 */
#include "shuf-t.h"
#include "utils.h"
#include <time.h>

const char* SHUF_T_VERSION = "1.2.2";
ShuftSettings settings;

using namespace std;

#define time_elapsed difftime( time(NULL), performance_timer )

int processCommandLineArguments(CSimpleOpt* opt)
{
    //set default parameters
    settings.verbose = true;
    settings.output_limit = 0;
    settings.buffer_size = 1024*1024*1024;
    settings.header = 0;
    unsigned int seed = (unsigned int)time(NULL);
    bool ir_src = false;
    bool out_file = false;
    //get shuffle parameters

    while (opt->Next()) {
        if (opt->LastError() == SO_SUCCESS) {
            switch (opt->OptionId())
            {
            case OPT_HELP:
                showHelp();
                return 1;
            case OPT_VER:
                std::cout << "Program version: " << SHUF_T_VERSION << std::endl;
                return 1;
            case OPT_QUIET:
                settings.verbose = false;
                break;
            case OPT_BUF:
                settings.buffer_size = (size_t)atof(opt->OptionArg())*1024*1024;
                break;
            case OPT_TOP:
                settings.header = atoi(opt->OptionArg());
                break;
            case OPT_HEAD:
                settings.output_limit = atoi(opt->OptionArg());
                break;
            case OPT_LINES:
            {
                string s(opt->OptionArg());
                if(!getRangeArgument(s, settings.start_line, settings.end_line))
                {
                    fprintf(stderr, "invalid lines range ‘%s'", opt->OptionArg());
                    return -1;
                }
                swapIfNeeded(settings.start_line, settings.end_line);
            }
                break;
            case OPT_SEED:
                seed = atoi(opt->OptionArg());
                break;
            case OPT_RANGE:
            {
                settings.src = SOURCE_INPUT_RANGE;
                IRData* ir = new IRData();
                settings.src_data = ir;
                string s(opt->OptionArg());
                if (!getRangeArgument(s, ir->min, ir->max))
                {
                    fprintf(stderr, "invalid input range ‘%s'", opt->OptionArg());
                    return -1;
                }

                swapIfNeeded(ir->min, ir->max);
                ir_src = true;
            }
                break;
            case OPT_OUT:
            {
                settings.dst = DEST_FILE;
                FileData* fd = new FileData();
                settings.dst_data = fd;
                fd->setFilename( string(opt->OptionArg()), io_buf::WRITE);
                out_file = true;
            }
                break;
            default:
                printf("Invalid argument: %s\n", opt->OptionText());
                return 1;
            }
        } else {
            printf("Invalid argument: %s\n", opt->OptionText());
            return 1;
        }
    }

    srand(seed);
    print("random seed: ");
    stringstream ss;
    ss << seed << '\n';
    print(ss.str());


    if (opt->FileCount() > 0)
    {
        if (!ir_src)
        {
            settings.src = SOURCE_FILE;
            FileData* fd = new FileData();
            settings.src_data = fd;
            fd->setFilename( string(opt->File(0)) );
        } else fprintf(stderr, "WARNING: Both input file and --input_range parameter are specified. Input file is ignored.");
    } else
    if (!ir_src )
    {
        //stdin
        settings.src = SOURCE_STDIN;
        settings.src_data = new TempFileData();
    }

    if (!out_file)
    {
        settings.dst = DEST_STDOUT;
        io_buf* buf =  new io_buf();
        settings.dst_data = buf;
        buf->open_file(NULL, io_buf::WRITE);
    }

    return 0;

}

int main(int argc, char *argv[])
{
    CSimpleOpt* opt_parser = initCommandLineOptions(argc, argv);
    if (!opt_parser) return 0;


    int result = processCommandLineArguments(opt_parser);
    delete opt_parser;

    if (result != 0)
        return result;

    time_t performance_timer;


    if (settings.src == SOURCE_STDIN)
    {
        performance_timer = time(NULL);;
        print("reading data from stdin...\n");
        std::FILE* tempFile = readStdinToTmpFile();
        ((TempFileData*)settings.src_data)->setPtr(tempFile);
        printTime(time_elapsed);
        if (!tempFile) return -2;
    }

    performance_timer = time(NULL);


    print("searching line offsets:  ");

    io_buf* in;

    switch (settings.src)
    {
    case SOURCE_INPUT_RANGE:
    {
        IRData* ir_data = (IRData*)settings.src_data;
        std::FILE* tempFile = openTmpFile();
        if (!tempFile) return -2;
        ir_data->tempFile.setPtr( tempFile );

        in = &ir_data->tempFile.file_stream;
        storeInputRangeToFile(ir_data->tempFile.ptr(), ir_data->min, ir_data->max);
        in->reset();
        result = readMetadata(*in, in->size());
        break;
    }
    case SOURCE_FILE:
    {
        FileData* fd = (FileData*)settings.src_data;
        in = &fd->file_stream;
        result = readMetadata(*in, in->size());
        break;
    }
    case SOURCE_STDIN:
    {
        TempFileData* tfd = (TempFileData*)settings.src_data;
        in = &tfd->file_stream;
        in->reset();
        result = readMetadata(*in, in->size());
        break;
    }
    default: in = NULL; break;
    }


    io_buf* out;
    if (settings.dst == DEST_FILE)
        out = &((FileData*)settings.dst_data)->file_stream;
    else
        out = (io_buf*) settings.dst_data;

    stringstream ss;
    ss << "offsets found: " << settings.metadata.size() << '\n';
    print(ss.str());
    printTime(time_elapsed);
    performance_timer = time(NULL);;


    print("shuffling line offsets:  ");

    shuffleMetadata();

    printTime(time_elapsed);
    performance_timer = time(NULL);;

    print("writing lines to output: ");


    writeData(*in, *out);

    if (settings.dst == DEST_FILE)
        ((FileData*)settings.dst_data)->file_stream.close_file();

    if (settings.src == SOURCE_FILE)
        ((FileData*)settings.src_data)->file_stream.close_file();

    printTime(time_elapsed);


    return 0;
}
