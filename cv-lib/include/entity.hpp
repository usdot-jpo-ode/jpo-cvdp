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
 * Oak Ridge National Laboratory, UT Battelle, Center for Trustworthy Embedded Systems
 */

#ifndef CVDP_DI_ENTITY_HPP
#define CVDP_DI_ENTITY_HPP

#include <iostream>
#include <memory>
#include <limits>

#include "osm.hpp"

namespace geo {
struct Point;
class Area;
class Location;
class Edge;
class Bounds;
class Circle;
class Grid;
}

/**
 * TODO: breaks with Google style guide, but makes things easier.
 */
namespace std {

template<> struct hash<geo::Point>
{
    size_t operator()( const geo::Point& pt ) const;
};

template<> struct hash<geo::Location>
{
    size_t operator()( const geo::Location& loc ) const;
};

template<> struct hash<std::shared_ptr<geo::Edge>>
{
    size_t operator()( const std::shared_ptr<geo::Edge>& eptr ) const;
};

}

/**
 * @brief Namespace for geographic elements and structures.
 */
namespace geo {

const double kPi = 3.14159265358979323846;          ///< Pi to 20 digits.
const double kEarthRadiusM = 6378137.0;             ///< The Earth's radius in meters.
const double kGPSEpsilon = std::numeric_limits<double>::epsilon()*100;

/**
 * @brief Convert radians to degrees.
 *
 * @param double radians Degrees in as radians (0, 2 * #kPi).
 * @return The decimal degrees.
 */
inline double to_degrees(double radians) { return radians * 180.0 / kPi; }

/**
 * @brief Convert degrees to radians.
 *
 * @param double degrees Degrees as a decimal value (0, 360).
 * @return The decimal degrees.
 */
inline double to_radians(double degrees) { return degrees * kPi / 180.0; }

/**
 * @brief A point is a 2D GPS coordinate. It stores the latitude and longitude as decimal values, e.g (90.0, 180.0)
 */
struct Point {
    double lat;                     ///< latitude in decimal degrees.
    double lon;                     ///< longitude in decimal degrees.

    /**
     * @brief Default point constructor with 0.0 assigned to each coordinate.
     */
    Point();

    /**
     * @brief Construct a new point.
     * 
     * @param double latitude The decimal latitude value.
     * @param double longitude The decimal longitude value.
     */
    Point(double latitude, double longitude);

    /**
     * @brief Construct a new point from another point.
     * 
     * @param const Point& point The other point.
     */
    Point(const Point& pt);

    /**
     * @brief Compare this point with another point. Two points are equal if 
     * respective coordinates are equivalent.
     * 
     * @note This does floating point comparison and could be inconsistent
     *       accross platforms.
     *
     * @param const Point& other The other point.
     * @return bool True if this point is equal to the other point, 
     *              otherwise False.
     */
    bool operator==(const Point& other) const;
    
    /**
     * @brief Return a hash value for a point.
     *
     * @param const Point& pt The point to hash.
     * @return size_t The hash value of the point.
     */
    friend std::size_t std::hash<Point>::operator()(const Point& pt) const;

    /**
     * @brief Write a point to a stream.
     * 
     * @param std::ostream& os The stream object where the point will be 
     *                         written.
     * @param const Point& pt The point to write.
     * @return std::ostream& Returns the given stream object.
     */
    friend std::ostream& operator<<(std::ostream& os, const Point& pt);
};

/**
 * @brief Interface for entities which can be partially contained within other
 * entities. Entity is the base class for all shapes, points, lines, etc.
 */
class Entity {
    public:
        using Ptr = std::shared_ptr<Entity>;                ///< Shared pointer to an entity.
        using CPtr = std::shared_ptr<const Entity>;         ///< Constant shared pointer to entity.
        using PtrList = std::vector<CPtr>;            ///< Set of constant shared pointers to entities.
        using PtrSet = std::unordered_set<CPtr>;            ///< Set of constant shared pointers to entities.

        /**
         * @brief Get a string identifier for this entity.
         * 
         * @return std::string The type of this entity.
         */ 
        virtual const std::string get_type(void) const = 0;

        /**
         * @brief Determine is this entity is within the bounds object.
         * 
         * @param const Bound& bounds Bounds object to test against.
         * @return bool True if this enitity is within the bounds, otherwise
         *              false.      
         */ 
        virtual bool touches(const Bounds& bounds) const = 0;
};

/**
 * @brief A Location is a #Point that can be constructed with a unique identifier.
 * Locations provide more useful geospatial funtions than Points.
 */
class Location : public Point, public Entity {
    public:
        using Ptr = std::shared_ptr<Location>;              ///< Shared pointer to a location instance.
        using CPtr = std::shared_ptr<const Location>;

