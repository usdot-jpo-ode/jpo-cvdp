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
#include <cmath>
#include <iomanip>
#include <sstream>

#include "entity.hpp"
#include "utilities.hpp"

namespace geo {

Point::Point() :
    lat{0.0},
    lon{0.0}
{}

Point::Point( double lat, double lon ) :
    lat{lat},
    lon{lon}
{}

Point::Point( const Point& pt ) :
    lat{pt.lat},
    lon{pt.lon}
{}

bool Point::operator==( const Point& other ) const
{
    return (double_utilities::are_equal(lat, other.lat, kGPSEpsilon) && double_utilities::are_equal(lon, other.lon, kGPSEpsilon));
}

// Static location methods
double Location::distance( const Location& loc1, const Location& loc2 ) 
{
    double x = (loc2.lonr - loc1.lonr) * std::cos( (loc1.latr + loc2.latr ) / 2.0 );
    double y = (loc2.latr - loc1.latr);
    return std::sqrt( x*x + y*y ) * kEarthRadiusM;
}

double Location::distance( double lat1, double lon1, double lat2, double lon2 ) 
{
    Location loc1{ lat1, lon1 };
    Location loc2{ lat2, lon2 };
    return distance( loc1, loc2 );
} 

double Location::distance_haversine( const Location& loc1, const Location& loc2 )
{
    double x = std::sin( to_radians(loc2.lat - loc1.lat) / 2.0);
    double y = std::sin( to_radians(loc2.lon - loc1.lon) / 2.0);

    double a = x*x + std::cos( loc1.latr ) * std::cos( loc2.latr ) * y*y;
    double c = 2.0 * std::asin(std::sqrt(a));
    return c * kEarthRadiusM;
}

double Location::distance_haversine( double lat1, double lon1, double lat2, double lon2 )
{
    Location loc1{ lat1, lon1 };
    Location loc2{ lat2, lon2 };
    return distance_haversine( loc1, loc2 );
}

Location Location::project_position( const Location& loc, double bearing, double distance )
{
    double latr;
    double lonr;
    double lon;

    distance /= kEarthRadiusM;
    bearing = to_radians(bearing);

    latr = std::asin( std::sin(loc.latr) * std::cos(distance) + std::cos(loc.latr) * std::sin(distance) * std::cos(bearing));
    lonr = loc.lonr + std::atan2( std::sin(bearing) * std::sin(distance) * std::cos(loc.latr), std::cos(distance) - std::sin(loc.latr) * std::sin(latr));
    lon = to_degrees(lonr);
    lon = std::fmod(lon + 540.0, 360.0) - 180.0;

    return Location(to_degrees(latr), lon);
}

Location Location::project_position( double lat, double lon, double bearing, double distance )
{
    Location loc{ lat, lon };
    return project_position( loc, bearing, distance );
}

Location Location::midpoint( const Location& loc1, const Location& loc2 )
{
    double latr;
    double lonr;
    double lon;

    double t = std::cos( loc2.latr );
    double d = loc2.lonr - loc1.lonr;

    double bx = t * std::cos(d);
    double by = t * std::sin(d);

    latr = std::atan2( std::sin( loc1.latr ) + std::sin( loc2.latr ), std::sqrt( ( std::cos( loc1.latr ) + bx ) * ( std::cos( loc1.latr ) + bx ) + by*by ));
    lonr = loc1.lonr + std::atan2( by, std::cos( loc1.latr ) + bx );
    lon = to_degrees(lonr);
    lon = std::fmod(lon + 540.0, 360.0) - 180.0;

    return Location(to_degrees(latr), lon);
}

Location Location::midpoint( double lat1, double lon1, double lat2, double lon2 )
{
    Location loc1{ lat1, lon1 };
    Location loc2{ lat2, lon2 };
    return midpoint( loc1, loc2 );
}

double Location::bearing(const Location& location_a, const Location& location_b) 
{
    double lon_delta = location_b.lonr - location_a.lonr;
    
    double x = std::sin(lon_delta) * std::cos(location_b.latr);
    double y = std::cos(location_a.latr) * std::sin(location_b.latr) - std::sin(location_a.latr) * std::cos(location_b.latr) * std::cos(lon_delta);
    
    return std::fmod(to_degrees(std::atan2(x, y)) + 360.0, 360.0);
}

double Location::bearing( double latitude_a, double longitude_a, double latitude_b, double longitude_b )
{
    Location location_a(latitude_a, longitude_a);
    Location location_b(latitude_b, longitude_b);

    return bearing(location_a, location_b);
}

Location::Location( double lat, double lon, uint64_t uid ) : 
    Point{ lat, lon }, 
    uid{ uid }, 
    latr{ to_radians( lat ) }, 
    lonr{ to_radians( lon ) }
{}

Location::Location( double lat, double lon ) : 
    Location::Location{ lat, lon, 0 }
{}

const std::string Location::get_type() const {
    return "location";
}

bool Location::touches(const Bounds& bounds) const {
    return bounds.contains(*this); 
}

double Location::distance_to( const Location& loc ) const
{
    return Location::distance( *this, loc );
}

double Location::distance_to_haversine( const Location& loc ) const
{
    return Location::distance_haversine( *this, loc );
}

Location Location::midpoint( const Location& loc )
{
    return Location::midpoint( *this, loc );
}

Location Location::project_position( double bearing, double distance ) const
{
    return Location::project_position( *this, bearing, distance );
}

double Location::bearing_to(const Location& location) {
    return Location::bearing( *this, location);
}

bool Location::operator==( const Location& other ) const
{
    return (double_utilities::are_equal(lat, other.lat, kGPSEpsilon) && double_utilities::are_equal(lon, other.lon, kGPSEpsilon) && uid==other.uid);
}

Vertex::Vertex( double lat, double lon ) : 
    Location{ lat, lon },
    edges_{}
{}

Vertex::Vertex( double lat, double lon, uint64_t uid ) : 
    Location{ lat, lon , uid },
    edges_{}
{}

Vertex::Vertex( const Location& loc ) : 
    Vertex{ loc.lat, loc.lon, loc.uid }
{}

uint32_t Vertex::degree() const
{
    return static_cast<uint32_t>(edges_.size());
}

uint32_t Vertex::outdegree() const
{
    uint32_t d = degree();

    if (d > 2) {
        return d - 1;
    }
    return 0;
}

bool Vertex::add_edge( EdgePtr eptr )
{
    auto result = edges_.insert( eptr );

    return result.second;
}

bool Vertex::add_edges( EdgePtrSet& eptrs )
{
    uint32_t before = degree();
    edges_.insert( eptrs.begin(), eptrs.end() );

    return (degree() > before);
}

void Vertex::update_location( const Location& loc )
{
    uid = loc.uid;
    lat = loc.lat;
    lon = loc.lon;
    latr = loc.latr;
    lonr = loc.lonr;
}

bool Vertex::is_same_point(const Point& p) {
    return double_utilities::are_equal(lat, p.lat, kGPSEpsilon) && double_utilities::are_equal(lon, p.lon, kGPSEpsilon);
}

const EdgePtrSet& Vertex::get_incident_edges() const
{
    return edges_;
}

Edge::Edge( const Vertex::Ptr& vp1, const Vertex::Ptr& vp2, const osm::Highway type, uint64_t id, bool explicit_edge ) :
    v1{ vp1 },
    v2{ vp2 },
    uid_{id},
    way_type_{type},
    explicit_edge_{explicit_edge}
{
}

Edge::Edge( const Vertex::Ptr& vp1, const Vertex::Ptr& vp2, uint64_t id, bool explicit_edge ) :
    Edge{ vp1, vp2, osm::Highway::OTHER, id, explicit_edge }
{
}


Edge::Edge( const Vertex& v1, const Vertex& v2, bool explicit_edge ) :
    Edge{ std::make_shared<Vertex>(v1), std::make_shared<Vertex>(v2), osm::Highway::OTHER, 0, explicit_edge }
{
}

Edge::Edge( const Vertex& v1, const Vertex& v2, uint64_t id, bool explicit_edge ) :
    Edge{ std::make_shared<Vertex>(v1), std::make_shared<Vertex>(v2), osm::Highway::OTHER, id, explicit_edge }
{
}

const std::string Edge::get_type(void) const {
    return "edge";
}

bool Edge::touches(const Bounds& bounds) const {
    return bounds.contains_or_intersects(*this);
}

bool Edge::operator==( const Edge& other ) const
{
    return (v1->is_same_point(*(other.v1)) && v2->is_same_point(*(other.v2))) ||
           (v1->is_same_point(*(other.v2)) && v2->is_same_point(*(other.v1)));
}

bool Edge::is_explicit() const
{
    return explicit_edge_;
}

bool Edge::is_implicit() const
{
    return !explicit_edge_;
}

osm::Highway Edge::get_way_type() const
{
    return way_type_;
}

int Edge::get_way_type_index() const
{
    return static_cast<int>( way_type_ );
}

uint64_t Edge::get_uid() const
{
    return uid_;
}

double Edge::get_way_width() const
{
    return osm::highway_width_map[static_cast<int>(way_type_)];
}

double Edge::dlatitude() const
{
    return v2->lat - v1->lat;
}

double Edge::dlongitude() const
{
    return v2->lon - v1->lon;
}

double Edge::length() const
{
    return Location::distance( *v1, *v2 );
}

double Edge::distance_from_point(const Location& loc) const {
    double diff1 = v1->lat - v2->lat;
    double diff2 = v1->lon - v2->lon;
    
    double dist_squared = diff1 * diff1 + diff2 * diff2;

    double diff_lat = loc.lat - v1->lat;
    double diff_lon =  loc.lon - v1->lon;

    double diff_lat_b = v2->lat - v1->lat;
    double diff_lon_b = v2->lon - v1->lon;
    double dot = diff_lat * diff_lat_b + diff_lon * diff_lon_b;

    if (double_utilities::are_equal(dist_squared, 0.0, kGPSEpsilon)) {
        return 0.0;
    }

    double t = dot / dist_squared;

    if (t <= 0.0) {
        return Location::distance(loc, *v1);
    } 

    if (t >= 1.0) {
        return Location::distance(loc, *v2);
    }

    Location tmp(v1->lat + (t * (v2->lat - v1->lat)), v1->lon + (t * (v2->lon - v1->lon)));

    return Location::distance(tmp, loc);
}

double Edge::bearing() const
{
    double lon_delta =  v2->lonr - v1->lonr; 
    double x = std::sin(lon_delta) * std::cos( v2->latr );
    double y = (std::cos(v1->latr) * std::sin(v2->latr)) - (std::sin(v1->latr) * std::cos(v2->latr) * std::cos(lon_delta));
    return std::fmod(to_degrees(std::atan2(x,y)) + 360.0,360.0);
}

double Edge::length_haversine() const
{
    return Location::distance_haversine( *v1, *v2 );
}

bool Edge::intersects( double lat1, double lon1, double lat2, double lon2 ) const
{
    double edgedlat = lat2 - lat1;
    double edgedlon = lon2 - lon1;

    double d = -edgedlat * dlongitude() + dlatitude() * edgedlon;

    if (double_utilities::are_equal(d, 0, kGPSEpsilon)) {
        return false;
    }

    double xdlat = v1->lat - lat1;
    double xdlon = v1->lon - lon1;

    double s = (-dlongitude() * xdlat + dlatitude() * xdlon) / d;
    double t = (edgedlat * xdlon - edgedlon * xdlat) / d;

    return s >= 0.0 && s <= 1.0 && t >= 0.0 && t <= 1.0;
}

bool Edge::intersects( const Edge& edge ) const
{
    return intersects( edge.v1->lat, edge.v1->lon, edge.v2->lat, edge.v2->lon );
}

AreaPtr Edge::to_area() const
{
    return to_area( get_way_width(), 0.0 );
}

AreaPtr Edge::to_area( double extension ) const
{
    return to_area( get_way_width(), extension );
}

AreaPtr Edge::to_area( double cap_width, double extension ) const
{
    double half_width;
    double ab_bearing;
    double x_bearing;
    double y_bearing;
    Vertex::Ptr v1_tmp;
    Vertex::Ptr v2_tmp;

    if (cap_width <= 0.0) {
        throw ZeroAreaException();
    }

    half_width = cap_width / 2.0;
    ab_bearing = v1->bearing_to(*v2);

    v1_tmp = v1; 
    v2_tmp = v2; 

    if (extension > 0.0) {
        // Extend the nodes of this edge.
        v1_tmp = std::make_shared<Vertex>(v1_tmp->project_position(std::fmod(ab_bearing - 180.0, 360.0), extension));
        v2_tmp = std::make_shared<Vertex>(v2_tmp->project_position(ab_bearing, extension));
    }

    // Get the bearing to the area corners.
    x_bearing = std::fmod(ab_bearing - 90.0, 360.0);
    y_bearing = std::fmod(ab_bearing + 90.0, 360.0);
    
    // Get the locations of the corners and return the area.
    return std::make_shared<Area>(v1_tmp->project_position(x_bearing, half_width),
        v2_tmp->project_position(x_bearing, half_width),
        v2_tmp->project_position(y_bearing, half_width),
        v1_tmp->project_position(y_bearing, half_width));
}

Area::Area( const Point& p1, const Point& p2, const Point& p3, const Point& p4 ) :
    corners_{}
{
    corners_.push_back( p1 );
    corners_.push_back( p2 );
    corners_.push_back( p3 );
    corners_.push_back( p4 );
}

Area::Area( const Point&& p1, const Point&& p2, const Point&& p3, const Point&& p4 ) :
    corners_{}
{
    corners_.push_back( p1 );
    corners_.push_back( p2 );
    corners_.push_back( p3 );
    corners_.push_back( p4 );
}

const std::string Area::get_type() const {
    return "area";
}

bool Area::touches(const Bounds& bounds) const {
    if (bounds.contains(corners_[0]) || bounds.contains(corners_[1]) || bounds.contains(corners_[2]) || bounds.contains(corners_[3])) {
        return true;
    }

    if (contains(bounds.sw) || contains(bounds.nw) || contains(bounds.se) || contains(bounds.ne)) {
        return true;
    }

    return bounds.intersects(corners_[0], corners_[1]) || bounds.intersects(corners_[1], corners_[2]) || bounds.intersects(corners_[2], corners_[3]) || bounds.intersects(corners_[3], corners_[0]);
}

const std::vector<Point>& Area::get_corners() const {
    return corners_;
}

bool Area::outside_edge( int p1, const Point& pt ) const
{
    //   See the documentation for the method "relationship" in class Line.
    //   
    //                edge[1]
    //        (c1) +----->-----+ (c2)
    //             |           |
    //             |           |
    //    edge[0]  ^           | edge[2]
    //             |           |
    //             |           |
    //        (c0) +-----<-----+ (c3)
    //                edge[3]
    //   
    //   p1 is actually the index of the edge that you want starting at 0.
    
    if (p1 < 0 || p1 > 3) return false;

    // p1 is the index of the first point that defines the edge of interest.
    // p1+1%4 is the index of the second point that defines the edge of interest.
    int p2 = (p1 + 1) % 4;

    double C = corners_[p1].lat * ( corners_[p2].lon - corners_[p1].lon ) - corners_[p1].lon * ( corners_[p2].lat - corners_[p1].lat );
    double D = -pt.lat * ( corners_[p2].lon - corners_[p1].lon ) + pt.lon * ( corners_[p2].lat - corners_[p1].lat ) + C;

    // negative D indicates pt is to the left of a line from p1 to p2.
    return (D < 0.0);
}

bool Area::contains( const Point& pt ) const
{
    return !(outside_edge( 0, pt ) ||
             outside_edge( 1, pt ) ||
             outside_edge( 2, pt ) ||
             outside_edge( 3, pt ));
}

const std::string Area::get_poly_string() const
{
    std::ostringstream ss;
    ss << std::setprecision(10) << std::dec;

    for ( auto& pt : corners_ ) {
        ss << pt.lon << "," << pt.lat << ",0 ";
    }

    ss << corners_[0].lon << "," << corners_[0].lat << ",0";

    return ss.str();
}

Circle::Circle(const Location& location, double radius) :
    Location(location.lat, location.lon, location.uid),
    radius(radius),
    north(project_position(location.lat, location.lon, 0.0, radius)),
    south(project_position(location.lat, location.lon, 180.0, radius)),
    east(project_position(location.lat, location.lon, 90.0, radius)),
    west(project_position(location.lat, location.lon, 270.0, radius))
    {}

Circle::Circle(double latitude, double longitude, double radius) :
    Location(latitude, longitude),
    radius(radius),
    north(project_position(latitude, longitude, 0.0, radius)),
    south(project_position(latitude, longitude, 180.0, radius)),
    east(project_position(latitude, longitude, 90.0, radius)),
    west(project_position(latitude, longitude, 270.0, radius))
    {}

Circle::Circle(double latitude, double longitude, uint64_t uid, double radius) :
    Location(latitude, longitude, uid),
    radius(radius),
    north(project_position(latitude, longitude, 0.0, radius)),
    south(project_position(latitude, longitude, 180.0, radius)),
    east(project_position(latitude, longitude, 90.0, radius)),
    west(project_position(latitude, longitude, 270.0, radius))
    {}

const std::string Circle::get_type(void) const {
    return "circle";
}

bool Circle::touches(const Bounds& bounds) const {
    bool cardinals_within_bounds = bounds.contains(north) || bounds.contains(south) || bounds.contains(east) || bounds.contains(west);

    if (bounds.contains(*this) || cardinals_within_bounds) {
        return true;
    }

    // No point is within the bounds.
    // Check if the bounds is strictly contained within this circle.
    return contains(bounds.nw) || contains(bounds.ne) || contains(bounds.se) || contains(bounds.sw);
}

bool Circle::contains(const Point& point) const {
   return distance(lat, lon, point.lat, point.lon) <= radius;
}

bool Circle::operator==(const Circle& other) const {
    return double_utilities::are_equal(other.lat, lat, kGPSEpsilon) && double_utilities::are_equal(other.lon, lon, kGPSEpsilon) && double_utilities::are_equal(radius, other.radius, kGPSEpsilon);
}

Bounds::Bounds() :
    nw{ Point{0.0,0.0} },
    ne{ Point{0.0,0.0} },
    se{ Point{0.0,0.0} },
    sw{ Point{0.0,0.0} }
{}

Bounds::Bounds( const Point& swloc, const Point& neloc ) :
    nw{ Point{ neloc.lat, swloc.lon } },
    ne{ Point{ neloc.lat, neloc.lon } },
    se{ Point{ swloc.lat, neloc.lon } },
    sw{ Point{ swloc.lat, swloc.lon } }
{}

Bounds::Bounds( const Bounds& bounds ) :
    nw{ bounds.nw },
    ne{ bounds.ne },
    se{ bounds.se },
    sw{ bounds.sw }
{}

bool Bounds::contains( const Point& pt ) const
{
    return  (sw.lat <= pt.lat && pt.lat <= ne.lat && sw.lon <= pt.lon && pt.lon <= ne.lon);
}

bool Bounds::contains( const Edge& edge ) const
{
    return ( contains( *(edge.v1) ) && contains( *(edge.v2) ) );
}

bool Bounds::contains( const Circle& circle ) const
{
    return contains(circle.north) && contains(circle.south) && contains(circle.east) && contains(circle.west);
}

bool Bounds::contains_or_intersects( const Edge& e ) const
{
    return contains( *(e.v1) ) || contains( *(e.v2) ) || intersects( e );
}

bool Bounds::intersects( const Edge& e ) const
{
    if (e.intersects(sw.lat, sw.lon, ne.lat, sw.lon)) return true;
    if (e.intersects(ne.lat, sw.lon, ne.lat, ne.lon)) return true;
    if (e.intersects(ne.lat, ne.lon, sw.lat, ne.lon)) return true;
    if (e.intersects(sw.lat, sw.lon, sw.lat, ne.lon)) return true;
    return false;
}

bool Bounds::intersects(const Point& pt_a, const Point& pt_b) const {
    Edge e(Vertex(pt_a.lat, pt_a.lon), Vertex(pt_b.lat, pt_b.lon));
    return intersects(e);
}

bool Bounds::intersects( const Circle& circle ) const {
    return intersects(circle.north, circle.east) || intersects(circle.east, circle.south) || intersects(circle.south, circle.west) || intersects(circle.west, circle.north);
}

bool Bounds::contains_or_intersects(const Circle& circle ) const {
    return intersects(circle) || contains(circle);
}

Point Bounds::west_midpoint() const
{
    return Point{ sw.lat + height()/2.0, sw.lon };
}

Point Bounds::east_midpoint() const
{
    return Point{ sw.lat + height()/2.0, ne.lon };
}

Point Bounds::north_midpoint() const
{
    return Point{ ne.lat, sw.lon + width()/2.0 };
}

Point Bounds::south_midpoint() const
{
    return Point{ sw.lat, sw.lon + width()/2.0 };
}

Point Bounds::center() const
{
    return Point{ sw.lat + height()/2.0, sw.lon + width()/2.0 };
}

double Bounds::width() const
{
    return ne.lon - sw.lon;
}

double Bounds::height() const
{
    return ne.lat - sw.lat;
}

Grid::Grid(const geo::Bounds& bounds, uint32_t row, uint32_t col) :
    geo::Bounds{bounds},
    row{row},
    col{col}
    {}

Grid::Grid(const geo::Point& sw_loc, const geo::Point& ne_loc, uint32_t row, uint32_t col) :
    geo::Bounds{sw_loc, ne_loc},
    row{row},
    col{col}
    {}

const std::string Grid::get_type() const {
    return "grid";
}
   
bool Grid::touches(const geo::Bounds& bounds) const {
    if (bounds.contains(sw) || bounds.contains(ne) || bounds.contains(se) || bounds.contains(nw)) {
        return true;
    } 

    if (contains(bounds.sw) || contains(bounds.ne) || contains(bounds.se) || contains(bounds.nw)) {
        return true;
    }
    return false;
}

Grid::GridPtrVector Grid::build_grid(const geo::Location& nw_point, double grid_width, double lat_threshold, double lon_threshold) {
    GridPtrVector ret;
    uint32_t row = 0;
    uint32_t col = 0;
    double height_nw_lat = nw_point.lat;
    double height_nw_lon = nw_point.lon;
    double width_nw_lat = nw_point.lat;
    double width_nw_lon = nw_point.lon;
    bool has_next_nw = false;
    double next_height_nw_lat = 0.0;
    double next_height_nw_lon = 0.0;

    while (height_nw_lat > lat_threshold) {
        col = 0; 
    
        width_nw_lat = height_nw_lat;
        width_nw_lon = height_nw_lon;
        has_next_nw = false;

        while (width_nw_lon < lon_threshold) {
            geo::Location sw_node = geo::Location::project_position(width_nw_lat, width_nw_lon, 180.0, grid_width);   
            geo::Location ne_node = geo::Location::project_position(width_nw_lat, width_nw_lon, 90.0, grid_width);  
            sw_node.lon = width_nw_lon;
            ne_node.lat = width_nw_lat;

            ret.push_back(std::make_shared<const Grid>(sw_node, ne_node, row, col));
            
            if (!has_next_nw) {
                next_height_nw_lat = sw_node.lat;
                next_height_nw_lon = sw_node.lon;
                has_next_nw = true;
            }
        
            width_nw_lat = ne_node.lat;
            width_nw_lon = ne_node.lon;
            col++;
        } 

        height_nw_lat = next_height_nw_lat;
        height_nw_lon = next_height_nw_lon;
        row++;
    }

    return ret;
}

std::ostream& operator<< (std::ostream& os, const Grid& grid) 
{
    return os << grid.sw << "," << grid.ne << "," << grid.row << "," << grid.col; 
}

std::ostream& operator<<( std::ostream& os, const Point& loc )
{
    return os << std::setprecision(16) << loc.lat << "," << loc.lon;
}

std::ostream& operator<<( std::ostream& os, const Bounds& loc )
{
    return os << loc.sw << "," << loc.ne;
}

std::ostream& operator<<( std::ostream& os, const Location& loc )
{
    return os << std::setprecision(16) << loc.uid << "," << loc.lat << "," << loc.lon;
}

std::ostream& operator<< (std::ostream& os, const Vertex& vertex)
{
    return os << std::setprecision(16) << vertex.uid << "," << vertex.lat << "," << vertex.lon << "," << vertex.degree();
}

std::ostream& operator<< (std::ostream& os, const Edge& edge)
{
    return os << edge.uid_ << "," << (edge.is_explicit() ? "explicit" : "implicit") << "," << osm::highway_name_map[edge.way_type_] << "," << *(edge.v1) << "," << *(edge.v2);
}

std::ostream& operator<< ( std::ostream& os, const Area& area )
{
    os << "[";
    for ( auto& c : area.corners_ ) { 
        os << c << ", ";
    }
    os << "]";
    return os;
}

std::ostream& operator<< (std::ostream& os, const Circle& circle)
{
    return os << circle.lat << ", " << circle.lon << ", " << circle.radius;
}

}  // end namespace Geo

namespace std {

size_t hash<geo::Point>::operator()( const geo::Point& pt ) const
{
    size_t r = ( hash<double>()( pt.lat ) ^ hash<double>()( pt.lon ) >> 7 );
    return r;
}

size_t hash<geo::Location>::operator()( const geo::Location& loc ) const
{
    size_t r =hash<geo::Point>()( loc ) ^ hash<uint64_t>()( loc.uid ) >> 7;
    return r;
}

size_t hash<geo::EdgePtr>::operator()( const geo::EdgePtr& eptr ) const
{
    size_t r = (hash<geo::Location>()( *(eptr->v1) ) ^ hash<geo::Location>()( *(eptr->v2) ) >> 7 );
    r = r ^ hash<uint64_t>()( eptr->uid_ ) << 7;
    return r;
}

}  // end namespace std

