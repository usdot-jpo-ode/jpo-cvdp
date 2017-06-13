/** 
 * @file 
 * @author   Jason M. Carter (carterjm@ornl.gov)
 * @author   Aaron E. Ferber (ferberae@ornl.gov)
 * @date     April 2017
 * @version  0.1
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
 *    Oak Ridge National Laboratory, Center for Trustworthy Embedded Systems, UT Battelle.
 */

#ifndef CVDP_DI_OSM_HPP
#define CVDP_DI_OSM_HPP

#include "names.hpp"
#include <exception>
#include <sstream>

namespace osm {

/**
 * @brief Pertains to our custom Shape files; these are names for indices to fields within the comma-separated file.
 */
enum class Fields { TYPE = 0, ID = 1, GEOGRAPHY = 2, ATTRIBUTES = 3 };

/**
 * @brief Related to our custom Shape files; For a Point geometry, these are indices to the different elements.
 */
enum class PtFields { ID = 0, LAT = 1, LON = 2 };

/**
 * @brief Related to our custom Shape files or a splittable attribute string. For an attribute, these are indices to the
 * components of the key-value pair.
 */
enum class AttFields { KEY = 0, VALUE = 1 };

/**
 * @brief An enumeration of all the OSM highway types. These are used to include and exclude certain road types from a
 * de-identification procedures.
 */
enum class Highway {
    MOTORWAY = 0,
    TRUNK = 1,
    PRIMARY = 2,
    SECONDARY = 3,
    TERTIARY = 4,
    UNCLASSIFIED = 5,
    RESIDENTIAL = 6,
    SERVICE = 7,
    MOTORWAY_LINK = 8,
    TRUNK_LINK = 9,
    PRIMARY_LINK = 10,
    SECONDARY_LINK = 11,
    TERTIARY_LINK = 12,
    LIVING_STREET = 13,
    PEDESTRIAN = 14,
    TRACK = 15,
    BUS_GUIDEWAY = 16,
    RACEWAY = 17,
    ROAD = 18,
    FOOTWAY = 19,
    BRIDLEWAY = 20,
    STEPS = 21,
    PATH = 22,
    CYCLEWAY = 23,
    PROPOSED = 24,
    CONSTRUCTION = 25,
    BUS_STOP = 26,
    CROSSING = 27,
    ELEVATOR = 28,
    EMERGENCY_ACCESS_POINT = 29,
    ESCAPE = 30,
    GIVE_WAY = 31,
    MINI_ROUNDABOUT = 32,
    MOTORWAY_JUNCTION = 33,
    PASSING_PLACE = 34,
    REST_AREA = 35,
    SPEED_CAMERA = 36,
    STREET_LAMP = 37,
    SERVICES = 38,
    STOP = 39,
    TRAFFIC_SIGNALS = 40,
    TURNING_CIRCLE = 41,
    OTHER = 42
};

using HighwaySet = std::unordered_set<Highway>;                         ///< Alias for a set of OSM Highway types.

// return type from a set find operation.
using HighwayMap = std::unordered_map<std::string,Highway>;             ///< A type for a lookup table to find the Highway type associated with its string name.
using HighwayNameMap = std::unordered_map<Highway,std::string>;         ///< A type for a lookup table to find the string name of the Highway given its enumeration type.
using HighwayWidthMap = std::vector<double>;                            ///< A type for a lookup vector where positions correspond to the order of the Highway enumeration type.

extern HighwaySet highway_blacklist;                                    ///< The set of Highway types that should not be included when building the quad tree structure.
extern HighwayMap highway_map;                                          ///< A lookup table to find the Highway type associated with its string name. 
extern HighwayNameMap highway_name_map;                                 ///< A lookup table to find the string name for a Highway enumeration type.
extern HighwayWidthMap highway_width_map;                               ///< A lookup vector to find the widths associated with Highway types; the order is the same as the Highway enumeration.

/**
 * @brief An exception triggered when a way type is provided but is in the exclusion list for building the quad tree.
 */
class invalid_way_exception : public std::runtime_error
{
    private:
        static int count_;               ///< The number of times the exception was triggered during a run.
        std::string message_;            ///< The exception message;
        Highway type_;                   ///< The Highway type that triggered the exception.

    public:

        /**
         * @brief Construct an Invalid Way Exception provided the way that triggered the exception.
         *
         * @param way_type The way that triggered the exception.
         */
        invalid_way_exception( const Highway& way_type );

        /**
         * @brief Return the exception message string.
         *
         * @return The exception message as a constant pointer to a character string.
         */
        virtual const char* what() const noexcept;

        /**
         * @brief Get the number of times an invalid_way_exception was triggered.
         *
         * @return A count of the number of times an invalid way was encountered as an integer.
         */
        int occurrences() const;
};

}  // end namespace osm

namespace std {

template<> struct hash<osm::Highway>
{
    
    /**
     * @brief A hashcode for a Highway type.
     *
     * @return return a correctly cast enumeration value as a size_t type.
     */
    size_t operator()( const osm::Highway& h ) const;
};

} // end namespace std

#endif