        /** 
         * @brief Compute the spherical distance, in meters, between two locations.
         * 
         * @param const Location& location_a The first location. 
         * @param const Location& location_b The second location. 
         * @return double The spherical distance, in meters, between the two
         *                locations.
         *
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        static double distance(const Location& location_a, const Location& location_b);

        /** 
         * @brief Compute the spherical distance, in meters, between two locations.
         * 
         * @param double latitude_a The decimal latitude of the first 
         *                          location.
         * @param double longitude_a The decimal longitude of the first 
         *                           location.
         * @param double latitude_b The decimal latitude of the second 
         *                          location.
         * @param double longitude_b The decimal longitude of the second 
         *                           location.
         * @return double The spherical distance, in meters, between the two
         *                locations.
         *
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        static double distance(double latitude_a, double longitude_a, double latitude_b, double longitude_b);

        /**
         * @brief Compute the Haversine distance, in meters, between two locations.
         *
         * @param const Location& location_a The first location. 
         * @param const Location& location_b The second location. 
         * @return double The Haversine distance, in meters, between the two
         *                locations.
         *
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        static double distance_haversine(const Location& location_a, const Location& location_b);

        /** 
         * @brief Compute the Haversine distance, in meters, between two locations.
         * 
         * @param double latitude_a The decimal latitude of the first location.
         * @param double longitude_a The decimal longitude of the first location.
         * @param double latitude_b The decimal latitude of the second location.
         * @param double longitude_b The decimal longitude of the second location.
         * @return double The Haversine distance, in meters, between the two
         *                locaitons.
         *
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        static double distance_haversine(double latitude_a, double longitude_a, double latitude_b, double longitude_b);

        /**
         * @brief Project a location along a bearing for a distance, in meters. 
         * 
         * @param const Location& location The location to project.
         * @param double bearing The bearing, in decimal degrees, to 
         *                       project along.
         * @param double distance The distance, in meters, to project.
         * @return Location The projected location.
         * 
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        static Location project_position(const Location& location, double bearing, double distance);

        /**
         * @brief Project a location along a bearing for a distance, in meters. 
         * 
         * @param double latitude The latitude, in decimal degrees, of 
         *                        the location to project.
         * @param double longitude The longitude, in decimal degrees, of 
         *                         the location to project.
         * @param double bearing The bearing, in decimal degrees, to 
         *                       project along.
         * @param double distance The distance, in meters, to project.
         * @return Location The projected location.
         * 
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        static Location project_position(double latitude, double longitude, double bearing, double distance);

        /**
         * @brief Compute the midpoint between two locations.
         *
         * @param const Location& location_a The first location.
         * @param const Location& location_b The second location.
         * @return Location The midpoint between the two locations.
         * 
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        static Location midpoint(const Location& location_a, const Location& location_b);

        /**
         * @brief Compute the midpoint between two locations.
         *
         * @param double latitude_a The latitude, in decimal degrees, of the 
         *                          first location.
         * @param double longitude_a The longitude, in decimal degrees, of
         *                           the first location.
         * @param double latitude_b The latitude, in decimal degrees, of the 
         *                          second location.
         * @param double longitude_b The longitude, in decimal degrees, of
         *                           the second location.
         * @return Location The midpoint between the two locations.
         * 
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        static Location midpoint(double latitude_a, double longitude_a, double latitude_b, double longitude_b);

        /**
         * @brief Compute the bearing between two locations, in decimal degrees.
         *
         * @param const Location& location_a The first location.
         * @param const Location& location_b The second location.
         * @return double The bearing between the two locations, in
         *                decimal degrees.
         * 
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        static double bearing( const Location& location_a, const Location& location_b );

        /**
         * @brief Compute the bearing between two locations, in decimal degrees.
         *
         * @param double latitude_a The latitude, in decimal degrees, of the 
         *                          first location.
         * @param double longitude_a The longitude, in decimal degrees, of
         *                           the first location.
         * @param double latitude_b The latitude, in decimal degrees, of the 
         *                          second location.
         * @param double longitude_b The longitude, in decimal degrees, of
         *                           the second location.
         * @return double The bearing between the two locations, in
         *                decimal degrees.
         * 
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        static double bearing( double latitude_a, double longitude_a, double latitude_b, double longitude_b );

        uint64_t uid;                                               ///< The unique ID of the location.
        double latr;                                                ///< The latitude in radians of the location.
        double lonr;                                                ///< The longitude in radians of the location.

        /**
         * @brief Construct a new location with a unique ID of 0.
         * 
         * @param double latitude The latitude of the location.
         * @param double longitude The longitude of the location.
         */ 
        Location(double latitude, double longitude);

        /**
         * @brief Construct a new location.
         * 
         * @param double latitude The latitude of the location.
         * @param double longitude The longitude of the location.
         * @param uint64_t uid The unique ID of the location.
         */ 
        Location(double latitude, double longitude, uint64_t uid);

        /**
         * @brief Get a string identifier for this enitity.
         * 
         * @return std::string The type of this enitity.
         */ 
        const std::string get_type(void) const;

