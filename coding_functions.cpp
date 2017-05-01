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

#include "coding_functions.h"
#include "math_functions.h"
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <QDebug>
#include <QMap>

QMap<QString, QString> codes;
QMap<QString, QString> rcodes;
QMap<QString, QString> sites;
QMap<QString, QString> rsites;
QMap<QString, QString> names;

// FUNCTION: setIndex {{{
// Sets ZKL_INDEX_ZEKYLLS array which contains all
// zekylls that potentially can be part of the index
//
std::tuple< std::vector<std::string>, int> setIndex(int index) {

    std::vector<std::string> ZKL_INDEX_ZEKYLLS;

    // Compute first element pointed to by index
    int first=(index-1)*150;

    int error=0;
    for ( int i=first; i<=(first+150-1); i ++ ) {
        // Convert the number to base 36 with leading zeros
        std::tuple< std::vector<char>, int > result = convert_integer_to_base_36(i);
        error += std::get<1>( result );
        std::string zekyll = std::string( std::get<0>( result ).begin(), std::get<0>( result ).end() );
        ZKL_INDEX_ZEKYLLS.push_back( zekyll );
    }
    // foreach(const string& s, ZKL_INDEX_ZEKYLLS) { qDebug() << QString(s.c_str()); }

    return std::make_tuple( ZKL_INDEX_ZEKYLLS, error );
}
// }}}

// input – bits decoded from zcode
// first reply = bits to skip
// second reply = ( file "" rev "" repo "" wordrev "" chksum "" site "" unused1 "" unused2 "" unused3 "" error "" )
std::tuple< int, QMap<QString,QString>, int > process_meta_data( const std::vector<int> & _bits ) {
    int size = _bits.size();
    std::vector<int> bits = _bits;
    std::reverse( bits.begin(), bits.end() );

    std::stringstream ss;
    std::copy( bits.begin(), bits.end(), std::ostream_iterator<int>(ss,""));
    QString strbits = QString::fromLatin1( ss.str().c_str() );

    QRegExp rx("^[01]+$");
    if( rx.indexIn(strbits) == -1 ) {
        return std::make_tuple( 0, QMap<QString, QString>(), 120 );
    }

    // We will use this value to compute how much bits were skipped
    int init_len = strbits.count();

    int REPLY, error = 0;
    QMap< QString, QString > decoded;
    decoded["file"] = "";
    decoded["rev"] = "";
    decoded["repo"] = "";
    decoded["wordrev"] = "";
    decoded["chksum"] = "";
    decoded["site"] = "";
    decoded["unused1"] = "";
    decoded["unused2"] = "";
    decoded["unused3"] = "";
    decoded["error"] = "";

    // Is there SS?
    QString str = strbits.left( codes["ss"].count() );
    if( str == codes["ss"] ) {
        strbits = strbits.mid( codes["ss"].count() );
        str = strbits.left( codes["ss"].count() );

        // Is there immediate following SS?
        if( str == codes["ss"] ) {
            // We should skip one SS and there is nothing to decode
            return std::make_tuple( codes["ss"].count(), decoded, 0 );
        }

        //
        // Follows meta data, decode it
        //

        // keys of the 'decoded' hash
        QString current_selector="error";
        int trylen;
        QString mat, trystr;
        while ( 1 ) {
            mat="";
            for ( trylen = 6; trylen <= 7; trylen ++ ) {
                // Take substring of len $trylen and check if
                // it matches any Huffman code
                trystr = strbits.left( trylen );
                if( rcodes.contains( trystr )) {
                    mat = rcodes[ trystr ];
                    break;
                }
            }

            if( mat == "" ) {
                return std::make_tuple( 0, decoded, 121 );
            }

            // Skip decoded bits
            strbits = strbits.mid( trylen );

            // Handle what has been matched, either selector or data
            if( mat == "ss" ) {
                break;
            } else if( decoded.contains(mat) ) {
                current_selector = mat;
            } else {
                if( current_selector == "site" ) {
                    mat = rsites[ mat ];
                }
                decoded[ current_selector ].append( mat );
            }
        }

        REPLY = init_len - strbits.count();
    } else {
        REPLY = 0;
    }

    return std::make_tuple( REPLY, decoded, error );
}

