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

#ifndef CVDP_DI_QUAD_HPP
#define CVDP_DI_QUAD_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <memory>

#include "names.hpp"
#include "entity.hpp"
#include "osm.hpp"

/**
 * @brief A Quad instance is a special tree. Instances are geographically defined and divided into four children. Each
 * Quad is a container. Leaf quads, those with no children, contain entities, e.g., Edges. This Quad implementation uses
 * two boundary definitions. One is a fuzzy boundary that is used to determine which entities are contained within a
 * quad. The actual boundary is used to retrieve entities. A Quad tree is a more efficient data structure to use for
 * searching through a geographical space since search is logarithmic in the number of levels.  The entities within a
 * leaf Quad must still be searched linearly.
 */
class Quad : public geo::Bounds {
    public:
        using Point  = geo::Point;
        using Edge   = geo::Edge;
        using Bounds = geo::Bounds;
        using Entity = geo::Entity;

        using Ptr = std::shared_ptr<Quad>;
        using CPtr = std::shared_ptr<const Quad>;
        using PtrList = std::vector<Ptr>;
        using PtrStack = std::stack<Ptr>;
        using EntityPtrStack = std::stack<Entity::CPtr>;
        using PtrSet = std::unordered_set<Ptr>;

        constexpr static double REDUCTION_FACTOR = 10.0;            ///< When the fuzzy dimensions are not set (i.e., 0), they will be set to the width of the quad divided by this factor.

        //! Maximum number of elements allowed in a quad node. If more elements
        // are added the quad will be split.
        constexpr static uint32_t MAX_ELEMENTS = 32;
        //! Minimum degree width/height for a quad.
        constexpr static double MIN_DEGREES = 0.003;

        constexpr static int BUFFER_SIZE = 8 * 1024;                ///< The input stream buffer size when generating a Quad tree from a file.
        /**
         * @brief Attempt to insert an Entity into the Quad tree.
         *
         * @param quadptr A pointer to the quad in which to insert the Entity
         * @param entity_ptr A pointer to the entity to insert into the given Quad or its children.
         * @ return True if the entity is inserted into the quad, False otherwise.
         */
        static bool insert( Ptr& quadptr, Entity::CPtr entity_ptr );

        /**
         * @brief Return the all the Bounds that contains the provided point.
         *
         * @param quadptr A pointer to the quad in which to get the bounds for.
         * @param leaf_only When set to true, only the leaf bounds of the quad
         * will be returned.
         * @param fuzzy When set to true, the bounds returned will reflect the fuzzy boundaries and not the actual boundaries
         * @return A pointer to a new Bounds instance.
         */
        static std::vector<Bounds::Ptr> retrieve_all_bounds( Ptr& quadptr, bool leaf_only = false, bool fuzzy = false );

        /**
         * @brief Construct a Quad
         *
         * @param swpoint The Southwest corner of the Quad.
         * @param nepoint The Northeast corner of the Quad.
         * @param level The numeric level of the quad (root is 0).
         * @param position A string describing the orientation of this Quad (debugging primarily).
         */
        Quad( const Point& swpoint, const Point& nepoint, int level = 0, const std::string& position = "" );

        /**
         * @brief Predicate indicating whether this Quad is split into children.
         *
         * @return true if this Quad has children; false if this Quad is a leaf.
         */
        bool haschildren() const;

        /**
         * @brief Predicate indicating whether this Quad has exceeded the maximum allowable number of elements.
         *
         * @return true if this Quad contains more than maxelements; false otherwise.
         */
        bool full() const;

        /** 
         * @brief Return the set of pointers to Entities in the Quad that contains the provided geopoint.
         *
         * @param pt The point whose containing Quad we are interested in.
         * @return A pointer to the set of entities contained in the same quad that contain pt.
         */
        const Entity::PtrList& retrieve_elements( const Point& pt ) const;

        /**
         * @brief Return the Bounds that contains the provided point.
         *
         * @param pt The point whose containing Quad we are interested in.
         * @param fuzzy When set to true, the bounds returned will reflect the fuzzy boundaries and not the actual boundaries
         * @return A pointer to a new Bounds instance.
         */
        Bounds::Ptr retrieve_bounds( const Point& pt, bool fuzzy = false ) const;

            
        /**
         * @brief Write a Quad as a human-readable string to the provided output stream.
         *
         * String format: Quad: { <sw>, <ne> } element count: <num elements> level: <level> 
         * children: <num children> fuzzy: { <sw>, <ne>, <fuzzy height>, <fuzzy width> }
         *
         * @param os The output stream the write the Quad to.
         * @param quad The Quad to write in human-readable format.
         * @return The output stream after the Quad has been written.
         */
        friend std::ostream& operator<< (std::ostream& os, const Quad& quad);

    private:
        static geo::Vertex::IdToPtrMap elementmap;              ///< Lookup table from vertex unique identifer to pointers to Vertex instance; prevents duplicating Vertex creation.
        static geo::Entity::PtrList empty_element_list;                ///< Fixed empty set of Edges; returned when a point is contained in a Quad with no Entities.

        int level_;                                             ///< The tree depth, or level, of this Quad.
        std::string position_;                                  ///< The relative position of this Quad amoung siblings.

        double fuzzywidth_;                                     ///< The amount to extend the horizontal dimension of the Quad for element insertion purposes.
        double fuzzyheight_;                                    ///< The amount to extend the vertical dimension of the Quad for element insertion purposes.

        Bounds fuzzybounds_;                                    ///< The fuzzy dimensions of this Quad as a Bounds instance.

        PtrList children_;                                      ///< The list of this Quad's children Quads.
        Entity::PtrList element_list_;                             ///< The elements contained in this Quad.

        /**
         * @brief Split this Quad into four children. The child list will be cleared, the children created and inserted. The order in
         * the list is ( NW, NE, SW, SE ); like reading a book (left to right, top to bottom).
         *
         * Note: This method is normally used when the dimensions of the quad are roughly square.
         */
        void quadsplit();

        /**
         * @brief Split this Quad along the horizontal (latitude) axis into two children (West and East). The child list will be cleared, two children (West and East) are created and
         * inserted. The order of the list is ( W, E )
         *
         * Note: This method is normally used when the height of the Quad is significantly larger than the width.
         */
        void horizontalsplit();

        /**
         * @brief Split this Quad along the vertical (longitude) axis into two children (North and South). The child list will be cleared, two children ( North and South ) are created and
         * inserted. The order of the list is ( North, South ).
         *
         * Note: This method is normally used when the width of the Quad is significantly larger than the height.
         */
        void verticalsplit();

        /**
         * @brief Attempt to split this Quad into children and insert those into this Quad's children list.
         *
         * this flag will force the split anyway; defaults to not splitting Quads that are small.
         * @return True if the quad is split, False otherise.
         */
        bool split( );
};

#endif