        /**
         * @brief Determine is this location is within the bounds object.
         * 
         * A location touches a bound if the point is contained within the
         * bounds or on any of the bounds' edges.
         * 
         * @param const Bound& bounds Bounds object to test against.
         * @return bool True if this location is within the bounds,
         *              otherwise False.      
         */ 
        bool touches(const Bounds& bounds) const;

        /**
         * @brief Compute the spherical distance, in meters, between this location
         * and another location.
         * 
         * @param const Location& location The other location.
         * @return double The spherical distance, in meters, between this 
         *                location and the other location.
         *
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        double distance_to(const Location& location) const;

        /**
         * @brief Compute the Haversine distance, in meters, between this location
         * and another location.
         * 
         * @param const Location& location The other location.
         * @return double The Haversine distance, in meters, between this 
         *                location and the other location.
         *
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        double distance_to_haversine(const Location& location) const;

        /**
         * @brief Compute the midpoint between this location and antoher location.
         *
         * @param const Location& location The other location.
         * @return Location The midpoint between this location and the other
         *                  location.
         * 
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        Location midpoint(const Location& location);
        
        /**
         * @brief Project this location along a bearing for a distance, in meters. 
         * 
         * @param double bearing The bearing, in decimal degrees, to 
         *                       project along.
         * @param double distance The distance, in meters, to project.
         * @return Location The projected location.
         * 
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        Location project_position(double bearing, double distance) const;

        /**
         * @brief Compute the bearing between this location and another location,
         *  in decimal degrees.
         *
         * @param const Location& location The other location.
         * @return double The bearing between this location and another
         *                location, in decimal degrees.
         * 
         * @see http://www.movable-type.co.uk/scripts/latlong.html
         */
        double bearing_to(const Location& location);

        /**
         * @brief Compare this location with another location. Two location are 
         * equal if they contain the same latitude and longitude.
         * 
         * @note This does floating point comparison and could be 
         *       inconsistent accross platforms.
         *
         * @param const Location& other The other location.
         * @return bool True if this location is equal to the other 
         *              location, otherwise False.
         */
        bool operator==(const Location& other) const;

        /**
         * @brief Return a hash value for a location.
         *
         * @param const Location& location The location to hash.
         * @return size_t The hash value of the location.
         */
        friend std::size_t std::hash<Location>::operator()(const Location& location) const;

        /**
         * @brief Write a location to a stream.
         * 
         * @param std::ostream& os The stream object where the location will 
         *                         be written. 
         * @param const Point& pt The point to write.
         * @return std::ostream& Returns the given stream object.
         */
        friend std::ostream& operator<< (std::ostream& os, const Location& location);
};

//! Pointer to edge.
using EdgePtr = std::shared_ptr<Edge>;

//! Pointer to constant edge.
using EdgeCPtr = std::shared_ptr<const Edge>;

//! Set of edge of pointers.
using EdgePtrSet = std::unordered_set<EdgePtr>;

//! Set of constant edge of pointers.
using EdgeCPtrSet = std::unordered_set<EdgeCPtr>;

//! Edge pointer vector.
using EdgePtrList = std::vector<EdgePtr>;

/**
 * @brief A vertex is a location with a set of incident edges.
 * 
 * Vertices may be used to find the degree or out-degree of a node in 
 * in  road network.
 */
class Vertex : public Location {
    public:
        
        using Ptr = std::shared_ptr<Vertex>;                        ///< Pointer to a vertex.
        using IdToPtrMap = std::unordered_map<uint64_t, Ptr>;       ///< Map of intergers to vertext pointers.

        /**
         * Construct a new vertex with a unique ID of 0.
         * 
         * @param double latitude The latitude of the vertex.
         * @param double longitude The longitude of the vertex.
         */ 
        Vertex(double latitude, double longitude);

        /**
         * Construct a new vertex.
         * 
         * @param double latitude The latitude of the vertex.
         * @param double longitude The longitude of the vertex.
         * @param uint64_t uid The unique ID of the vertex.
         */ 
        Vertex(double latitude, double longitude, uint64_t uid);

        /**
         * Construct a new vertex.
         * 
         * @param const Location& location The location of the vertex. Uses
         *                                 the UID of the location as well.
         */ 
        Vertex(const Location& location);

        /**
         * Add an incident edge to this vertex. 
         * 
         * @param EdgePtr edge_ptr Pointer to an edge.
         * 
         * @return bool True if this edge is not already an incident edge of
         *              this vertex, otherwise False.
         */
        bool add_edge(EdgePtr edge_ptr);

        /** 
         * Add a set of incident edges to this vertex.
         * 
         * @param EdgePtrSet& edges The set of edges to add.
         *
         * @return bool True if any edge in the is not already an edge of 
         *              this vertex, otehrwise False.
         */
        bool add_edges(EdgePtrSet& edges);