int insertBitsFromStrBits( std::vector<int> & dest, const QString & str ) {
    int error = 0;
    QStringList list = str.split( "", QString::SkipEmptyParts );
    bool was = false;
    foreach( const QString & b, list ) {
        was = true;
        if( b == "1" ) {
            dest.push_back( 1 );
        } else if( b == "0" ) {
            dest.push_back( 0 );
        } else {
            error += 157;
            dest.push_back( 1 );
        }
    }

    if( !was ) {
        error += 257;
    }
    return error;
}

int BitsStart( std::vector<int> & dest ) {
    int error = insertBitsFromStrBits( dest, codes["ss"] );
    if( error ) {
        error += 1570000;
    }
    return error;
}

std::tuple< std::vector<int>, int, QStringList > BitsForString( const QString & data ) {
    QStringList invalidChars;
    int error = 0;
    std::vector<int> out;
    QStringList list = data.trimmed().split( "", QString::SkipEmptyParts );
    foreach( const QString & l, list ) {
        QString strbits = codes[l];
        if( strbits.count() == 0 ) {
            invalidChars << l;
            error += 163;
        } else {
            error += insertBitsFromStrBits( out, strbits );
        }
    }

    return std::make_tuple( out, error, invalidChars );
}

std::tuple<int, QStringList> BitsWithPreamble( std::vector<int> & dest, const QString & type, const QString & data ) {
    QStringList invalidChars;
    int error;
    std::vector<int> bits;
    std::tie( bits, error, invalidChars ) = BitsForString( data );
    if( error ) {
        error += 1630000;
    }

    if( bits.size() > 0 ) {
        QString preambleStrBits = codes[ type ].trimmed();
        if( preambleStrBits.count() > 0 ) {
            // Preamble
            int newerror = insertBitsFromStrBits( dest, preambleStrBits );
            if( newerror ) {
                newerror += 1930000;
            }
            error += newerror;

            // Data
            dest.insert( dest.end(), bits.begin(), bits.end() );
        } else {
            error += 2330000;
        }
    }

    return std::make_tuple( error, invalidChars );
}

int BitsStop( std::vector<int> & dest ) {
    int error = insertBitsFromStrBits( dest, codes["ss"] );
    if( error ) {
        error += 2570000;
    }
    return error;
}

// FUNCTION: BitsCompareSuffix {{{2
// Compares suffix of the longer "$1" with whole "$2"
std::tuple< bool, int > BitsCompareSuffix( const std::vector<int> & bits, const QString & strBits ) {
    int error = 0;

    QStringList list = strBits.trimmed().split( "", QString::SkipEmptyParts );
    std::vector<int> bits2;

    if( list.size() == 0 ) {
        error += 193;
        return std::make_tuple( true, error );
    }

    foreach( const QString & b, list ) {
        if( b == "1" ) {
            bits2.push_back( 1 );
        } else if ( b == "0" ) {
            bits2.push_back( 0 );
        } else {
            error += 233;
            bits2.push_back( 1 );
        }
    }

    if( bits.size() < bits2.size() ) {
        return std::make_tuple( 0, error );
    }

    // Check if short_bits occur at the end of long_bits
    int beg_idx = bits.size() - bits2.size();
    int end_idx = bits.size();
    bool not_equal = false;
    int s = 0;
    for ( int l = beg_idx; l < end_idx; l++ ) {
        if( bits[l] != bits2[s] ) {
            not_equal = true;
            break;
        }

        if( (bits[l] != 0 && bits[l] != 1) || (bits2[s] != 0 && bits2[s] != 1) ) {
            error += 277;
        }

        s += 1;
    }

    return std::make_tuple( !not_equal, error );
}

