#include "coding_functions.h"
#include <string>
#include <iostream>

bool errorOnDisallowedChars( const std::string & type, const std::string & invalidChars )
{
    if( !invalidChars.empty() ) {
        std::string name = getNames()[ type ];
        if( name.size() > 0 ) {
            std::cout << "Invalid characters in " << name << ": " << invalidChars << ", they are skipped" << std::endl;
        } else {
            std::cout << "Invalid characters used: " << invalidChars << ", they are skipped" << std::endl;
        }
        return true;
    }
    return false;
}