        /**
         * Get the degree of this vertex.
         *  
         * The degree is the number of unique incident edges connected to 
         * this edge.
         *
         * @return uint32_t The degree of this vertex.
         */ 
        uint32_t degree() const;

        /**
         * Get the out-degree of this vertex.
         *  
         * The out-degree is the degree - 1 if the degree is greater than
         * 2. Othewise the out-degree is 0.
         *
         * @return uint32_t The out-degree of this vertex.
         * @see degree()
         */ 
        uint32_t outdegree() const;

        /**
         * Update the location of this vertex.
         *
         * Copy the geo-location and UID information of the given location
         * the corresponding fields of this vertex. The underlying incident
         * edges remain unchanged.
         * 
         * @param const Location& location The location to upate this vertex
         *                                 to.
         */
        void update_location(const Location& location);

        /**
         * Check if the location of this vertiex is the same as the given point.
         * 
         * Used mainly to compare edges.
         * 
         * @param p The Point object to check against.
         * @return True if the locations are the same, False otherise.
         */
        bool is_same_point(const Point& p);

        /**
         * Get the incident edge set of this vertex.
         * 
         * @return const EdgePtrSet& The incident edge set of this
         *                           vertex.
         */
        const EdgePtrSet& get_incident_edges() const;

        friend std::ostream& operator<< (std::ostream& os, const Vertex& vertex);

    private:
        EdgePtrSet edges_;                       ///< The incident edges of the vertex.
};

/**
 * @brief
 *
 */
class ZeroAreaException: public std::exception
{
    /**
     * @brief return a message describing the ZeroAreaException.
     *
     * @return the exception message as a constant char pointer.
     */
    const char* what() const throw()
    {
        return "Edge based area is zero.";
    }
};

//! Pointer to an area.
using AreaPtr = std::shared_ptr<Area>;

//! Pointer to a constant area.
using AreaCPtr = std::shared_ptr<const Area>;

/**
 * @brief An map edge consists of two Locations, possibly a UID, and an OSM type.
 *
 * An Edge can be explicit (directly defined by an OSM map segment), or implicit 
 * (indirectly defined based on the travel direction of a vehicle). A Edge is the 
 * only thing that gets inserted into a Quad.
 */
class Edge : public Entity {
    public:
        Vertex::Ptr v1;                 ///< A pointer to the edge's first vertex.
        Vertex::Ptr v2;                 ///< A pointer to the edge's second vertex.

        /**
         * @brief Construct an edge without an id from two vertices.
         *
         * @param v1 the first vertex of the edge.
         * @param v2 the second vertext of the edge.
         * @param explicit_edge default value, true, makes this edge and explicit edge; otherwise an implicit edge.
         */
        Edge( const Vertex& v1, const Vertex& v2, bool explicit_edge = true );

        /**
         * @brief Construct an edge with an id from two vertices.
         *
         * @param v1 the first vertex of the edge.
         * @param v2 the second vertext of the edge.
         * @param id a unique identifier for this edge; typically this is an OSM
         * identifier.
         * @param explicit_edge default value, true, makes this edge and explicit edge; otherwise an implicit edge.
         */
        Edge( const Vertex& v1, const Vertex& v2, uint64_t id, bool explicit_edge = true );

        /**
         * @brief Construct an edge with an id from two vertices.
         *
         * @param v1 the first vertex of the edge.
         * @param v2 the second vertext of the edge.
         * @param id the default value is 0. a unique identifier for this edge; typically this is an OSM
         * identifier.
         * @param explicit_edge default value, true, makes this edge and explicit edge; otherwise an implicit edge.
         */
        Edge( const Vertex::Ptr& v1, const Vertex::Ptr& v2, uint64_t id = 0, bool explicit_edge = true );
        Edge( const Vertex::Ptr& v1, const Vertex::Ptr& v2, const osm::Highway type, uint64_t id, bool explicit_edge = true );

        /**
         * Return a string identifying the type of this edge: explict or
         * implicit.
         * 
         * @return std::string The type of this entity.
         */ 
        const std::string get_type(void) const;

        /**
         * Determine if this edge is within the bounds object.
         * 
         * A edge touches a bounds if either end point is within the bounds
         * or it intersects any edge of the bounds.
         * 
         * @param const Bound& bounds Bounds object to test against.
         * @return bool True if this edge touches the bounds, otherwise
         *              False.      
         */ 
        bool touches(const Bounds& bounds) const;

        /**
         * @brief Compute and return the distance between this edge and a
         * location (point).
         *
         * @param loc the location from which to measure the distance.
         * @return the distance between the point and this edge in meters.
         */
        double distance_from_point(const Location& loc) const;

        /**
         * @brief Return the OSM highway type associated with this edge/way.
         *
         * @return an instance of the enumeration used to describe OSM way
         * types.
         */
        osm::Highway get_way_type() const;

