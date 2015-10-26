#include <ctime>
#include <string>
#include <cstdio>

#include "shuf-t.h"
#include "utils.h"

#include <unistd.h> // for isatty()

string source_filename;
string destination_filename;
bool use_input_range;
size_t input_range_min = 0;
size_t input_range_max = 0;

using namespace std;

inline bool getRangeArgument(string s, size_t& i1, size_t& i2)
{
    vector<string> sl = split(s, '-');
    if (!(sl.size() != 2) && (sl[1].size() <= 0)) return false;

    try
    {
        i1 = atol( sl[0].data() );
    }
    catch(...) { return false; }

    try
    {
        i2 = atol( sl[1].data() );
    }
    catch(...) { return false; }

    return true;
}

size_t processCommandLineArguments(po::variables_map& vm)
{
    //get shuffle parameters

    if(vm.count("buffer"))
        _param_buffer_size = vm["buffer"].as<float>()*1024*1024;

    _param_verbose = !vm.count("quiet");

    _is_terminal =  isatty(fileno(stdout));

    if(vm.count("top"))
        _param_header  = vm["top"].as<int>();

    if(vm.count("head-count"))
        _param_output_limit  = vm["head-count"].as<int>();

    if (vm.count("lines") && !getRangeArgument(vm["lines"].as<string>(), _param_start_line, _param_end_line))
    {
        fprintf(stderr, "lines argument is incorrect. Please use --help for format details.");
        return -1;
    }

    swapIfNeeded(_param_start_line, _param_end_line);

    if(vm.count("output"))
        destination_filename = vm["output"].as<string>();

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



    use_input_range = false;

    if ( (use_input_range = vm.count("input_range")) && !getRangeArgument(vm["input_range"].as<string>(), input_range_min, input_range_max))
    {
        fprintf(stderr, "--input_range argument is incorrect. Please use --help for format details.");
        return -1;
    }

    swapIfNeeded(input_range_min, input_range_max);

    if (vm.count("input-file"))
        source_filename = vm["input-file"].as< vector<string> >()[0];

    if (use_input_range && !source_filename.empty())
    {
        fprintf(stderr, "WARNING: Both output file and -i parameter are specified. Output file is ignored.");
    }

    return 0;

}

int main(int argc, char *argv[])
{
    po::variables_map vm;
    if (!initCommandLineOptions(vm, argc, argv)) return 0;

    size_t result = processCommandLineArguments(vm);
    if (result != 0)
        return result;


    if (!use_input_range && source_filename.empty())
    {
        print("reading data from stdin...\n");
//        source_filename = readStdinToTmpFile(); //TODO
    }

    clock_t performance_timer;
    performance_timer = clock();

    io_buf in;

    print("searching line offsets:  ");
    if (use_input_range)
    {
        result = openInputRangeSource(in, input_range_min, input_range_max);
    } else {
        if( !source_filename.empty() )
            result = openFileSource(in, source_filename);
    }

    stringstream ss;
    ss << "offsets found: " << metadata.size() << '\n';
    print(ss.str());    
    printTime(( clock() - performance_timer ) / (double) CLOCKS_PER_SEC *1000.);
    performance_timer = clock();

    io_buf out;

    if (!destination_filename.empty())
    {
        result = openFileDestination(out, destination_filename);
    } else {
        result = openStdOutDestination(out);
    }

    //    in.setCodec("CP1251");
    //    in.autoDetectUnicode(false);
    //    out.setCodec("CP1251");
    //    out.autoDetectUnicode(false);

    print("shuffling line offsets:  ");

    shuffleMetadata();

    printTime(( clock() - performance_timer ) / (double) CLOCKS_PER_SEC*1000.);
    performance_timer = clock();

    print("writing lines to output: ");

    writeData(in, out);

    printTime(( clock() - performance_timer ) / (double) CLOCKS_PER_SEC*1000.);

    closeFileDestination();

    if (use_input_range)
    {
        closeInputRangeSource();
    } else {
        closeFileSource();
    }

    return 0;
}
