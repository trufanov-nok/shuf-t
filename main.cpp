#include "shuf-t.h"
#include "utils.h"
#include <QDebug>
#include <QCoreApplication>
#include <QTime>
#include "qforkedtextstream.h"

QString source_filename;
QString destination_filename;
bool use_input_range;
uint input_range_min = 0;
uint input_range_max = 0;


inline bool getRangeArgument(QString s, uint& i1, uint& i2)
{
    QStringList sl = s.split('-', QString::SkipEmptyParts);
    bool ok = sl.count() == 2 && !sl[1].isEmpty();
    if(ok)
    {
        i1 = sl[0].toUInt(&ok);
        if(ok)
            i2 = sl[1].toUInt(&ok);
    }
    return ok;
}

uint processCommandLineArguments(QCommandLineParser& parser)
{
    //get shuffle parameters
    if(parser.isSet("b"))
        _param_buffer_size = parser.value("b").toFloat()*1024*1024;

    if(parser.isSet("q"))
        _param_verbose = !parser.value("q").toInt();

    if(parser.isSet("t"))
        _param_header  = parser.value("t").toUInt();

    if(parser.isSet("n"))
        _param_output_limit  = parser.value("n").toUInt();

    if (parser.isSet("l") && !getRangeArgument(parser.value("l"), _param_start_line, _param_end_line))
        {
        qCritical() << "lines argument is incorrect. Please use --help for format details.";
        return -1;
        }

    swapIfNeeded(_param_start_line, _param_end_line);

    if(parser.isSet("o"))
        destination_filename  = parser.value("o");

    if(parser.isSet("s"))
    {
        uint seed  = parser.value("s").toUInt();
        srand(seed);
    } else
        srand(time(NULL));



    use_input_range = false;

    if ( (use_input_range = parser.isSet("i")) && !getRangeArgument(parser.value("i"), input_range_min, input_range_max))
        {
            qCritical() << "--input_range argument is incorrect. Please use --help for format details.";
            return -1;
        }

    swapIfNeeded(input_range_min, input_range_max);

    QStringList sll = parser.positionalArguments();
    if (parser.positionalArguments().count() > 0)
      source_filename = parser.positionalArguments()[0];
    if (source_filename.isEmpty() && !use_input_range)
    {
        qCritical() << "ERROR: input filename is missing. Please check command line format with shuf-t --help";
        return -1;
    }


    if (use_input_range && !source_filename.isEmpty())
    {
        qWarning() << "WARNING: Both output file and -i parameter are specified. Output file is ignored.";
    }

return 0;

}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Shuf-t");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    initCommandOptionsParser(parser);
    parser.process(app);

    uint result = processCommandLineArguments(parser);
    if (result != 0)
        return result;


    QTime performance_timer;
    performance_timer.start();

    QForkedTextStream in;

    print("searching line offsets:  ");
    if (use_input_range)
    {
        result = openInputRangeSource(in, input_range_min, input_range_max);
    } else {
        result = openFileSource(in, source_filename);
    }

    print("offsets found: "+ QString::number(metadata.count())+'\n');
    printTime(performance_timer.elapsed());
    performance_timer.restart();

    QTextStream out;

    if (!destination_filename.isEmpty())
    {
        result = openFileDestination(out, destination_filename);
    } else {
        result = openStdOutDestination(out);
    }

    print("shuffling line offsets:  ");

    shuffleMetadata();

    printTime(performance_timer.elapsed());
    performance_timer.restart();

    print("writing lines to output: ");

    writeData(in, out);

    printTime(performance_timer.elapsed());

    closeFileDestination();

    if (use_input_range)
    {
        closeInputRangeSource();
    } else {
        closeFileSource();
    }

    return 0;
}