        /**
         * @brief Return the OSM way type as an integer.
         * 
         * @return the OSM way type as an integer.
         */
        int get_way_type_index() const;

        /**
         * @brief Return the width of this edge in meters.
         *
         * Widths are associated with explict OSM ways.
         *
         * @return the width of this OSM segment in meters.
         */
        double get_way_width() const;

        /**
         * @brief Return the difference in latitudes between the second vertex
         * and the first vertex.
         *
         * @return the latitude difference; this could be negative.
         */
        double dlatitude() const;

        /**
         * @brief Return the difference in longitudes between the second vertex
         * and the first vertex.
         *
         * @return the latitude difference; this could be negative.
         */
        double dlongitude() const;

        /**
         * @brief Compute and return an approximated distance between the two
         * vertices that define this edge.
         *
         * NOTE: This is not haversine distance, but the faster approximation.
         *
         * @return the length of this edge in meters.
         */
        double length() const;

        /**
         * @brief Compute and return the length of this edge using the slower
         * haversine formula.
         *
         * @return the length of this edge in meters.
         */
        double length_haversine() const;

        /**
         * @brief Compute the great circule bearing between the two vertices
         * that define this edge.
         *
         * @return the bearing in degrees.
         */
        double bearing() const;

        /**
         * @brief Predicate that indicates whether this edge and the edge
         * described by the two points provided intersect.
         *
         * @return true if the two lines intersect; false otherwise.
         */
        bool intersects( double lat1, double lon1, double lat2, double lon2 ) const;

        /**
         * @brief Predicate that indicates whether this edge and the edge
         * provided intersect.
         *
         * @return true if the two lines intersect; false otherwise.
         */
        bool intersects( const Edge& edge ) const;

        /**
         * @brief A predicate indicating whether this edge is an explicit edge.
         *
         * An explicit edge is defined from a map element.
         *
         * @return true if this edge is explict; false otherwise.
         */
        bool is_explicit() const;

        /**
         * @brief A predicate indicating whether this edge is an implicit edge.
         *
         * An implicit edge is defined after examining trip behavior "off" an
         * explicit edge.
         *
         * @return true if this edge is implicit; false otherwise.
         */
        bool is_implicit() const;

        /**
         * @brief Return the unique identifier for this edge.
         *
         * @return a 64-bit integer that uniquely identifies this edge.
         */
        uint64_t get_uid() const;

        /**
         * @brief Construct an Area from this edge using this edges predefined width 
         * from the OSM information.
         *
         * @return a shared pointer to the area that encapsulates this edge.
         * @throws ZeroAreaException when there area characterizes 0 space.
         */
        AreaPtr to_area( ) const;

        /**
         * @brief Return an area that encapsulates this edge and whose width is
         * based on this edge's way type; the area will be oriented in the direction of 
         * the edge.
         *
         * @param extension the meters to extend the area from each end of the edge.
         * @return a shared pointer to the area that encapsulates this edge.
         * @throws ZeroAreaException when there area characterizes 0 space.
         */
        AreaPtr to_area( double extension ) const;

        /**
         * @brief Take an edge from the map and make an area; this could be oriented in any
         * direction.
         *
         * @param capwidth the total width of the area in meters.
         * @param extension the meters to extend the area from each end of the
         * edge.
         * @return a shared pointer to the area that encapsulates this edge.
         * @throws ZeroAreaException when there area characterizes 0 space.
         */
        AreaPtr to_area( double capwidth, double extension ) const;

        /**
         * @brief Operator that evaluates whether two edges are equivalent based ONLY
         * on their vertex coordinates.
         *
         * This method checks both orientations.
         *
         * NOTE: This method does NOT check the unique identifier.
         *
         * @return true if the edges are equivalent in space; false otherwise.
         */
        bool operator==( const Edge& other ) const;

        /**
         * @brief Return the hash code for this edge (referenced by a shared
         * poitner); this is a form of unique identifier.
         *
         * @return this edge's hash code (the underlying edge not the pointer).
         */
        friend std::size_t std::hash<std::shared_ptr<Edge>>::operator()( const std::shared_ptr<Edge>& eptr ) const;

        /**
         * @brief Write a readable form of this edge to the provided stream.
         *
         * @param os the stream to write the edge to.
         * @param edge the edge to convert into readable form and output to the
         * stream.
         *
         * @return the stream.
         */
        friend std::ostream& operator<< (std::ostream& os, const Edge& edge);

