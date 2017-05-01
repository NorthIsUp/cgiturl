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

#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H
#include <vector>
#include <tuple>

std::tuple< std::vector<char>, int> convert_integer_to_base_36( int number );
std::tuple< std::vector<char>, int> numbers_to_letters( const std::vector<int> & digits );
std::tuple< std::vector<int>, int> letters_to_numbers( const std::vector<char> & letters );
std::tuple< std::vector<int>, int> letters_to_numbers( const std::string & letters );
std::tuple< std::vector<char>, int, int > div2( const std::vector<char> & letters );
std::tuple< std::string, int, int > div2( const std::string & letters );
std::tuple< std::vector<int>, int > decode_zcode( const std::string & code );
std::tuple<int, int> get_integer_from_base_36( const std::string & letters );
std::tuple< std::vector<int>, int > arr_01_to_24_bit_pack_numbers( const std::vector<int> & bits );
std::tuple< std::vector<int>, int > str_01_to_24_bit_pack_numbers( const std::string & sbits );
std::tuple< std::vector<int>, int, int > div_24_bit_pack_numbers_36( const std::vector<int> & numbers );
std::tuple< std::vector<char>, std::vector<int>, int > encode_zcode_24_bit_pack_numbers( const std::vector<int> & numbers );
std::tuple< std::vector<char>, std::vector<int>, int > encode_zcode_str01( const std::string & sbits );
std::tuple< std::vector<char>, std::vector<int>, int > encode_zcode_arr01( const std::vector<int> & bits );

#endif // MATH_FUNCTIONS_H
