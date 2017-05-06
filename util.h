#ifndef UTIL_H
#define UTIL_H 1

#include <functional> 
#include <algorithm> 
#include <cctype>
#include <locale>

// Left trim given string
static inline void ltrim( std::string &s ) {
    s.erase( s.begin(), std::find_if( s.begin(), s.end(), std::not1( std::ptr_fun<int, int>( std::isspace ) ) ) );
}

// Right trim given string
static inline void rtrim( std::string &s ) {
    s.erase( std::find_if( s.rbegin(), s.rend(), std::not1( std::ptr_fun<int, int>( std::isspace ) ) ).base(), s.end() );
}

// Left and right trim given string
static inline void trim( std::string &s ) {
    ltrim(s);
    rtrim(s);
}

// Return left-trimmed copy
static inline std::string ltrimmed( std::string s ) {
    ltrim(s);
    return s;
}

// Return right-trimmed copy
static inline std::string rtrimmed( std::string s ) {
    rtrim(s);
    return s;
}

// Return left- and right-trimmed copy
static inline std::string trimmed( std::string s ) {
    trim(s);
    return s;
}

bool errorOnDisallowedChars( const std::string & type, const std::string & invalidChars );

void PresentData(
        std::string & protocol,
        std::string & user,
        std::string & site,
        std::string & port,
        std::string & repo,
        std::string & rev,
        std::string & file
);

const char* single_to_narrow( wchar_t inchar );

const char* wide_to_narrow( wchar_t *input_buffer, int size );
#endif