    private:
        uint64_t uid_;                       ///< This edge's unique identifier.
        osm::Highway way_type_;              ///< This edge's OSM way type.
        bool explicit_edge_;                 ///< Indicates how this edge was constructed: from an OSM segment or inferred from the trip.
};

/**
 * @brief A recatangular container that can be oriented (angled) in any way; 
 * This is different from a Bound instance that is always horizontal.
 *
 * NOTE: Making this more compact. Use just the four corner points instead 
 * of lines. We can still use the relationship function because it only 
 * uses coordinates.
 *
 *                edge[1]
 *        (c1) +----->-----+ (c2)
 *             |           |
 *             |           |
 *    edge[0]  ^           | edge[2]
 *             |           |
 *             |           |
 *        (c0) +-----<-----+ (c3)
 *                edge[3]
 *   
 */
class Area : public Entity {
    private:
        std::vector<Point> corners_;                 ///< The corner points that describe this area; order starts in upper left corner and procedes clockwise.

    public:
        using Ptr = std::shared_ptr<Area>;          ///< Shared pointer to an Area.

        /**
         * @brief Construct an area from four points.
         *
         * @param p1 upper left corner.
         * @param p2 upper right corner.
         * @param p3 lower right corner.
         * @param p4 lower left corner.
         */
        Area( const Point& p1, const Point& p2, const Point& p3, const Point& p4 );

        /**
         * @brief Construct an area from four points (RValues)
         *
         * @param p1 upper left corner.
         * @param p2 upper right corner.
         * @param p3 lower right corner.
         * @param p4 lower left corner.
         */
        Area( const Point&& p1, const Point&& p2, const Point&& p3, const Point&& p4 );

        /**
         * Get a string that identifies the type of this entity.
         * 
         * @return std::string The type of this enitity.
         */ 
        const std::string get_type(void) const;

        /**
         * @brief Predicate that indicates whether this area is contained within the 
         * provided bounds or intersects spatially the bounds.
         * 
         * A location touches a bound if the point is contained within the
         * bounds or on any of the bounds' edges.
         * 
         * @param const Bound& bounds Bounds object to test against.
         * @return bool True if this location is within the bounds,
         *              otherwise False.      
         */ 
        bool touches(const Bounds& bounds) const;

        /**
         * @brief Predicate that indicates whether this Area contains the
         * provided point.
         *
         * TODO: There is a better algorithm to use for containment and then we
         * would not be restricted to rectangles.
         *
         * @param loc the location whose containment is checked.
         * @return true if the point is within the bounds of this area; false
         * otherwise.
         */
        bool contains( const Point& loc ) const;

        /**
         * @brief Predicate that indicates whether the point is outside (to the
         * left of) the edge designated by the integer parameter.
         *
         * Area edges are 0-indexed starting with the edge running from the
         * lower left corner to the upper left corner and proceding clockwise.
         *
         * TODO: There is a better algorithm to use for containment.
         *
         * @param edge see above.
         * @param loc the point whose position is being checked.
         * @return true if the point is to the left of the edge designated by
         * edge.
         */
        bool outside_edge( int edge, const Point& loc ) const;

        /**
         * @brief Return a constant reference to the list of corners that
         * describe this area.
         *
         * @return a constant reference to the list of corner points.
         */
        const std::vector<Point>& get_corners() const;

        /**
         * @brief Return a string formatted for a KML Polygon structure that
         * describes this area.
         *
         * @return a constant string to be used in KML.
         *
         */
        const std::string get_poly_string() const;

        /**
         * @brief Write a human-readable form of this Area to the provided
         * stream.
         *
         * @param os The stream to write the string.
         * @param area The area whose form will be written.
         * @return The output stream after it has been written to.
         */
        friend std::ostream& operator<< ( std::ostream& os, const Area& area );
};

/**
 * @brief A circle is a 2D GPS coordinate and a radius measured in meters. The
 * circle is also described by its northernmost, southernmost, easternmost,
 * and westernmost points; these are used to determine whether the circle is
 * contained within a bounds.
 *
 * Circle instances are used to describe regions that we use to isolate certain trip
 * points. Circles are considered locations since they are located based on a
 * specific point.
 */
class Circle : public Location {
    public:
        using Ptr = std::shared_ptr<Circle>;
        using CPtr = std::shared_ptr<const Circle>; 

        double radius;                              ///< The radius of the circle in meters.
        Location north;                             ///< The northernmost point on this circle.
        Location south;                             ///< The southernmost point on this circle.
        Location east;                              ///< The easternmost point on this circle.
        Location west;                              ///< The westernmost point on this circle.

        /**
         * Construct a new circle.
         * 
         * @param const Location& location The center of the circle.
         * @param double radius The radius, in meteres, of the circle.
         */
        Circle(const Location& location, double radius);

        /**
         * Construct a new circle.
         * 
         * @param double latitude The latitude of the center of the circle.
         * @param double longitude The longitude of the center of the 
         *                         circle.
         * @param double radius The radius, in meteres, of the circle.
         */
        Circle(double latitude, double longitude, double radius);

        /**
         * Construct a new circle with a unique ID.
         * 
         * @param double latitude The latitude of the center of the circle.
         * @param double longitude The longitude of the center of the 
         *                         circle.
         * @param uint64_t uid The unique ID of the circle.
         * @param double radius The radius, in meteres, of the circle.
         */
        Circle(double latitude, double longitude, uint64_t uid, double radius);

