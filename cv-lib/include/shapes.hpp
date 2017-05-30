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

#ifndef CVDP_SHAPES_HPP
#define CVDP_SHAPES_HPP

#include <memory>
#include "entity.hpp"

namespace shapes {

using StreamPtr = std::shared_ptr<std::istream>;        ///< Shared pointer to an input stream.
using StrVector = std::vector<std::string>;             ///< List of std::string instances.

static const int SHAPE_TYPE = 0;
static const int SHAPE_ID = 1;
static const int SHAPE_GEOGRAPHY = 2;
static const int SHAPE_ATTS = 3;

static const int POINT_ID = 0;
static const int POINT_LAT = 1;
static const int POINT_LON = 2;

/**
 * @brief Create and store a collection of shapes (Circles, Edges, and Grids) based on their definition in a file.
 *
 * A shaped file is a comma-delimited file having the following fields:
 * - type : the type of the shape, e.g., edge.
 * - id   : an unique 64-bit integer identifier for a shape.
 * - geography : a list of semicolon-delimited geographic coordinates. The sequence is latitude, longitude, latitude, ...
 * - attributes : a list of colon-delimited key value pairs, e.g., way_type=secondary.
 *
 * Geographies are specified in their respective make_<shape> methods.
 *
 * The order of the shapes in the file does not matter.
 */
class CSVInputFactory
{
    public:
        /**
         * @brief Construct a CSVInputFactor for testing -- this does not consume a file.
         */
        CSVInputFactory();

        /**
         * @brief Construct a Shape Factor give a file specification.
         *
         * @param file_path the file, including path, that contains the shape specifications.
         */
        CSVInputFactory(const std::string& file_path);

        /** @brief Open the shape specification file, create the shapes, and close the file.
         *
         * Shapes will be stored in the respective containers. If a shape specification is incorrect it will be skipped and a message 
         * will be displayed on std::cerr.
         *
         * @throws invalid_argument when the file could not be opened or the file is malformed, e.g., no header.
         */
        void make_shapes(void);

        /**
         * @brief Return an immutable vector of the Circle shapes specified in the file.
         *
         * Note: The file must contain circles and the make_shapes method must have been called.
         *
         * @return an immutable vector containing pointer to circle instances.
         */
        const std::vector<geo::Circle::CPtr>& get_circles(void) const;

        /**
         * @brief Return an immutable vector of the Edge shapes specified in the file.
         *
         * Note: The file must contain edges and the make_shapes method must have been called.
         *
         * @return an immutable vector containing pointer to edge instances.
         */
        const std::vector<geo::EdgeCPtr>& get_edges(void) const;

        /**
         * @brief Return an immutable vector of the Grid shapes specified in the file.
         *
         * Note: The file must contain Grid and the make_shapes method must have been called.
         *
         * @return an immutable vector containing pointer to Grid instances.
         */
        const std::vector<geo::Grid::CPtr>& get_grids(void) const;


        /**
         * @brief Attempt to construct a Circle instance from the parts provided
         * and if successful add the Circle to the container.
         *
         * Circle Specification:
         * - line_parts[0] : "circle"
         * - line_parts[1] : unique 64-bit integer identifier
         * - line_parts[2] : A sequence of colon-split elements that define the center.
         *      - Center: <lat>:<lon>:<radius in meters>
         *
         * @param line_parts A vector of strings where each string is a part of
         * a shape specification.
         * @throws out_of_range exception for incorrect lat/lon center point or
         * radius.
         */
        void make_circle(const StrVector& line_parts); 

        /**
         * @brief Attempt to construct and Edge instance from the parts provided
         * and if successful add the Edge to the container.
         *
         * Edge Specification:
         * - line_parts[0] : "edge"
         * - line_parts[1] : unique 64-bit integer identifier
         * - line_parts[2] : A sequence of colon-split points; each point is semi-colon split.
         *      - Point: <uid>;latitude;longitude
         * - line_parts[3] : A sequence of colon-split key=value attributes.
         *      - Attribute Pair: <attribute>=<value>
         *
         * @param line_parts A vector of strings where each string is a part of
         * a shape specification.
         * @throws out_of_range exception for incorrect positions.
         */
        void make_edge(const StrVector& line_parts); 