// FUNCTION: BitsRemoveIfStartStop() {{{2
// This function removes any SS bits if meta-data is empty
int BitsRemoveIfStartStop( std::vector<int> & bits ) {
    bool result;
    int error;

    std::tie( result, error ) = BitsCompareSuffix( bits, codes[ "ss" ] );
    error += error ? 2770000 : 0;

    if( result ) {
        bits.erase( bits.end() - codes["ss"].count(), bits.end() );

        int error2;
        std::tie( result, error2 ) = BitsCompareSuffix( bits, codes[ "ss" ] );
        error += error2 ? error2 + 3110000 : 0;

        if( result ) {
            bits.erase( bits.end() - codes["ss"].count(), bits.end() );
            // Two consecutive SS bits occured, correct removal
        } else {
            // We couldn't remove second SS bits, so it means
            // that there is some meta data, and we should
            // restore already removed last SS bits
            int error3 = insertBitsFromStrBits( bits, codes["ss"] );
            error += error3 ? error3 + 3370000 : 0;
        }
    } else {
        // This shouldn't happen, this function must be
        // called after adding SS bits
        error += 3730000;
    }

    return error;
}

void create_codes_map() {
    codes["ss"]      = "101000";
    codes["file"]    = "100111";
    codes["rev"]     = "101110";
    codes["repo"]    = "101111";
    codes["wordrev"] = "101100";
    codes["chksum"]  = "101101";
    codes["site"]    = "100010";
    codes["unused1"] = "100011";
    codes["unused2"] = "100000";
    codes["unused3"] = "1100110";
    codes["b"] = "110001";
    codes["a"] = "110000";
    codes["9"] = "101011";
    codes["8"] = "101010";
    codes["."] = "101001";
    codes["/"] = "100110";
    codes["_"] = "100101";
    codes["-"] = "100100";
    codes["~"] = "100001";
    codes["x"] = "011111";
    codes["w"] = "011110";
    codes["z"] = "011101";
    codes["y"] = "011100";
    codes["t"] = "011011";
    codes["s"] = "011010";
    codes["v"] = "011001";
    codes["u"] = "011000";
    codes["5"] = "010111";
    codes["4"] = "010110";
    codes["7"] = "010101";
    codes["6"] = "010100";
    codes["1"] = "010011";
    codes["0"] = "010010";
    codes["3"] = "010001";
    codes["2"] = "010000";
    codes["h"] = "001111";
    codes["g"] = "001110";
    codes["j"] = "001101";
    codes["i"] = "001100";
    codes["d"] = "001011";
    codes["c"] = "001010";
    codes["f"] = "001001";
    codes["e"] = "001000";
    codes["p"] = "000111";
    codes["o"] = "000110";
    codes["r"] = "000101";
    codes["q"] = "000100";
    codes["l"] = "000011";
    codes["k"] = "000010";
    codes["n"] = "000001";
    codes["m"] = "000000";
    codes["J"] = "1111111";
    codes["I"] = "1111110";
    codes["L"] = "1111101";
    codes["K"] = "1111100";
    codes["F"] = "1111011";
    codes["E"] = "1111010";
    codes["H"] = "1111001";
    codes["G"] = "1111000";
    codes["R"] = "1110111";
    codes["Q"] = "1110110";
    codes["T"] = "1110101";
    codes["S"] = "1110100";
    codes["N"] = "1110011";
    codes["M"] = "1110010";
    codes["P"] = "1110001";
    codes["O"] = "1110000";
    codes["Z"] = "1101111";
    codes["Y"] = "1101110";
    codes[" "] = "1101101";
    codes["A"] = "1101100";
    codes["V"] = "1101011";
    codes["U"] = "1101010";
    codes["X"] = "1101001";
    codes["W"] = "1101000";
    codes["B"] = "1100111";
    codes["D"] = "1100101";
    codes["C"] = "1100100";
}