        /**
         * Get a string that identifies this entity's type.
         * 
         * @return std::string The type of this enitity.
         */ 
        const std::string get_type(void) const;

        /**
         * Predicate indicating whether this circle is within the provided
         * bounds.
         * 
         * A circle touches a bounds if its center is within the bounds, 
         * or any of its four cardinal points (north, south, east, west) 
         * are in the bounds, or the bounds is strictly contained within 
         * the circle.
         * 
         * @param const Bound& bounds Bounds object to test against.
         * @return bool True if this circle touches the bounds, otherwise
         *              False.      
         */ 
        bool touches(const Bounds& bounds) const;

        /**
         * Determine if the given point is within this circle.
         * 
         * @param const Point& point The point to test.
         * @return bool True is the point is within the circle, False 
         *              otherwise.
         */ 
        bool contains(const Point& point) const;

        /**
         * Compare this circle with another circle. Two circles are 
         * equal if they contain they have the same point and radius.
         * 
         * @note This does floating point comparison and could be 
         *       inconsistent accross platforms.
         *
         * @param const Circle& other The other circle.
         * @return bool True if this circle is equal to the other 
         *              circle, otherwise False.
         */
        bool operator==(const Circle& other) const;

        /**
         * Write a circle to a stream.
         * 
         * @param std::ostream& os The stream object where the circle will 
         *                         be written. 
         * @param const Circle& circle The circle to write.
         * @return std::ostream& Returns the given stream object.
         */
        friend std::ostream& operator<< (std::ostream& os, const Circle& circle);
};

/** @brief A boundary is a horizontally/vertically oriented rectangle based on a
 * SW node lat-lon and a NE node lat-lon and the difference of lat and lon
 * between those nodes. In other words, it cannot be angled.
 */
class Bounds {

    public:
        using Ptr = std::shared_ptr<Bounds>;            ///< Shared pointer to a Bounds instance.

        Point nw;                                       ///< The location of the northwest corner of the bounds.
        Point ne;                                       ///< The location of the northeast corner of the bounds.
        Point se;                                       ///< The location of the southeast corner of the bounds.
        Point sw;                                       ///< The location of the southwest corner of the bounds.

        /**
         * @brief Construct an empty Bounds instance; no corners defined.
         */
        Bounds();

        /**
         * @brief Copy construct a bounds instance from another Bounds instance.
         *
         * @param bounds the bounds instance to copy.
         *
         */
        Bounds( const Bounds& bounds );

        /**
         * @brief Create a bounds instance using the southwest and northeast corners. The remaining corners of the
         * bounds can be defined from these two points.
         *
         * @param swloc The southwest corner of the bounds.
         * @param neloc The northeast corner of the bounds.
         */
        Bounds( const Point& swloc, const Point& neloc );

        /**
         * @brief A predicate indicating whether the point provided is contained geographically within this bounds.
         *
         * @param loc the location to check for inclusion within this bounds.
         * @return true if loc is within the bounds; false otherwise.
         */
        bool contains( const Point& loc ) const;

        /**
         * @brief A predicate indicating whether the edge provided is contained geographically within this bounds.
         *
         * @param e the edge to check against this bounds.
         * @return true if e is completely within the bounds; false otherwise.
         */
        bool contains( const Edge& e ) const;

        /**
         * @brief A predicate indicating whether the provided circle's north, south, east, and west point are within
         * the bounds. 
         *
         * @param c the circle to check against the bounds.
         * @return true if any portion of the circle is within the bounds; false otherwise.
         */
        bool contains( const Circle& c ) const;

        /**
         * @brief A predicate indicating whether any portion of the provided edge intersects at least one of the boundary lines. 
         *
         * Note: If the edge is completely within the bounds, this method returns false -- that is not an intersection.
         *
         * @param e the edge to check against the boundary lines.
         * @return true if e intersection one or more of this bound's boundary edges; false otherwise.
         */
        bool intersects( const Edge& e ) const;

        /**
         * @brief A predicate indicating whether any portion of the provided circle intersects at least one of the boundary lines.
         *
         * Note: If any of the circle's north, south, east, or west points create an edge that intersects the bounds.
         *
         * @param c the circle to check against the boundary.
         * @return true if c intersects one or more of this bound's boundary edges; false otherwise.
         */
        bool intersects( const Circle& c ) const;

        /**
         * @brief A predicate indicating whether one or more of the edge end points are inside of this bounds.
         *
         * @param edge the edge to check against the boundary lines.
         * @return true if e intersects or is contained within the boundary.
         */
        bool contains_or_intersects( const Edge& edge ) const;

