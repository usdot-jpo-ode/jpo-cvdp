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
 * Contributors: Oak Ridge National Laboratory, Center for Trustworthy Embedded Systems, 
 * UT Battelle.
 */

#include "quad.hpp"
#include "utilities.hpp"

geo::Vertex::IdToPtrMap Quad::elementmap{};
geo::Entity::PtrList Quad::empty_element_list{};

Quad::Quad( const geo::Point& swpoint, const geo::Point& nepoint, int level, const std::string& position )
    : geo::Bounds{ swpoint, nepoint }, 
    level_{level}, 
    position_{position}
{
    fuzzywidth_ = width() / REDUCTION_FACTOR;
    fuzzyheight_ = height() / REDUCTION_FACTOR;

    fuzzybounds_.sw.lat = sw.lat - fuzzyheight_;
    fuzzybounds_.sw.lon = sw.lon - fuzzywidth_;

    fuzzybounds_.nw.lat = nw.lat + fuzzyheight_;
    fuzzybounds_.nw.lon = nw.lon - fuzzywidth_;

    fuzzybounds_.ne.lat = ne.lat + fuzzyheight_;
    fuzzybounds_.ne.lon = ne.lon + fuzzywidth_;

    fuzzybounds_.se.lat = se.lat - fuzzyheight_;
    fuzzybounds_.se.lon = se.lon + fuzzywidth_;
}

void Quad::quadsplit()
{
    children_.clear();
    int nextlevel = level_ + 1;
    children_.emplace_back( std::make_shared<Quad>( west_midpoint(), north_midpoint(), nextlevel ) );
    children_.emplace_back( std::make_shared<Quad>( center(), ne, nextlevel ) );
    children_.emplace_back( std::make_shared<Quad>( sw, center(), nextlevel ) );
    children_.emplace_back( std::make_shared<Quad>( south_midpoint(), east_midpoint(), nextlevel ) );
}

void Quad::horizontalsplit()
{
    children_.clear();
    int nextlevel = level_ + 1;
    children_.emplace_back( std::make_shared<Quad>( sw, north_midpoint(), nextlevel ) );
    children_.emplace_back( std::make_shared<Quad>( south_midpoint(), ne, nextlevel ) );
}

void Quad::verticalsplit()
{
    children_.clear();
    int nextlevel = level_ + 1;
    children_.emplace_back( std::make_shared<Quad>( west_midpoint(), ne, nextlevel ) );
    children_.emplace_back( std::make_shared<Quad>( sw, east_midpoint(), nextlevel ) );
}

bool Quad::split()
{
    bool isverticalsplit = height() / 2.0 >= Quad::MIN_DEGREES;
    bool ishorizontalsplit = width() / 2.0 >= Quad::MIN_DEGREES;

    if (isverticalsplit && ishorizontalsplit) {
        quadsplit();

        return true;
    } else if (isverticalsplit) {
        verticalsplit();

        return true;
    } else if (ishorizontalsplit) {
        horizontalsplit();

        return true;
    } 

    return false;
}

bool Quad::haschildren() const
{
    return !children_.empty();
}

bool Quad::full() const
{
    return static_cast<int>(element_list_.size()) > MAX_ELEMENTS;
}

bool Quad::insert( Quad::Ptr& quadptr, geo::Entity::CPtr entity_ptr )
{
    if ( !entity_ptr->touches(quadptr->fuzzybounds_) ) return false;

    PtrStack quadstack;
    quadstack.push(quadptr);

    while (!quadstack.empty()) {
        // attempt to insert this element (edge) in the quad at the top of
        // the stack.
        Ptr currquad = quadstack.top();
        quadstack.pop();

        // Create a list of entities to process.
        if (currquad->haschildren()) {
            // this quad has children (implies no elements in this quad)
            for ( auto& quad : currquad->children_ ) {
                // attempt to insert into all child quads that contain the edge.
                if (entity_ptr->touches(quad->fuzzybounds_)) {
                    quadstack.push( quad );
                } 
            }

            continue;

        } 
        
        // This is a leaf node. Try to insert.
        currquad->element_list_.push_back(entity_ptr);

        // Try to split the quad if its full.
        if (currquad->full() && currquad->split()) {
            // quad is saturated with elements; split and redistribute.
            // add it first so we redistribute everything including this
            // element.
            for ( auto& e : currquad->element_list_ ) {
                for ( auto& quad : currquad->children_ ) {
                    insert(quad, e);
                }
            }

            currquad->element_list_.clear();
        } 
    }

    return true;
}

std::ostream& operator<<( std::ostream& os, const Quad& quad )
{
    return os << "Quad: {" << quad.sw << ", " << quad.ne << "} element count: " << quad.element_list_.size() << " level: " << quad.level_ << " children: " << quad.children_.size() << " fuzzy: {" << quad.fuzzybounds_.sw << ", " << quad.fuzzybounds_.ne << ", " << quad.fuzzybounds_.height() << ", " << quad.fuzzybounds_.width() << "}";
}

const geo::Entity::PtrList& Quad::retrieve_elements( const geo::Point& pt ) const
{
    const Quad* currquad = this;

    if (currquad->contains(pt)) {
        // guard against providing a point that is not contained in the top level quad.
        // this quad and one of the children at every level will contain this point.

        while (currquad->haschildren()) {
            for (auto& child : currquad->children_) {
                // one of these must contain the point.
                if (child->contains( pt )) {
                    currquad = child.get();  // grab the raw pointer.
                    break;                   // stop at the first child; retrieval quads are disjoint.
                }
            }
        }
        return currquad->element_list_;

    } else {
        return Quad::empty_element_list;
    }
}

geo::Bounds::Ptr Quad::retrieve_bounds( const geo::Point& pt, bool fuzzy) const
{
    const Quad* currquad = this;

    if (currquad->contains(pt)) {
        // guard against providing a point that is not contained in the top level quad.
        // this quad and one of the children at every level will contain this point.

        while (currquad->haschildren()) {
            for (auto& child : currquad->children_) {
                // one of these must contain the point.
                if (child->contains( pt )) {
                    currquad = child.get();  // grab the raw pointer.
                    break;                   // stop at the first child; retrieval quads are disjoint.
                }
            }

        }

        if (fuzzy) {
            return std::make_shared<geo::Bounds>(currquad->fuzzybounds_);
        } else {
            return std::make_shared<geo::Bounds>(*currquad);
        }

    } else {
        return geo::Bounds::Ptr{};
    }
}

std::vector<geo::Bounds::Ptr> Quad::retrieve_all_bounds( Quad::Ptr& quadptr, bool leaf_only, bool fuzzy )
{
    std::vector<geo::Bounds::Ptr> ret;
    PtrStack quadstack;
    quadstack.push(quadptr);

    while (!quadstack.empty()) {
        Ptr currquad = quadstack.top();
        quadstack.pop();
        
        if (currquad->haschildren()) {
            for (auto& child : currquad->children_) {
                quadstack.push(child);
            }

            if (!leaf_only) {
                if (fuzzy) {
                    ret.push_back(std::make_shared<geo::Bounds>(currquad->fuzzybounds_));
                } else {
                    ret.push_back(std::make_shared<geo::Bounds>(*currquad));
                }
            }
        } else {
            if (fuzzy) {
                ret.push_back(std::make_shared<geo::Bounds>(currquad->fuzzybounds_));
            } else {
                ret.push_back(std::make_shared<geo::Bounds>(*currquad));
            }
        }
    }

    return ret;
}