void create_rcodes_map() {
    rcodes["101000"] = "ss";
    rcodes["100111"] = "file";
    rcodes["101110"] = "rev";
    rcodes["101111"] = "repo";
    rcodes["101100"] = "wordrev";
    rcodes["101101"] = "chksum";
    rcodes["100010"] = "site";
    rcodes["100011"] = "unused1";
    rcodes["100000"] = "unused2";
    rcodes["1100110"] = "unused3";
    rcodes["110001"] = "b";
    rcodes["110000"] = "a";
    rcodes["101011"] = "9";
    rcodes["101010"] = "8";
    rcodes["101001"] = ".";
    rcodes["100110"] = "/";
    rcodes["100101"] = "_";
    rcodes["100100"] = "-";
    rcodes["100001"] = "~";
    rcodes["011111"] = "x";
    rcodes["011110"] = "w";
    rcodes["011101"] = "z";
    rcodes["011100"] = "y";
    rcodes["011011"] = "t";
    rcodes["011010"] = "s";
    rcodes["011001"] = "v";
    rcodes["011000"] = "u";
    rcodes["010111"] = "5";
    rcodes["010110"] = "4";
    rcodes["010101"] = "7";
    rcodes["010100"] = "6";
    rcodes["010011"] = "1";
    rcodes["010010"] = "0";
    rcodes["010001"] = "3";
    rcodes["010000"] = "2";
    rcodes["001111"] = "h";
    rcodes["001110"] = "g";
    rcodes["001101"] = "j";
    rcodes["001100"] = "i";
    rcodes["001011"] = "d";
    rcodes["001010"] = "c";
    rcodes["001001"] = "f";
    rcodes["001000"] = "e";
    rcodes["000111"] = "p";
    rcodes["000110"] = "o";
    rcodes["000101"] = "r";
    rcodes["000100"] = "q";
    rcodes["000011"] = "l";
    rcodes["000010"] = "k";
    rcodes["000001"] = "n";
    rcodes["000000"] = "m";
    rcodes["1111111"] = "J";
    rcodes["1111110"] = "I";
    rcodes["1111101"] = "L";
    rcodes["1111100"] = "K";
    rcodes["1111011"] = "F";
    rcodes["1111010"] = "E";
    rcodes["1111001"] = "H";
    rcodes["1111000"] = "G";
    rcodes["1110111"] = "R";
    rcodes["1110110"] = "Q";
    rcodes["1110101"] = "T";
    rcodes["1110100"] = "S";
    rcodes["1110011"] = "N";
    rcodes["1110010"] = "M";
    rcodes["1110001"] = "P";
    rcodes["1110000"] = "O";
    rcodes["1101111"] = "Z";
    rcodes["1101110"] = "Y";
    rcodes["1101101"] = " ";
    rcodes["1101100"] = "A";
    rcodes["1101011"] = "V";
    rcodes["1101010"] = "U";
    rcodes["1101001"] = "X";
    rcodes["1101000"] = "W";
    rcodes["1100111"] = "B";
    rcodes["1100101"] = "D";
    rcodes["1100100"] = "C";
}

void create_sites_maps() {
        sites["gh"] = "1";
        sites["bb"] = "2";
        sites["gl"] = "3";

        rsites["1"] = "gh";
        rsites["2"] = "bb";
        rsites["3"] = "gl";
}

void create_helper_maps() {
    names["rev"] = "revision";
    names["file"] = "file name";
    names["repo"] = "user/repo";
    names["site"] = "site";
}

QMap< QString, QString > & getCodes() { return codes; }
QMap< QString, QString > & getRCodes() { return rcodes; }
QMap< QString, QString > & getSites() { return sites; }
QMap< QString, QString > & getRSites() { return rsites; }
QMap< QString, QString > & getNames() { return names; }