        /**
         * @brief A predicate indicating whether one or more of the two points are inside of this bounds.
         *
         * @param pt_a one of the end points of the edge.
         * @param pt_b one of the end points of the edge.
         * @return true if the edge defined by pt_a and pt_b intersects or is contained within the boundary.
         */
        bool intersects(const Point& pt_a, const Point& pt_b) const;

        /**
         * @brief A predicate indicating whether some portion (could be all) of the circle is contained within the
         * bounds.
         *
         * @param circle the circle to check against the boundary lines.
         * @return true if circle is contained in or intersects the boundary.
         */
        bool contains_or_intersects( const Circle& circle ) const;

        /**
         * @brief Retrieve the midpoint of the western boundary of the bounds.
         *
         * @return the midpoint of the western boundary.
         */
        Point west_midpoint() const;

        /**
         * @brief Retrieve the midpoint of the eastern boundary of the bounds.
         *
         * @return the midpoint of the eastern boundary.
         */
        Point east_midpoint() const;

        /**
         * @brief Retrieve the midpoint of the northern boundary of the bounds.
         *
         * @return the midpoint of the northern boundary.
         */
        Point north_midpoint() const;

        /**
         * @brief Retrieve the midpoint of the southern boundary of the bounds.
         *
         * @return the midpoint of the southern boundary.
         */
        Point south_midpoint() const;

        /**
         * @brief Retrieve the center point of the boundary.
         *
         * @return the center point of the boundary.
         */
        Point center() const;

        /**
         * @brief Return the width of the Bounds in LONGITUDE units. This is the 
         * difference in the East longitude and West longitude. Returns a positive 
         * value independent of relationship with the Prime Meridian.
         *
         * @return the width of the Bounds in LONGITUDE degrees.
         */
        double width() const;

        /**
         * @brief Return the height of the Bounds in LATITUDE units.  This is the 
         * difference in the North latitude and South latitude. Returns a positive 
         * value independent of hemisphere.
         *
         * @return the height of the Bounds in LATITUDE degrees.
         */
        double height() const;

        /**
         * @brief Write a user-readable form of this Boundary.
         *
         * @param os the output stream to write the boundary to.
         * @param bounds the boundary instance to write in user-readable format.
         * @return the output stream after the bounds have been written.
         */
        friend std::ostream& operator<< (std::ostream& os, const Bounds& bounds);
};

/**
 * @brief A Grid divides up a region into disjoint grid squares.
 */
class Grid : public Bounds, public Entity {
    public:
        using Ptr = std::shared_ptr<Grid>;                  ///< A shared pointer to a Grid instance.
        using CPtr = std::shared_ptr<const Grid>;           ///< A shared pointer to a constant Grid instance.
        using GridPtrVector = std::vector<CPtr>;            ///< A list of Grid pointers.

        uint32_t row;                                       ///< The row index of this grid square.
        uint32_t col;                                       ///< The column index of this grid square.

        /**
         * @brief Create a Grid from a Bounds and a specific row and column.
         *
         * @param bounds The Bounds to use to define the geolocation of the Grid.
         * @param row The row within the larger Grid where this Grid Square is placed.
         * @param col The column within the larger Grid where this Grid Square is placed.
         */
        Grid(const Bounds& bounds, uint32_t row, uint32_t col);

        /**
         * @brief Create a Grid from the Southwest and Northeast corner points and a specific row and column.
         *
         * @param sw_loc The location of the Southwest corner of the Grid Square.
         * @param ne_loc The location of the Northeast corner of the Grid Square.
         * @param row The row within the larger Grid where this Grid Square is placed.
         * @param col The column within the larger Grid where this Grid Square is placed.
         */
        Grid(const Point& sw_loc, const Point& ne_loc, uint32_t row, uint32_t col);

        /**
         * @brief Get a string identifier for this entity type.
         * 
         * @return std::string The type of this entity.
         */ 
        const std::string get_type(void) const;

        /**
         * @brief Predicate indicating whether any of the corners of this grid are within
         * the provided bounds, the grid is contained in the bounds, or the bounds is contained in the grid.
         *
         * @param bounds Bounds object to test against.
         * @return bool True if this grid touches the bounds, otherwise
         *              False. 
         */ 
        bool touches(const Bounds& bounds) const;

        /**
         * @brief Build a collection of disjoint Grids of a particular size anchored at particular point.
         *
         * @param nw_point The anchor point for the grid.
         * @param grid_width The width of each individual grid in meters
         * @param lat_threshold The minimum latitude bounds of the collection of grids.
         * @param lon_threshold The maximum longitude bounds of the collection of grids.
         */
        static GridPtrVector build_grid(const Location& nw_point, double grid_width, double lat_threshold, double lon_threshold);

        /**
         * @brief Write a single grid to the provided stream in human-readable format.
         *
         * @param os the output stream.
         * @param grid the grid to write to the stream.
         * @return the stream after the grid has been written.
         */
        friend std::ostream& operator<< (std::ostream& os, const Grid& grid);
};

}

#endif