        /**
         * @brief Attempt to construct a Grid instance from the parts provided
         * and if successful add the Grid to the container.
         *
         * Grid Specification:
         * - line_parts[0] : "grid"
         * - line_parts[1] : A '_' split row-column pair.
         * - line_parts[2] : A sequence of colon-split elements defining the grid position.
         *      - Point: <sw lat>:<sw lon>:<ne lat>:<ne lon>
         *
         * @param line_parts A vector of strings where each string is a part of a shape specification.
         * @throws out_of_range exception for incorrect positions.
         */
        void make_grid(const StrVector& line_parts);

    private:

        std::string file_path_;                                 ///< The file containing the shape specifications.
        geo::Vertex::IdToPtrMap vertex_map_;                      ///< Map from identifiers to pointers to previously constructed vertices; prevents duplicates seen in OSM.
        std::vector<geo::Circle::CPtr> circles_;                ///< Vector of constant pointers to Circle instances.
        std::vector<geo::EdgeCPtr> edges_;                      ///< Vector of constant pointers to Edge instances.
        std::vector<geo::Grid::CPtr> grids_;                    ///< Vector of constant pointers to Grid instances.
};

/**
 * @brief Write a collection of shapes (Circles, Edges, and Grids) based on their data structure elements.
 *
 * See #CSVInputFactor for the file specification.
 */
class CSVOutputFactory
{
    public:
        /**
         * @brief Construct an Shape output instance.
         *
         * @param file_path the file name, including path, to use to write the * shape file.
         */
        CSVOutputFactory(const std::string& file_path);

        /**
         * @brief Add a Circle shape (pointer) to the collection to eventually
         * write.
         *
         * @param circle_ptr a shared pointer to a constant Circle instance.
         */
        void add_circle(geo::Circle::CPtr circle_ptr);

        /**
         * @brief Add a Edge shape (pointer) to the collection to eventually
         * write.
         *
         * @param edge_ptr a shared pointer to a constant Edge instance.
         */
        void add_edge(geo::EdgeCPtr edge_ptr);

        /**
         * @brief Add a Grid shape (pointer) to the collection to eventually
         * write.
         *
         * @param grid_ptr a shared pointer to a constant Grid instance.
         */
        void add_grid(geo::Grid::CPtr grid_ptr);

        /**
         * @brief Write a shape file containing the shapes previously added to
         * the collections maintained by this instance of the #CSVOutputFactory.
         *
         * @throws invalid_argument when the file specified when constructed
         * could not be used/opened.
         */
        void write_shapes(void) const;

        /**
         * @brief Write a single Circle to the specified stream.
         *
         * @param os The output stream to write the Circle specification to.
         * @param circle_ptr A shared pointer to the Circle instance.
         */
        void write_circle(std::ofstream& os, geo::Circle::CPtr circle_ptr) const; 

        /**
         * @brief Write a single Edge to the specified stream.
         *
         * @param os The output stream to write the Edge specification to.
         * @param edge_ptr A shared pointer to the Edge instance.
         */
        void write_edge(std::ofstream& os, geo::EdgeCPtr edge_ptr) const; 

        /**
         * @brief Write a single Grid to the specified stream.
         *
         * @param os The output stream to write the Grid specification to.
         * @param edge_ptr A shared pointer to the Grid instance.
         */
        void write_grid(std::ofstream& os, geo::Grid::CPtr grid_ptr) const;

    private:
        std::string file_path_;                         ///< The file to write the shape specification to.
        std::vector<geo::Circle::CPtr> circles_;        ///< The collection of Circle instances to write.
        std::vector<geo::EdgeCPtr> edges_;              ///< The collection of Edge instances to write.
        std::vector<geo::Grid::CPtr> grids_;            ///< The collection of Grid instance to write.
};

}  // end namespace Shapes

#endif
