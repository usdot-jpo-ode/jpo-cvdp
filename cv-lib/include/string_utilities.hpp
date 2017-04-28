/** 
 * @file 
 * @author Jason M. Carter
 * @author Aaron E. Ferber
 * @date April 2017
 * @version
 *
 * @copyright Copyright 2017 US DOT - Joint Program Office
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef CTES_STRING_UTILITIES_H
#define CTES_STRING_UTILITIES_H

#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

using StrVector     = std::vector<std::string>;                             ///< Alias for a vector of strings.
using StrVectorPtr  = std::shared_ptr<std::vector<std::string>>;            ///< Alias for a pointer to a vector of strings.
using StrVectorCPtr = std::shared_ptr<const std::vector<std::string>>;      ///< Alias for a pointer to a constant vector of strings.
using StrPair       = std::pair<std::string,std::string>;                   ///< Alias for a pair of strings.
using StrSet        = std::unordered_set<std::string>;                      ///< Alias for a set of strings.
using StrStrMap     = std::unordered_map<std::string,std::string>;          ///< Alias for a string to string map.

namespace string_utilities {

extern const std::string DELIMITERS;

/**
 * @brief Split the provided string at every occurrence of delim and put the results in the templated type T.
 *
 * @param s the string to split.
 * @param delim the char where the splits are to be performed.
 * @param T the type where the string components are put.
 */
template<typename T>
    void split(const std::string &s, char delim, T result) {
        std::stringstream ss;
        ss.str(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            *(result++) = item;
        }
    }

/**
 * @brief Split the provided string at every occurrence of delim and return the components in a vector of strings.
 *
 * @param s the string to split.
 * @param delim the char where the splits are to be performed; default is ','
 * @return a vector of strings.
 */
StrVector split(const std::string &s, char delim = ',');


/**
 * @brief Remove the whitespace from the right side of the string; this is done
 * in-place; no copy happens.
 *
 * @param s the string to trim.
 * @return the new string without the whitespace on the right side.
 */
std::string& rstrip( std::string& s );

/**
 * @brief Remove the whitespace from the left side of the string to the first
 * non-whitespace character; this is done in-place; no copy happens.
 *
 * @param s the string to trim.
 * @return the new string without the whitespace on the left side.
 */
std::string& lstrip( std::string& s );

/**
 * @brief Remove the whitespace surrounding the string; this is done in-place; no copy
 * happens.
 *
 * @param s the string to trim.
 * @return the new string with the whitespace before and after removed.
 */
std::string& strip( std::string& s );

/**
 * @brief Split and attribute pair on the provided delimiter and return the pair of strings.
 *
 * @param s The string attribute pair, e.g., <attr>=<value>
 * @param delim The character delimiter that splits the attribute pair, e.g., '='
 * @return a pair of strings (std::pair)
 */
StrPair split_attribute( const std::string& s, char delim = '=' );

}  // end namespace.

#endif
