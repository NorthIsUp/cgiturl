/*
 Copyright 2016 Sebastian Gniazdowski

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "math_functions.h"
#include <QDebug>
#include <algorithm>
#include "singleton.h"
#include "messages.h"

#define MessagesI Singleton<Messages>::instance()

const char characters[37]={ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                      'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '\0' };

const char *characters_begin = &characters[0];
const char *characters_end = &characters[36];

// FUNCTION: convert_integer_to_base_36 {{{
// Takes number, returns string [a-z0-9]+ that
// is representation of the number in base 36

std::tuple< std::vector<char>, int > convert_integer_to_base_36(int number) {
    std::vector<int> digits;

    int new_number = number;
    int remainder;
    while ( new_number != 0 ) {
        remainder = new_number % 36;
        new_number = new_number/36;

        digits.push_back( remainder );
    }

    if ( digits.size() == 0 )
        digits.push_back(0);
    if ( digits.size() == 1 )
        digits.push_back(0);
    if ( digits.size() == 2 )
        digits.push_back(0);

    std::reverse(digits.begin(), digits.end());

    std::tuple<std::vector<char>, int> letters_error = numbers_to_letters(digits);

    return letters_error;
}
// }}}

// FUNCTION: numbers_to_letters {{{
// Converts array of numbers into string [a-z0-9]+
std::tuple< std::vector<char>, int> numbers_to_letters(const std::vector<int> & digits) {
    std::vector<char> REPLY;
    for( std::vector<int>::const_iterator it = digits.begin(); it != digits.end(); it ++ ) {
        int i = *it;
        if ( i < 0 || i > 35 ) {
            MessagesI.AppendMessageT( "Incorrect number during character conversion: " + QString("%1").arg(i) );
            return std::tuple<std::vector<char>, int>( REPLY, 1 );
        }
        REPLY.push_back( characters[i] );
    }

    return std::tuple<std::vector<char>, int>( REPLY, 0 );
}

// FUNCTION: letters_to_numbers {{{
// Converts each character of input string into number 0..35
std::tuple< std::vector<int>, int> letters_to_numbers( const std::vector<char> & letters ) {
    std::vector<int> reply;

    QString str = QString::fromUtf8( &letters[0], letters.size() );
    QRegExp rx("^[a-z0-9]+$");
    rx.setCaseSensitivity( Qt::CaseSensitive );
    if ( rx.indexIn( str ) == -1 ) {
        rx = QRegExp("[^a-z0-9]");
        QString message( "Incorrect character in Zcode" );
        if ( rx.indexIn( str ) != -1 ) {
            // Append the wrong character
            message += QString( " (<b>%1</b>)" ).arg( rx.cap(0) );
        }
        MessagesI.AppendMessageT( message + ", allowed are a-z and 0-9" );
        return std::make_tuple( reply, 100 );
    }

    int number;
    for( std::vector<char>::const_iterator it = letters.begin(); it != letters.end(); it ++ ) {
        const char *number_it = std::find( characters_begin, characters_end, *it );
        number = std::distance( characters_begin, number_it );
        reply.push_back( number );
    }

    return std::make_tuple( reply, 0 );
}

std::tuple< std::vector<int>, int> letters_to_numbers( const std::string & letters ) {
    std::vector<char> letters2( letters.c_str(), letters.c_str() + letters.size() );
    return letters_to_numbers( letters2 );
}

// FUNCTION: decode_zcode {{{
// Takes zekyl code, i.e. 1/someletters
// and decodes it to sequence of bits
std::tuple< std::vector<int>, int > decode_zcode( const std::string & code ) {
    QString _code = QString::fromStdString( code );
    QStringList num_let = _code.split("/");
    if( num_let.count() != 2 ) {
        return std::make_tuple( std::vector<int>(), 101 );
    }

    bool ok = false;
    int number = num_let.first().toInt( &ok );
    if( !ok ) {
        return std::make_tuple( std::vector<int>(), 102 );
    }

    std::string letters = num_let.last().toUtf8().constData();

    // The zcode can have at most 30 digits
    // This is the 150 bits (150 zekylls)
    // written in base 36. We have to obtain
    // the 150 bits. We will implement division
    // in base 36 and gradually obtain the 150 bits.

    std::vector<int> bits;
    std::string workingvar = letters;
    QRegExp rx("^0*$");
    while( rx.indexIn( QString::fromStdString( workingvar ) ) == -1 ) {
        int subtracted, error;
        std::tie( workingvar, subtracted, error ) = div2( workingvar );
        if( error != 0 ) {
            return std::make_tuple( std::vector<int>(), 103 );
        }
        bits.push_back( subtracted );
        // print "After div $workingvar/${reply[2]}"
    }
    std::reverse( bits.begin(), bits.end() );
    // print "Bits of the letters $letters are: ${(j::)bits[@]}"

    return std::make_tuple( bits, 0 );
}
// }}}

// FUNCTION: get_integer_from_base_36 {{{
// Converts given base-36 string into integer
// Warning: it tagets integer (signed), so
// the size of number is limited here (while
// decode_zcode generates series of bits of
// arbitrary length)
std::tuple<int, int> get_integer_from_base_36( const std::string & letters ) {
    std::vector<int> bits;
    std::string workingvar = letters;

    // Get base-2 generated array consisting of 1 and 0
    QRegExp rx("^0*$");
    while( rx.indexIn( QString::fromLatin1( workingvar.c_str() ) ) == -1 ) {
        int subtracted, error;
        std::tie( workingvar, subtracted, error ) = div2( workingvar );
        if( error != 0 ) {
            return std::make_tuple( 0, error + 1000 );
        }
        bits.push_back( subtracted );
    }

    std::reverse( bits.begin(), bits.end() );

    // Now sum up the obtained 0 and 1
    int i, mul = 1, size = bits.size();
    int REPLY=0;
    for ( i = size - 1; i >= 0; i -- ) {
        REPLY = REPLY + bits[i]*mul;
        mul = mul * 2;
    }

    // TODO: detect overflow and other problems
    return std::make_tuple( REPLY, 0 );
}
// }}}

// FUNCTION: encode_zcode_str01 {{{
// Takes string of 0 and 1 that mark which zekylls are active
// and encodes it to base 36 number expressed via a-z0-9
std::tuple< std::vector<char>, std::vector<int>, int > encode_zcode_str01( const std::string & sbits ) {
    std::vector<int> numbers;
    int error;
    std::tie( numbers, error ) = str_01_to_24_bit_pack_numbers( sbits );
    if( error != 0 ) {
        return make_tuple( std::vector<char>(), std::vector<int>(), error + 2000 );
    }

    std::vector<char> code;
    std::tie( code, numbers, error ) = encode_zcode_24_bit_pack_numbers( numbers );
    if( error != 0 ) {
        return make_tuple( code, numbers, error + 3000 );
    }

    return make_tuple( code, numbers, 0 );
}
// }}}

// FUNCTION: encode_zcode_arr01 {{{
// Takes array of 0 and 1 that mark which zekylls are
// active and encodes it to base 36 number expressed
// via a-z0-9
std::tuple< std::vector<char>, std::vector<int>, int > encode_zcode_arr01( const std::vector<int> & bits ) {
    std::vector<int> numbers;
    int error;
    std::tie( numbers, error ) = arr_01_to_24_bit_pack_numbers( bits );
    if( error != 0 ) {
        return make_tuple( std::vector<char>(), std::vector<int>(), error + 8000 );
    }
    std::vector<char> code;
    std::tie( code, numbers, error ) = encode_zcode_24_bit_pack_numbers( numbers );
    if( error != 0 ) {
        return make_tuple( code, numbers, error + 9000 );
    }
    return make_tuple( code, numbers, 0 );
}
// }}}

// FUNCTION: encode_zcode_24-bit_pack_numbers {{{
// Takes 24-bit pack numbers whose bits mark which zekylls are active
// and encodes them to base 36 number expressed via a-z0-9
std::tuple< std::vector<char>, std::vector<int>, int > encode_zcode_24_bit_pack_numbers( const std::vector<int> & numbers ) {
    std::vector<int> nums_base36, workingvar;
    workingvar = numbers;

    bool all_zero = true;
    for( unsigned int i = 0; i < workingvar.size(); i ++ ) {
        if( workingvar[i] != 0 ) {
            all_zero = false;
            break;
        }
    }

    while ( !all_zero ) {
        int error, subtracted;
        std::tie( workingvar, subtracted, error ) = div_24_bit_pack_numbers_36( workingvar );
        nums_base36.push_back( subtracted );

        all_zero = true;
        for( unsigned int i = 0; i < workingvar.size(); i ++ ) {
            if( workingvar[i] != 0 ) {
                all_zero = false;
                break;
            }
        }
    }

    std::reverse( nums_base36.begin(), nums_base36.end() );

    std::vector<char> letters;
    int error;
    tie( letters, error ) = numbers_to_letters( nums_base36 );

    if( error != 0 ) {
        error += 4000;
    }

    return make_tuple( letters, nums_base36, error );
}
// }}}

// FUNCTION: div2 {{{
// input - zcode's letters
// Result – ( "zcode's letters after division" "remainder 0 or 1" )
std::tuple< std::vector<char>, int, int > div2( const std::vector<char> & letters ) {
    // First translate the letters to numbers and put them into array
    std::tuple< std::vector<int>, int > res = letters_to_numbers( letters );
    int error = std::get<1>( res );
    if( error != 0 ) {
        return std::make_tuple( std::vector<char>(), 0, error + 5000 );
    }

    std::vector<int> numbers = std::get<0>( res );

    // Now operate on the array performing long-division
    int cur = 0, last = numbers.size() - 1;

    std::vector<int> result;

    int prepared_for_division = numbers[cur];
    int subtracted;
    while (( 1 )) {
        int quotient = prepared_for_division / 2;

        result.push_back( quotient );

        int recovered = quotient*2;
        subtracted = prepared_for_division - recovered;

        cur ++;
        if ( cur > last ) {
            break;
        }

        prepared_for_division = 36 * subtracted + numbers[cur];
    }

    // Now convert the result to letters

    std::tuple< std::vector<char>, int > res2 = numbers_to_letters( result );
    error = std::get<1>(res2);
    if( error != 0 ) {
        return std::make_tuple( std::vector<char>(), 0, error + 6000 );
    }

    return std::make_tuple( std::get<0>( res2 ), subtracted, 0 );
}
// }}}

// FUNCTION: div2 {{{
std::tuple< std::string, int, int > div2( const std::string & letters ) {
    std::vector<char> out_letters;
    int subtracted, error;
    tie( out_letters, subtracted, error ) = div2( std::vector<char>( letters.c_str(), letters.c_str() + letters.size() ) );
    if( error != 0 ) {
        error += 7000;
    }
    return std::make_tuple( std::string( out_letters.begin(), out_letters.end() ), subtracted, error );
}
// }}}

// FUNCTION: str_01_to_24-bit_pack_numbers {{{
// Takes STRING of 0 and 1 and converts it to array of numbers
// that are 24-bit packs taken from right to left, from the string
std::tuple< std::vector<int>, int > str_01_to_24_bit_pack_numbers( const std::string & sbits ) {
    QString sbits2 = QString::fromLatin1( sbits.c_str() );
    QStringList lbits = sbits2.split( "", QString::SkipEmptyParts );

    std::vector<int> bits;
    foreach ( const QString &str, lbits ) {
        bool ok = false;
        int bit = str.toInt( &ok );
        if( !ok ) {
            return make_tuple( std::vector<int>(), 104 );
        }

        if( bit != 0 && bit != 1 ) {
            return make_tuple( std::vector<int>(), 105 );
        }

        bits.push_back( bit );
    }

    return arr_01_to_24_bit_pack_numbers( bits );
}
// }}}

// FUNCTION: arr_01_to_24-bit_pack_numbers {{{
// Takes ARRAY (@) of 0 and 1 and converts it to array of numbers
// that are 24-bit packs taken from right to left, from the string
std::tuple< std::vector<int>, int > arr_01_to_24_bit_pack_numbers( const std::vector<int> & bits ) {
    std::vector<int> pack, numbers;
    int count=0, i, size = bits.size();

    // Take packs of 24 bits, convert each to number and store in array
    for ( i = size - 1; i >= 0; i-- ) {
        if( bits[i] != 0 && bits[i] != 1 ) {
            return std::make_tuple( std::vector<int>(), 106 );
        }

        pack.push_back( bits[i] );
        count+=1;
        if ( count < 24 && i != 0 ) {
                continue;
        }

        count = 0;
        std::reverse( pack.begin(), pack.end() );

        // Convert the max. 24 bit pack to number
        int result = 0;
        for( unsigned int p = 0; p < pack.size(); p ++ ) {
            result = result*2 + pack[p];
        }

        numbers.push_back( result );

        pack.clear();
    }

    std::reverse( numbers.begin(), numbers.end() );

    return std::make_tuple( numbers, 0 );
}
// }}}

// FUNCTION: div_24-bit_pack_numbers_36 {{{
// input - series of 0 and 1 (bits marking which zekyll is active)
// output - result of division and remainder 0 ... 35, also error
std::tuple< std::vector<int>, int, int > div_24_bit_pack_numbers_36( const std::vector<int> & numbers ) {
    // Now operate on the array performing long-division
    int cur = 0, last = numbers.size() - 1;

    std::vector<int> result;

    int subtracted;
    int prepared_for_division = numbers[cur];
    while ( 1 ) {
        int quotient = prepared_for_division / 36;

        result.push_back( quotient );

        int recovered = quotient * 36;
        subtracted = prepared_for_division - recovered;

        cur ++;
        if ( cur > last ) {
            break;
        }

        prepared_for_division = 16777216 * subtracted + numbers[cur];

        if( numbers[cur] > prepared_for_division ) {
            return make_tuple( std::vector<int>(), 0, 107);
        }
    }

    return std::make_tuple( result, subtracted, 0 );
}
// }}}
