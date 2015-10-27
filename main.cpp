/*
Copyright (c) by Alexander Trufanov. Released under a Simplified BSD
license as described in the file LICENSE.
 */
#include "shuf-t.h"
#include "utils.h"

ShuftSettings settings;

using namespace std;

#define time_elapsed ((double)( clock() - performance_timer )) /  CLOCKS_PER_SEC

int processCommandLineArguments(po::variables_map& vm)
{
    //get shuffle parameters

    settings.verbose = !vm.count("quiet");

    if(vm.count("buffer"))  settings.buffer_size = vm["buffer"].as<float>()*1024*1024;
    if(vm.count("top"))     settings.header  = vm["top"].as<int>();
    if(vm.count("head-count"))  settings.output_limit  = vm["head-count"].as<int>();
    if (vm.count("lines"))
    {
        if(!getRangeArgument(vm["lines"].as<string>(), settings.start_line, settings.end_line))
        {
            fprintf(stderr, "invalid lines range ‘%s'", vm["lines"].as<string>().data());
            return -1;
        }
        swapIfNeeded(settings.start_line, settings.end_line);
    }

    size_t seed  = 0;
    if(vm.count("seed"))
    {
        seed  = vm["seed"].as<int>();
    } else
        seed = time(NULL);

    srand(seed);
    print("random seed: ");
    stringstream ss;
    ss << seed << '\n';
    print(ss.str());


    bool ir_src = false;
    if (vm.count("input_range"))
    {
        settings.src = SOURCE_INPUT_RANGE;
        IRData* ir = new IRData();
        settings.src_data = ir;

        if (!getRangeArgument(vm["input_range"].as<string>(), ir->min, ir->max))
        {
            fprintf(stderr, "invalid input range ‘%s'", vm["input_range"].as<string>().data());
            return -1;
        }

        swapIfNeeded(ir->min, ir->max);
        ir_src = true;
    }


    if (vm.count("input-file"))
    {
        if (!ir_src)
        {
            settings.src = SOURCE_FILE;
            FileData* fd = new FileData();
            settings.src_data = fd;
            fd->setFilename( vm["input-file"].as< vector<string> >()[0] );
        } else fprintf(stderr, "WARNING: Both input file and --input_range parameter are specified. Input file is ignored.");
    } else if (!ir_src)
    {
        //stdin
        settings.src = SOURCE_STDIN;
        settings.src_data = new TempFileData();
    }

    if(vm.count("output"))
    {
        settings.dst = DEST_FILE;
        FileData* fd = new FileData();
        settings.dst_data = fd;
        fd->setFilename( vm["output"].as<string>(), io_buf::WRITE);
    } else {
        settings.dst = DEST_STDOUT;
        io_buf* buf =  new io_buf();
        settings.dst_data = buf;
        buf->open_file(NULL, io_buf::WRITE);
    }

    return 0;

}

int main(int argc, char *argv[])
{
    po::variables_map vm;

    if (!initCommandLineOptions(vm, argc, argv)) return 0;

    int result = processCommandLineArguments(vm);
    if (result != 0)
        return result;

    clock_t performance_timer;


    if (settings.src == SOURCE_STDIN)
    {
        performance_timer = clock();
        print("reading data from stdin...\n");
        std::FILE* tempFile = readStdinToTmpFile();
        ((TempFileData*)settings.src_data)->setPtr(tempFile);
        printTime(time_elapsed);
        if (!tempFile) return -2;
    }

    performance_timer = clock();


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
    performance_timer = clock();


    print("shuffling line offsets:  ");

    shuffleMetadata();

    printTime(time_elapsed);
    performance_timer = clock();

    print("writing lines to output: ");


    writeData(*in, *out);

    if (settings.dst == DEST_FILE)
      ((FileData*)settings.dst_data)->file_stream.close_file();

    if (settings.src == SOURCE_FILE)
        ((FileData*)settings.src_data)->file_stream.close_file();
    else if (settings.src == SOURCE_STDIN)
        ((TempFileData*)settings.src_data)->file_stream.close_file();
    else  ((IRData*)settings.src_data)->tempFile.file_stream.close_file();

    printTime(time_elapsed);


    return 0;
}
