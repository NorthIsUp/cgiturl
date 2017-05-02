#include "math_functions.h"
#include "coding_functions.h"
#include "util.h"

#include <vector>
#include <map>
#include <string>
#include <iostream>

int main( int argc, char * argv[]) {
    create_codes_map();
    create_rcodes_map();
    create_sites_maps();
    create_helper_maps();

    std::string site="2";
    std::string repo="psprint/zkl";
    std::string rev="development";
    std::string file="Src/params.c";
    std::vector<int> selectors;

    std::wstring gcode = build_gcode( site, repo, rev, file, selectors );
    std::wcout << gcode << std::endl;

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
    return 0;
}
