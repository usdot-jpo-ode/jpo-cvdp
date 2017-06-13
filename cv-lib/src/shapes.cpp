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

#include <algorithm>
#include <fstream>
#include <iomanip>

#include "shapes.hpp"
#include "osm.hpp"
#include "utilities.hpp"

namespace shapes {

using StreamPtr = std::shared_ptr<std::istream>;

CSVInputFactory::CSVInputFactory() :
    file_path_{}
{}

CSVInputFactory::CSVInputFactory(const std::string& file_path) :
    file_path_{file_path}
{}

/**
 * Edge Specification:
 * - line_parts[0] : "edge"
 * - line_parts[1] : unique 64-bit integer identifier
 * - line_parts[2] : A sequence of colon-split points; each point is semi-colon split.
 *      - Point: <uid>;latitude;longitude
 * - line_parts[3] : A sequence of colon-split key=value attributes.
 *      - Attribute Pair: <attribute>=<value>
 */
void CSVInputFactory::make_edge(const StrVector& line_parts) {
    double lat;
    double lon;
    uint64_t edge_id;
    uint64_t vertex_id;
    osm::Highway way_type{osm::Highway::OTHER};                     // default value.

    if ( line_parts.size() < 3) {
        // lines cannot be defined without points.
        throw std::invalid_argument("insufficient number of components to create an edge: " + std::to_string(line_parts.size()) + "; requires 3." );
    }

    // Attributes must be processed first (if they exist) so we pickup the specified way_type.
    if ( line_parts.size() > 3 ) {
        StrVector att_strings = string_utilities::split( line_parts[SHAPE_ATTS], ':' );
        StrStrMap atts;

        for (auto& att_string : att_strings) {
            // att_string format: <attribute>=<value>
            StrPair att = string_utilities::split_attribute( att_string );

            string_utilities::strip( att.first );
            string_utilities::strip( att.second );

            // a pair of empty strings could be returned. If any component is empty do nothing.
            if ( !att.first.empty() && !att.second.empty() ) {
                // a non-empty key and value.
                atts[att.first] = att.second;
            }	
        }

        auto s1 = atts.find("way_type");
        if ( s1 != atts.end() ) {
            // map uses all lower case.
            std::transform( s1->second.begin(), s1->second.end(), s1->second.begin(), ::tolower );
            auto s2 = osm::highway_map.find( s1->second );
            if ( s2 != osm::highway_map.end() ) {
                way_type = s2->second;
            } // othewise, use the default value.
        }

        auto blacklist_item = osm::highway_blacklist.find( way_type );
        if (blacklist_item != osm::highway_blacklist.end()) {
            // this edge type should be ignored since it is in the blacklist.
            throw osm::invalid_way_exception{ way_type };
        }
    }

    edge_id = std::stoull( line_parts[SHAPE_ID] );                  // throws.
    StrVector geo_parts{ string_utilities::split( line_parts[SHAPE_GEOGRAPHY], ':' ) };

    if ( geo_parts.size() != 2 ) {
        // too many or too few points.
        throw std::out_of_range{ "too many or too few points to define an edge: " + std::to_string(geo_parts.size()) };
    }

    geo::Vertex::Ptr vp[2];
    for ( int pi = 0; pi < 2; ++pi ) {

        // A point in a geometry is a triple: uid; latitude; longitude.
        StrVector point_parts{ string_utilities::split( geo_parts[pi], ';' ) };

        if ( point_parts.size() != 3 ) {
            // too many or too few components to define a point -- just skip this point.
            throw std::out_of_range{ "too many or too few elements to define a point: " + std::to_string(point_parts.size()) };
        }

        // convert all the parts so we can perform checks when the id was previously used.
        vertex_id = std::stoull( point_parts[POINT_ID] );           // throws.
        lat = std::stod( point_parts[POINT_LAT] );                  // throws.
        lon = std::stod( point_parts[POINT_LON] );                  // throws.

        auto element_item = vertex_map_.find(vertex_id);
        if (element_item != vertex_map_.end()) {
            // point already defined; use existing instance.
            // needed because we have an incident edge list.
            vp[pi] = vertex_map_[vertex_id];
            if ( !double_utilities::are_equal(vp[pi]->lat, lat, geo::kGPSEpsilon) || !double_utilities::are_equal(vp[pi]->lon, lon, geo::kGPSEpsilon)) {
                std::cerr << "WARNING: identical vertex id with different coordinates!\n";
            }

        } else {
            // point must be instantiated.

            if (lat > 80.0 || lat < -84.0) {
                throw std::out_of_range{ "bad latitude: " + std::to_string(lat) };
            }

            if (lon >= 180.0 || lon <= -180.0) {
                throw std::out_of_range{"bad longitude: " + std::to_string(lon) };
            }

            vp[pi] = std::make_shared<geo::Vertex>(lat,lon,vertex_id);  
            vertex_map_[vertex_id] = vp[pi];
        }    
    }

    if ( vp[0]->uid == vp[1]->uid ) {
        throw std::invalid_argument("The identifiers for the edges points are the same.");
    }

    // NOTE: the way id does not uniquely identify the edge, as a way is sequence of edges.
    geo::EdgePtr edge_ptr = std::make_shared<geo::Edge>( vp[0], vp[1], way_type, edge_id ); 
    vp[0]->add_edge( edge_ptr );
    vp[1]->add_edge( edge_ptr );
    edges_.push_back(edge_ptr);
}

void CSVInputFactory::make_circle(const StrVector& line_parts) 
{
    // Circle Specification:
    // - line_parts[0] : "circle"
    // - line_parts[1] : unique 64-bit integer identifier
    // - line_parts[2] : A sequence of colon-split elements that define the center.
    //      - Center: <lat>:<lon>:<radius in meters>
    // 
    if ( line_parts.size() < 3) {
        // lines cannot be defined without points.
        throw std::invalid_argument("insufficient number of components to create a circle: " + std::to_string(line_parts.size()) + "; requires 3." );
    }

    uint64_t uid = std::stoull(line_parts[1]);

    StrVector parts = string_utilities::split(line_parts[2], ':');

    if ( parts.size() != 3 ) {
	    throw std::out_of_range{ "wrong number of elements for circle center: " + std::to_string( parts.size() ) };
    } 

    double lat = std::stod(parts[0]);

    if (lat > 80.0 || lat < -84.0) {
        throw std::out_of_range{ "bad latitude: " + std::to_string(lat) };
    }

    double lon = std::stod(parts[1]);

    if (lon >= 180.0 || lon <= -180.0) {
        throw std::out_of_range{"bad longitude: " + std::to_string(lon) };
    }

    double radius = std::stod(parts[2]);

    if (radius < 0.0) {
        throw std::out_of_range{"bad radius: " + std::to_string(radius) };
    }
    
    circles_.push_back(std::make_shared<geo::Circle>(lat, lon, uid, radius));
}

void CSVInputFactory::make_grid(const StrVector& line_parts) {

    // Grid Specification:
    // - line_parts[0] : "grid"
    // - line_parts[1] : A '_' split row-column pair.
    // - line_parts[2] : A sequence of colon-split elements defining the grid position.
    //      - Point: <sw lat>:<sw lon>:<ne lat>:<ne lon>
    //
    if ( line_parts.size() < 3) {
        // lines cannot be defined without points.
        throw std::invalid_argument("insufficient number of components to create a grid: " + std::to_string(line_parts.size()) + "; requires 3." );
    }

    StrVector id_parts = string_utilities::split(line_parts[1], '_');
    
    if (id_parts.size() != 2) {
        throw std::out_of_range("geo::Grid missing row/col fields.");
    }

    // id_parts has 2 elements ROW and COL

    uint32_t row = std::stoul(id_parts[0]);
    uint32_t col = std::stoul(id_parts[1]);

    StrVector geo_parts = string_utilities::split(line_parts[2], ':');

    if (geo_parts.size() != 4) {
        throw std::out_of_range("geo::Grid missing bounds data.");
    }

    // geo_parts has 2 points each defined as a pair (lat, lon)

    double sw_lat = std::stod(geo_parts[0]);
    double sw_lon = std::stod(geo_parts[1]);
    double ne_lat = std::stod(geo_parts[2]);
    double ne_lon = std::stod(geo_parts[3]);

    if (sw_lat > 80.0 || sw_lat < -84.0) {
        throw std::out_of_range{ "bad latitude: " + std::to_string(sw_lat) };
    }

    if (sw_lon >= 180.0 || sw_lon <= -180.0) {
        throw std::out_of_range{"bad longitude: " + std::to_string(sw_lon) };
    }

    if (ne_lat > 80.0 || ne_lat < -84.0) {
        throw std::out_of_range{ "bad latitude: " + std::to_string(ne_lat) };
    }

    if (ne_lon >= 180.0 || ne_lon <= -180.0) {
        throw std::out_of_range{"bad longitude: " + std::to_string(ne_lon) };
    }
    
    geo::Bounds bounds(geo::Point(sw_lat, sw_lon), geo::Point(ne_lat, ne_lon));
    geo::Grid::CPtr grid_ptr = std::make_shared<const geo::Grid>(bounds, row, col);
    grids_.push_back(grid_ptr); 
}

void CSVInputFactory::make_shapes() {
    std::string line;
    std::ifstream file(file_path_);

    if (file.fail()) {
        throw std::invalid_argument("Could not open shape file: " + file_path_);
    }

    // Get the header.
    if (!std::getline(file, line)) {
        throw std::invalid_argument("Shape file missing header!");
    }

    while (std::getline(file, line)) {
        try {
            StrVector parts = string_utilities::split(line, ',');

            if (parts.size() < 3 || parts.size() > 4) {
		// Shape file attribute order: type,id,geography[,attributes]
		// First 3 are required; fourth is optional.
                std::cerr << "Too few or too many elements in shape specification: " << parts.size() << " fields.\n";
                continue;
            }

            std::string type = parts[0];

            if (type == "circle") {
                make_circle(parts);
            } else if (type == "edge") {
                make_edge(parts);
            } else if (type == "grid") {
                make_grid(parts);
            }

        } catch (std::exception& e) {
            // Deal with all the exceptions thrown from the make_<shape> methods.
            // Skip the specification and move to the next shape.
            // TODO: need some logging here.
            std::cerr << "Failed to make shape: " << e.what() << std::endl;
        }
    }
    file.close();
}

const std::vector<geo::Circle::CPtr>& CSVInputFactory::get_circles() const {
    return circles_;
} 

const std::vector<geo::EdgeCPtr>& CSVInputFactory::get_edges() const {
    return edges_;
}

const std::vector<geo::Grid::CPtr>& CSVInputFactory::get_grids() const {
    return grids_;
}

CSVOutputFactory::CSVOutputFactory(const std::string& file_path) :
    file_path_{file_path}
    {}

void CSVOutputFactory::add_circle(geo::Circle::CPtr circle_ptr) {
    circles_.push_back(circle_ptr);
}

void CSVOutputFactory::add_edge(geo::EdgeCPtr edge_ptr) {
    edges_.push_back(edge_ptr);
}

void CSVOutputFactory::add_grid(geo::Grid::CPtr grid_ptr) {
    grids_.push_back(grid_ptr);
}

void CSVOutputFactory::write_circle(std::ofstream& os, geo::Circle::CPtr circle_ptr) const {
    os << std::setprecision(16) << "circle," << circle_ptr->uid << "," << circle_ptr->lat << ":" << circle_ptr->lon << ":" << circle_ptr->radius << std::endl;
}

void CSVOutputFactory::write_edge(std::ofstream& os, geo::EdgeCPtr edge_ptr) const {
    std::string highway_name;

    try {
        highway_name = osm::highway_name_map.at(edge_ptr->get_way_type());
    } catch (std::out_of_range&) {
        highway_name = "unknown";
    }

    os << std::setprecision(16) << "edge," << edge_ptr->get_uid() << "," << edge_ptr->v1->uid << ";" << edge_ptr->v1->lat << ";" << edge_ptr->v1->lon << ":" << edge_ptr->v2->uid << ";" << edge_ptr->v2->lat << ";" << edge_ptr->v2->lon << ",way_type=" << highway_name << ":way_id=" << edge_ptr->get_uid() << std::endl;
}

void CSVOutputFactory::write_grid(std::ofstream& os, geo::Grid::CPtr grid_ptr) const {
    os << "grid," << std::setprecision(16) << grid_ptr->row << "_" << grid_ptr->col << "," << grid_ptr->sw.lat << ":" << grid_ptr->sw.lon << ":" << grid_ptr->ne.lat << ":" << grid_ptr->ne.lon << std::endl;
}

void CSVOutputFactory::write_shapes() const {
    std::ofstream file(file_path_, std::ofstream::trunc);

    if (file.fail()) {
        throw std::invalid_argument("Could not open shape file: " + file_path_);
    }

    file << "type,id,geography,attributes" << std::endl;

    for (auto& circle_ptr : circles_) {
        write_circle(file, circle_ptr);
    }        

    for (auto& edge_ptr : edges_) {
        write_edge(file, edge_ptr);
    }

    for (auto& grid_ptr : grids_) {
        write_grid(file, grid_ptr);
    }

    file.close();
}

}  // end namespace shapes
