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

#ifndef CODING_FUNCTIONS_H
#define CODING_FUNCTIONS_H
#include <tuple>
#include <vector>
#include <string>
#include <QString>
#include <QMap>

std::tuple< std::vector<std::string> , int > setIndex(int index);

void create_codes_map();
void create_rcodes_map();
void create_sites_maps();
void create_helper_maps();

QMap< QString, QString > & getCodes();
QMap< QString, QString > & getRCodes();
QMap< QString, QString > & getSites();
QMap< QString, QString > & getRSites();
QMap< QString, QString > & getNames();

std::tuple< int, QMap<QString,QString>, int > process_meta_data( const std::vector<int> & _bits );
int BitsStart( std::vector<int> & dest );
std::tuple<int, QStringList> BitsWithPreamble( std::vector<int> & dest, const QString & type, const QString & data );
int BitsStop( std::vector<int> & dest );
std::tuple< bool, int > BitsCompareSuffix( const std::vector<int> & bits, const QString & strBits );
int BitsRemoveIfStartStop( std::vector<int> & bits );
std::tuple< bool, int > BitsCompareSuffix( const std::vector<int> & bits, const QString & strBits );
int insertBitsFromStrBits( std::vector<int> & dest, const QString & str );

#endif // CODING_FUNCTIONS_H
