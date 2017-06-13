/** 
 * @file 
 * @author   Jason M. Carter (carterjm@ornl.gov)
 * @author   Aaron E. Ferber (ferberae@ornl.gov)
 * @date     April 2017
 * @version  0.1
 *
 * @copyright Copyright 2017 US DOT - Joint Program Office
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
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

#include <iostream>
#include "osm.hpp"

namespace std {
    size_t hash<osm::Highway>::operator()( const osm::Highway& h ) const
    {
        return static_cast<size_t>( h );
    }
}

namespace osm {

    HighwayMap highway_map = {
        {"motorway",Highway::MOTORWAY},
        {"trunk",Highway::TRUNK},
        {"primary",Highway::PRIMARY},
        {"secondary",Highway::SECONDARY},
        {"tertiary",Highway::TERTIARY },
        {"unclassified",Highway::UNCLASSIFIED },
        {"residential",Highway::RESIDENTIAL },
        {"service",Highway::SERVICE },
        {"motorway_link",Highway::MOTORWAY_LINK },
        {"trunk_link",Highway::TRUNK_LINK },
        {"primary_link",Highway::PRIMARY_LINK },
        {"secondary_link",Highway::SECONDARY_LINK },
        {"tertiary_link",Highway::TERTIARY_LINK },
        {"living_street",Highway::LIVING_STREET },
        {"pedestrian",Highway::PEDESTRIAN },
        {"track",Highway::TRACK },
        {"bus_guideway",Highway::BUS_GUIDEWAY },
        {"raceway",Highway::RACEWAY },
        {"road",Highway::ROAD },
        {"footway",Highway::FOOTWAY },
        {"bridleway",Highway::BRIDLEWAY },
        {"steps",Highway::STEPS },
        {"path",Highway::PATH },
        {"cycleway",Highway::CYCLEWAY },
        {"proposed",Highway::PROPOSED },
        {"construction",Highway::CONSTRUCTION },
        {"bus_stop",Highway::BUS_STOP },
        {"crossing",Highway::CROSSING },
        {"elevator",Highway::ELEVATOR },
        {"emergency_access_point",Highway::EMERGENCY_ACCESS_POINT },
        {"escape",Highway::ESCAPE },
        {"give_way",Highway::GIVE_WAY },
        {"mini_roundabout",Highway::MINI_ROUNDABOUT },
        {"motorway_junction",Highway::MOTORWAY_JUNCTION },
        {"passing_place",Highway::PASSING_PLACE },
        {"rest_area",Highway::REST_AREA },
        {"speed_camera",Highway::SPEED_CAMERA },
        {"street_lamp",Highway::STREET_LAMP },
        {"services",Highway::SERVICES },
        {"stop",Highway::STOP },
        {"traffic_signals",Highway::TRAFFIC_SIGNALS },
        {"turning_circle",Highway::TURNING_CIRCLE },
        {"user_defined",Highway::OTHER }
    };

    HighwayNameMap highway_name_map = {
        {Highway::MOTORWAY,"motorway"},
        {Highway::TRUNK,"trunk"},
        {Highway::PRIMARY,"primary"},
        {Highway::SECONDARY,"secondary"},
        {Highway::TERTIARY ,"tertiary"},
        {Highway::UNCLASSIFIED ,"unclassified"},
        {Highway::RESIDENTIAL ,"residential"},
        {Highway::SERVICE ,"service"},
        {Highway::MOTORWAY_LINK ,"motorway_link"},
        {Highway::TRUNK_LINK ,"trunk_link"},
        {Highway::PRIMARY_LINK,"primary_link" },
        {Highway::SECONDARY_LINK,"secondary_link" },
        {Highway::TERTIARY_LINK,"tertiary_link" },
        {Highway::LIVING_STREET,"living_street" },
        {Highway::PEDESTRIAN,"pedestrian" },
        {Highway::TRACK,"track" },
        {Highway::BUS_GUIDEWAY,"bus_guideway" },
        {Highway::RACEWAY,"raceway" },
        {Highway::ROAD,"road" },
        {Highway::FOOTWAY,"footway" },
        {Highway::BRIDLEWAY,"bridleway" },
        {Highway::STEPS,"steps" },
        {Highway::PATH,"path" },
        {Highway::CYCLEWAY,"cycleway" },
        {Highway::PROPOSED,"proposed" },
        {Highway::CONSTRUCTION,"construction" },
        {Highway::BUS_STOP,"bus_stop" },
        {Highway::CROSSING,"crossing" },
        {Highway::ELEVATOR,"elevator" },
        {Highway::EMERGENCY_ACCESS_POINT,"emergency_access_point" },
        {Highway::ESCAPE,"escape" },
        {Highway::GIVE_WAY,"give_way" },
        {Highway::MINI_ROUNDABOUT,"mini_roundabout" },
        {Highway::MOTORWAY_JUNCTION,"motorway_junction" },
        {Highway::PASSING_PLACE,"passing_place" },
        {Highway::REST_AREA,"rest_area" },
        {Highway::SPEED_CAMERA,"speed_camera" },
        {Highway::STREET_LAMP,"street_lamp" },
        {Highway::SERVICES,"services" },
        {Highway::STOP,"stop" },
        {Highway::TRAFFIC_SIGNALS,"traffic_signals" },
        {Highway::TURNING_CIRCLE,"turning_circle" },
        {Highway::OTHER,"user_defined" }
    };

    HighwayWidthMap highway_width_map = {
        22.0, // motorway
        16.0, // trunk
        30.0, // primary
        17.0, // secondary
        16.0, // tertiary
        22.0, // unclassified
        17.0, // residential
        16.0, // service
        16.0, // motorway_link
        16.0, // trunk_link
        30.0, // primary_link
        18.0, // secondary_link
        16.0, // tertiary_link
        16.0, // living_street
        10.0, // pedestrian
        16.0, // track
        16.0, // bus_guideway
        16.0, // raceway
        16.0, // road
        16.0, // footway
        16.0, // bridleway
        16.0, // steps
        16.0, // path
        16.0, // cycleway
        16.0, // proposed
        16.0, // construction 
        16.0, // bus_stop
        16.0, // crossing
        16.0, // elevator
        16.0, // emergency_access_point
        16.0, // escape
        16.0, // give_way
        16.0, // mini_roundabout
        16.0, // motorway_junction
        16.0, // passing_place
        16.0, // rest_area
        16.0, // speed_camera
        16.0, // street_lamp
        16.0, // services
        16.0, // stop
        16.0, // traffic_signals
        16.0, // turning_circle
        80.0  // user_defined
    };

    HighwaySet highway_blacklist{ Highway::PEDESTRIAN, Highway::SERVICE };

    invalid_way_exception::invalid_way_exception( const Highway& way_type ) :
        runtime_error{"way type excluded from use in quad map"},
        type_{way_type},
        message_{ runtime_error::what() }
    { 
        // count up the number of blacklisted ways found.
        ++count_; 
        message_ += " [" + std::to_string(count_) + "] : ";
        message_ += std::to_string(static_cast<int>(type_));
    }

    const char* invalid_way_exception::what() const noexcept
    {
        // CANNOT CONSTRUCT THE ERROR MESSAGE HERE!!
        return message_.c_str();
    }

    int invalid_way_exception::count_ = 0;

    int invalid_way_exception::occurrences() const
    {
        return count_;
    }
}


