/** 
 * @file 
 * @author Jason M. Carter
 * @author Aaron E. Ferber
 * @date April 2017
 * @version
 *
 * @copyright Copyright 2017 US DOT - Joint Program Office
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contributors:
 *    Oak Ridge National Laboratory.
 */

#include "utilities.hpp"

#include <cmath>

const std::string string_utilities::DELIMITERS = " \f\n\r\t\v";

StrVector string_utilities::split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}
    
std::string& string_utilities::rstrip( std::string& s )
{
  return s.erase( s.find_last_not_of( string_utilities::DELIMITERS ) + 1 );
}

std::string& string_utilities::lstrip( std::string& s )
{
  return s.erase( 0, s.find_first_not_of( string_utilities::DELIMITERS ) );
}

std::string& string_utilities::strip( std::string& s )
{
  return string_utilities::lstrip( rstrip ( s ));
}

StrPair string_utilities::split_attribute( const std::string& s, char delim) {
    StrPair r;
    size_t pos = s.find(delim);
    if (pos < std::string::npos) {
        r.first = s.substr(0,pos);
        pos += 1;
        if (pos < std::string::npos) {
            r.second = s.substr(pos,s.size());
        }
    } 
    return r;
}

bool double_utilities::are_equal(double a, double b, double epsilon) {
    return std::fabs(a - b) < epsilon;
}
