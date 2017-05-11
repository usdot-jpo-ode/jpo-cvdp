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
 * Contributors: Oak Ridge National Laboratory, Center for Trustworthy Embedded Systems, 
 * UT Battelle.
 */

#include "quad.hpp"
#include "string_utilities.hpp"

int    Quad::minlevels = 7;
int    Quad::maxlevels = 9;
double Quad::mindegrees = 0.003;
int    Quad::maxelements = 32;
double Quad::fuzzywidth = 0.0;
double Quad::fuzzyheight = 0.0;

Geo::Vertex::IdToPtrMap Quad::elementmap{};
Geo::Entity::PtrSet Quad::emptyedgeset{};

Quad::Quad( const Geo::Point& swpoint, const Geo::Point& nepoint, int level, const std::string& position )
    : Geo::Bounds{ swpoint, nepoint }, 
    level_{level}, 
    position_{position},
    fuzzywidth_{ Quad::fuzzywidth },            
    fuzzyheight_{ Quad::fuzzyheight }         
{
    if ( fuzzywidth_ == 0.0 ) {
        fuzzywidth_ = width() / REDUCTION_FACTOR;
    }

    if ( fuzzyheight_ == 0.0 ) {
        fuzzyheight_ = height() / REDUCTION_FACTOR;
    }

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

void Quad::split( bool force )
{
    bool isverticalsplit = height() / 2.0 >= Quad::mindegrees;
    bool ishorizontalsplit = width() / 2.0 >= Quad::mindegrees;

    if (isverticalsplit && ishorizontalsplit) {
        quadsplit();

    } else if (isverticalsplit) {
        verticalsplit();

    } else if (ishorizontalsplit) {
        horizontalsplit();

    } else if (force) {
        // Too small to split, but we will force it.
        quadsplit();

    }  // otherwise, Quad is too small to split.
}

bool Quad::haschildren() const
{
    return !children_.empty();
}

bool Quad::full() const
{
    return static_cast<int>(elementset_.size()) > Quad::maxelements;
}

bool Quad::insert( Quad::Ptr& quadptr, Geo::Entity::CPtr entity_ptr )
{
    if ( !entity_ptr->touches(quadptr->fuzzybounds_) ) return false;

    PtrStack quadstack;
    quadstack.push(quadptr);
    bool is_insert = false;

    while (!quadstack.empty()) {

        // attempt to insert this element (edge) in the quad at the top of
        // the stack.
        Ptr currquad = quadstack.top();
        quadstack.pop();

        if (currquad->haschildren()) {
            // this quad has children (implies no elements in this quad)
            for ( auto& quad : currquad->children_ ) {
                // attempt to insert into all child quads that contain the edge.
                if (entity_ptr->touches(quad->fuzzybounds_)) {
                    quadstack.push( quad );
                } 
            }

            continue;

        } else if (currquad->level_ < Quad::minlevels) {
            // quad not a minimum level; split independent of size
            // constraints.
            currquad->split(true);
            for ( auto& quad : currquad->children_ ) {
                // attempt to insert into all child quads that contain the
                // edge.
                if (entity_ptr->touches(quad->fuzzybounds_)) {
                    quadstack.push( quad );
                } 
            }
            continue;

        } else if (currquad->level_ < Quad::maxlevels) {
            // quad not at maximum level; "attempt" to split the quad.
            currquad->split();
            if (currquad->haschildren()) {
                // successful split
                for ( auto& quad : currquad->children_ ) {
                    // attempt to insert into all child quads that contain
                    // edge.
                    if (entity_ptr->touches(quad->fuzzybounds_)) {
                        quadstack.push( quad );
                    }
                }
                continue;
            } // if not split, add the element or force split and redistribute.
        }

        // level is at max or level is less than max and split request failed to produce children.
        currquad->elementset_.insert(entity_ptr);
        is_insert = true;

        if (currquad->full()) {
            // quad is saturated with elements; split and redistribute.
            // add it first so we redistribute everything including this
            // element.
            currquad->split( true );

            for ( auto& e : currquad->elementset_ ) {
                for ( auto& quad : currquad->children_ ) {
                    if (e->touches(quad->fuzzybounds_)) {
                        quad->elementset_.insert(e);
                        is_insert = true;
                    }
                }
            }

            currquad->elementset_.clear();
        } 
    }

    return is_insert;
}

std::ostream& operator<<( std::ostream& os, const Quad& quad )
{
    return os << "Quad: {" << quad.sw << ", " << quad.ne << "} element count: " << quad.elementset_.size() << " level: " << quad.level_ << " children: " << quad.children_.size() << " fuzzy: {" << quad.fuzzybounds_.sw << ", " << quad.fuzzybounds_.ne << ", " << quad.fuzzybounds_.height() << ", " << quad.fuzzybounds_.width() << "}";
}

const Geo::Entity::PtrSet& Quad::retrieve_elements( const Geo::Point& pt ) const
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
        return currquad->elementset_;

    } else {
        return Quad::emptyedgeset;
    }
}

Geo::Bounds::Ptr Quad::retrieve_bounds( const Geo::Point& pt, bool fuzzy ) const
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
            return std::make_shared<Geo::Bounds>(currquad->fuzzybounds_);
        } else {
            return std::make_shared<Geo::Bounds>(*currquad);
        }

    } else {
        return Geo::Bounds::Ptr{};
    }
}

std::vector<Geo::Bounds::Ptr> Quad::retrieve_all_bounds( Quad::Ptr& quadptr, bool leaf_only, bool fuzzy )
{
    std::vector<Geo::Bounds::Ptr> ret;
    PtrStack quadstack;
    quadstack.push(quadptr);
    bool is_insert = false;

    while (!quadstack.empty()) {
        Ptr currquad = quadstack.top();
        quadstack.pop();
        
        if (currquad->haschildren()) {
            for (auto& child : currquad->children_) {
                quadstack.push(child);
            }

            if (!leaf_only) {
                if (fuzzy) {
                    ret.push_back(std::make_shared<Geo::Bounds>(currquad->fuzzybounds_));
                } else {
                    ret.push_back(std::make_shared<Geo::Bounds>(*currquad));
                }
            }
        } else {
            if (fuzzy) {
                ret.push_back(std::make_shared<Geo::Bounds>(currquad->fuzzybounds_));
            } else {
                ret.push_back(std::make_shared<Geo::Bounds>(*currquad));
            }
        }
    }

    return ret;
}
