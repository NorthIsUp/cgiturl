#include "math_functions.h"
#include "coding_functions.h"
#include "util.h"
#include "optionparser.h"

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <locale>

struct Arg: public option::Arg
{
    static void printError(const char* msg1, const option::Option& opt, const char* msg2)
    {
        fprintf(stderr, "ERROR: %s", msg1);
        fwrite(opt.name, opt.namelen, 1, stderr);
        fprintf(stderr, "%s", msg2);
    }

    static option::ArgStatus Unknown(const option::Option& option, bool msg)
    {
        if (msg) printError("Unknown option '", option, "'\n");
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Required(const option::Option& option, bool msg)
    {
        if (option.arg != 0)
            return option::ARG_OK;

        if (msg) printError("Option '", option, "' requires an argument\n");
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus NonEmpty(const option::Option& option, bool msg)
    {
        if (option.arg != 0 && option.arg[0] != 0)
            return option::ARG_OK;

        if (msg) printError("Option '", option, "' requires a non-empty argument\n");
        return option::ARG_ILLEGAL;
    }
};
 

enum  optionIndex { UNKNOWN, HELP, REV, PATH, DECODE };
const option::Descriptor usage[] =
{
 { UNKNOWN, 0, "", "", option::Arg::None, "USAGE: cgiturl <URL> [options]\n"
                                          "       cgiturl -d <GitURL|gcode>\n\n"
                                          "Options:" },
 { HELP,    0, "h", "help",     option::Arg::None, " -h, --help              Print help and exit" },
 { REV,     0, "r", "revision", Arg::NonEmpty,     " -r <r>, --revision <r>  Which revision to encode in GitURL" },
 { PATH,    0, "p", "path",     Arg::NonEmpty,     " -p <p>, --path <p>      A path to encode in GitURL" },
 { DECODE,  0, "d", "decode",   Arg::NonEmpty,     " -d <c>, --decode <c>    GitURL or gcode to decode" },

 { UNKNOWN, 0, "", "",          option::Arg::None, "\nExamples:\n"
                               "  giturl https://github.com/zdharma/giturl -p lib/codes_huffman\n"
                               "  giturl -d 3ếЭãѿŒΓȏĤÒṄÅṥҫȜẙПȟϛṄӗŀєþ\n"
                               "  giturl -d gitu://VЯĦṟěň4ӊǮ4ХƵȽȤẢ4ФḸțḊƵȽȟϛṄӗŀєþ\n" },
 {0,0,0,0,0,0}
};

int main( int argc, char * argv[]) {
    create_codes_map();
    create_rcodes_map();
    create_sites_maps();
    create_helper_maps();

    argc -= ( argc>0 ); argv += ( argc>0 ); // skip program name argv[0] if present
    option::Stats  stats( usage, argc, argv );
    std::vector<option::Option> options( stats.options_max );
    std::vector<option::Option> buffer( stats.buffer_max );
    option::Parser parse( true, usage, argc, argv, &options[0], &buffer[0] );

    if ( parse.error() ) {
        fprintf( stderr, "Error when parsing options, aborting\n" );
        fflush( stderr );
        return 1;
    }

    if ( options[HELP] ) {
        option::printUsage(std::cout, usage);
        return 0;
    }

    if ( options[DECODE].count() == 0 ) {
        std::string site="2";
        std::string repo="psprint/zkl";
        std::string rev;
        if ( options[REV].count() > 0 ) {
            rev = std::string( options[REV].last()->arg );
        }
        std::string file;
        if ( options[PATH].count() > 0 ) {
            file = std::string( options[PATH].last()->arg );
        }
        std::vector<int> selectors;

        std::wstring gcode = build_gcode( site, repo, rev, file, selectors );
        std::wcout << gcode << std::endl;
    } else {
        std::wstring gcode( strlen( options[DECODE].last()->arg ), L' ');
        gcode.resize( std::mbstowcs( &gcode[0], options[DECODE].last()->arg, strlen( options[DECODE].last()->arg ) ) );

        std::vector<int> bits;
        int error;
        tie( bits, error ) = decode_zcode( gcode );
        if( error != 0 ) {
            std::cout << "Error during decoding the entered Zcode: " << error << std::endl;
        } else {
            int to_skip;
            std::map< std::string, std::string > decoded;
            tie( to_skip, decoded, error ) = process_meta_data( bits );

            std::cout << "Site: " << decoded["site"] << std::endl;
            std::cout << "Repo: " << decoded["repo"] << std::endl;
            std::cout << "Rev: " << decoded["rev"] << std::endl;
            std::cout << "File: " << decoded["file"] << std::endl;
        }
    }
    return 0;
}
