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

#include <librdkafka/rdkafkacpp.h>
#include <iostream>
#include <sstream>

#include "rapidjson/reader.h"

#include "cvlib.hpp"
#include "bsmfilter.hpp"

IdRedactor::IdRedactor() :
    inclusion_set_{},
    redacted_value_{"FFFFFFFF"},                    // default value.
    inclusions_{false}
{}

IdRedactor::IdRedactor( const ConfigMap& conf ) :
    IdRedactor{}
{
        auto search = conf.find("privacy.redaction.id.value");
        if ( search != conf.end() ) {
            redacted_value_ = search->second;
        }

        search = conf.find("privacy.redaction.id.inclusions");
        if ( search != conf.end() && search->second=="ON" ) {
            inclusions_ = true;
        }

        search = conf.find("privacy.redaction.id.included");
        if ( search != conf.end() ) {
           StrVector sv = string_utilities::split( search->second, ',' );
           for ( auto& id : sv ) {
               inclusion_set_.insert( id );
           }
        }
};

bool IdRedactor::AddIdInclusion( const std::string& id )
{
    auto result = inclusion_set_.insert( id );
    return result.second;
}

bool IdRedactor::RemoveIdInclusion( const std::string& id )
{
    bool r = false;
    auto search = inclusion_set_.find( id );
    if ( search != inclusion_set_.end() ) {
        // id is currently in the inclusion set so erase it.
        inclusion_set_.erase( search );
        r = true;
    }
    return r;
}

bool IdRedactor::operator()( std::string& id )
{
    if ( inclusions_ ) {
        auto search = inclusion_set_.find( id );
        if ( search == inclusion_set_.end() ) {
            // id is not in the inclusion set; NO REDACTION.
            return false;
        }
        // found in exclusion set.
    } 
    
    // Two cases:
    // 1. Use the inclusion set: found it so redact.
    // 2. Do not use the inclusion set: redact everything.
    id = redacted_value_;
    return true;
}

VelocityFilter::VelocityFilter() :
    min_{kDefaultMinVelocity},
    max_{kDefaultMaxVelocity}
{}

VelocityFilter::VelocityFilter( const ConfigMap& conf ) :
    VelocityFilter{}
{
    try {

        auto search = conf.find("privacy.filter.velocity.min");
        if ( search != conf.end() ) {
            min_ = std::stod( search->second );
        }

        search = conf.find("privacy.filter.velocity.max");
        if ( search != conf.end() ) {
            max_ = std::stod( search->second );
        }

    } catch ( std::exception& e ) {
        std::cerr << e.what() << '\n';
    }
}

void VelocityFilter::set_min( double v ) {
    min_ = v;
}

void VelocityFilter::set_max( double v ) {
    max_ = v;
}

bool VelocityFilter::operator()( double v ) {
    return v > min_ && v < max_;
}

BSM::BSM() :
    Geo::Point{90.0, 180.0},
    velocity_{ -1 },
    id_{"UNASSIGNED"}
{}

void BSM::reset() {
    lat = 90.0;
    lon = 180.0;
    velocity_ = -1.0;
    id_ = "UNASSIGNED";
}

void BSM::set_velocity( double v ) {
    velocity_ = v;
}

double BSM::get_velocity() const {
    return velocity_;
}

void BSM::set_latitude( double latitude ) {
    lat = latitude;
}

void BSM::set_longitude( double longitude ) {
    lon = longitude;
}

void BSM::set_id( const std::string& s ) {
    id_ = s;
}

std::ostream& operator<<( std::ostream& os, const BSM& bsm )
{
    os  << "Pos: (" << bsm.lat << ", " << bsm.lon << "), ";
    os  << "Spd: "  << bsm.velocity_ << " ";
    os  << "Id: "  <<  bsm.id_;
    return os;
}

BSMHandler::BSMHandler(Quad::Ptr quad_ptr, const std::unordered_map<std::string,std::string>& conf) :
    reader_{},
    activated_{0},
    result_{ ResultStatus::SUCCESS },
    bsm_{},
    quad_ptr_{quad_ptr},
    get_value_{ false },
    finalized_{ false },
    current_key_{},
    object_stack_{},
    tokens_{},
    json_{},
    vf_{ conf },
    idr_{ conf }
{
    auto search = conf.find("privacy.filter.velocity");
    if ( search != conf.end() && search->second=="ON" ) {
        activated_ |= kVelocityFilterFlag;
    }

    search = conf.find("privacy.filter.geofence");
    if ( search != conf.end() && search->second=="ON" ) {
        activated_ |= kGeofenceFilterFlag;
    }

    search = conf.find("privacy.redaction.id");
    if ( search != conf.end() && search->second=="ON" ) {
        activated_ |= kIdRedactFlag;
    }
}

bool BSMHandler::isWithinEntity(BSM &bsm) const {
    Geo::Circle::CPtr circle_ptr = nullptr;
    Geo::EdgeCPtr edge_ptr = nullptr;
    Geo::Grid::CPtr grid_ptr = nullptr;
    Geo::AreaPtr area_ptr = nullptr;
    Geo::Entity::PtrSet entity_set = quad_ptr_->retrieve_elements(bsm); 

    for (auto& entity_ptr : entity_set) {
        if (entity_ptr->get_type() == "circle") {
            circle_ptr = std::static_pointer_cast<const Geo::Circle>(entity_ptr);

            if (circle_ptr->contains(bsm)) {
                return true;
            }
        } else if (entity_ptr->get_type() == "edge") {
            edge_ptr = std::static_pointer_cast<const Geo::Edge>(entity_ptr); 
            area_ptr = edge_ptr->to_area();

            if (area_ptr->contains(bsm)) {
                return true;
            }
        } else if (entity_ptr->get_type() == "grid") {
            grid_ptr = std::static_pointer_cast<const Geo::Grid>(entity_ptr); 

            if (grid_ptr->contains(bsm)) {
                return true;
            }
        }
    }
    return false;
}

// bool BSMHandler::process( const char*payload, size_t length ) {
bool BSMHandler::process( const std::string& bsm_json ) {

    // TODO: if payload could be assured to be '\0' terminated we could pass the void * cast to
    // this ss and same the creation of the string.
    rapidjson::StringStream ss{ bsm_json.c_str() };

    // we are reusing the bsm instance in the handler, so reset the state.  Not necessary for deployment.
    bsm_.reset();

    rapidjson::ParseResult r = reader_.Parse<BSMHandler::flags>(ss, *this );
    if ( result_ == ResultStatus::SUCCESS && r.Code() != rapidjson::kParseErrorNone ) {
        // nothing triggered a change in status, but we still failed. Parse error.
        result_ = ResultStatus::PARSE;
    }

    return (r.Code() == rapidjson::kParseErrorNone );
}

void BSMHandler::reset() {
    tokens_.clear();
    object_stack_.clear();
    finalized_ = false;
    get_value_ = false;
    current_key_ = "";
    result_ = ResultStatus::SUCCESS;
    json_ = "";
}

BSMHandler::ResultStatus BSMHandler::get_result() const {
    return result_;
}

std::string BSMHandler::get_result_string() {
    switch ( result_ ) {
        case ResultStatus::SUCCESS :
            return "success";
            break;

        case ResultStatus::SPEED :
            return "speed";
            break;

        case ResultStatus::GEOPOSITION :
            return "geoposition";
            break;

        case ResultStatus::PARSE :
            return "parse";
            break;

        case ResultStatus::OTHER :
        default:
            return "other...";
    }
}

const BSM& BSMHandler::get_bsm() const {
    return bsm_;
}

const std::string& BSMHandler::get_bsm_json() {

    if ( !finalized_ ) {
        std::stringstream ss;
        for ( auto& s : tokens_ ) {
            ss << s;
        }
        json_ = ss.str();
        finalized_ = true;
    }

    return json_;
}

std::string::size_type BSMHandler::get_bsm_buffer_size() {
    
    if ( !finalized_ ) {
        get_bsm_json();
    }
    return json_.size();
}

bool BSMHandler::starting_new_object() const {
    return !tokens_.empty() && tokens_.back() == "{";
}

bool BSMHandler::finished_current_object() const {
    return !tokens_.empty() && tokens_.back() == "}";
}

bool BSMHandler::Null() 
{ 
    tokens_.push_back("null");
    if (get_value_) {
        std::cout << "null\n";
    }
    get_value_ = false;

    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::Bool(bool b) 
{ 
    if (b) {
        tokens_.push_back("true"); 
    } else {
        tokens_.push_back("false");
    }

    if (get_value_) {
        std::cout << std::boolalpha << b << '\n';
    }

    get_value_ = false;

    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::Int(int i) 
{
    tokens_.push_back( std::to_string(i) );
    if (get_value_) {
        std::cout << i << '\n';
    }
    get_value_ = false;

    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::Uint(unsigned u) 
{
    tokens_.push_back( std::to_string(u) );
    if (get_value_) {
        std::cout << u << '\n';
    }
    get_value_ = false;

    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::Int64(int64_t i) 
{
    tokens_.push_back( std::to_string(i) );
    if (get_value_) {
        std::cout << i << '\n';
    }
    get_value_ = false;

    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::Uint64(uint64_t u) 
{
    tokens_.push_back( std::to_string(u) );
    if (get_value_) {
        std::cout << u << '\n';
    }
    get_value_ = false;

    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::Double(double d) 
{
    tokens_.push_back( std::to_string(d) );
    if (get_value_) {
        std::cout << d << '\n';
    }
    get_value_ = false;

    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::RawNumber(const char* str, rapidjson::SizeType length, bool copy) { 
    
    tokens_.push_back( std::string(str,length) );

    if (get_value_) {
        get_value_ = false;

        if ("latitude" == current_key_) {
            double v = std::strtod( str, &end_ );
            bsm_.set_latitude( v );
        }

        if ("longitude" == current_key_) {
            double v = std::strtod( str, &end_);
            bsm_.set_longitude( v );
        }

        if ("speed" == current_key_) {
            double v = std::strtod( str, &end_ );
            bsm_.set_velocity( v );
            if ( (activated_ & kVelocityFilterFlag) && !vf_(v) ) {
                // Velocity filtering is activated and filtering failed -> suppress.
                result_ = ResultStatus::SPEED;
            }
        }
    }

    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::String(const char* str, rapidjson::SizeType length, bool copy) { 

    std::string s{ str, length };

    if ( get_value_ && (activated_ & kIdRedactFlag) && (current_key_=="id") ) {
        // previous token requires extracting value.
        // ID redaction is activiated and this is the value associated with the id field.

        // returns false if NO REDACTION.
        idr_( s );

        // this is needed only for print outs; the data in tokens_ is what is returned.
        bsm_.set_id( s );
        get_value_ = false;

    } else if ( get_value_ && (current_key_=="id") ) {
        // TODO: This else branch is here so when the tests are run the bsm id is set to the original value and
        // can be displayed correctly. In operation, only the information on the tokens_ stack is needed, so 
        // this operation is wasted.
        bsm_.set_id( s );
    }

    tokens_.push_back( "\"" + s + "\"" );
    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::StartObject() 
{
    if (finished_current_object()) {
        // an array sequence.
        tokens_.push_back(",");
    }

    tokens_.push_back("{");
    object_stack_.push_back(current_key_);
    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::EndObject(rapidjson::SizeType memberCount) 
{
    tokens_.push_back("}");
    std::string top = object_stack_.back();

    if ( (top=="position") && (activated_ & kGeofenceFilterFlag) && !isWithinEntity(bsm_) ) {
        // Shortcircuting: Parsed core position, geofencing is needed, and inside the geofence.
        result_ = ResultStatus::GEOPOSITION;
    } 

    object_stack_.pop_back();
    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::Key(const char* str, rapidjson::SizeType length, bool copy) {

    if (!starting_new_object()) {
        // commas separate elements within a JSON object.
        tokens_.push_back(",");
    }

    tokens_.push_back("\"" + std::string( str, length) + "\":");
    std::string top = object_stack_.back();
    current_key_ = str;

    if ( top == "position" ) {
        get_value_ = ("latitude"==current_key_ ||  "longitude"==current_key_);
    } else if ( top == "coreData" ) {
        get_value_ = ("id"==current_key_ || "speed"==current_key_ );
    }

    return result_ == ResultStatus::SUCCESS;
}


bool BSMHandler::StartArray() 
{
    tokens_.push_back("[");
    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::EndArray(rapidjson::SizeType elementCount) 
{
    tokens_.push_back("]");
    return result_ == ResultStatus::SUCCESS;
}

/**
 * Testing stub.
 */
int main_old(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Missing path to region file." << std::endl;
        exit(1);
    }

    // Setup the quad.
    Geo::Point sw{ 42.17, -83.91 };
    Geo::Point ne{ 42.431, -83.54 };

    std::string region_file_path = std::string(argv[1]);

    // Declare a quad with the given bounds.
    Quad::Ptr quad_ptr = std::make_shared<Quad>(sw, ne);
    // Read the file and parse the shapes.
    Shapes::CSVInputFactory shape_factory(region_file_path);
    shape_factory.make_shapes();

    // Add all the shapes to the quad.
    for (auto& circle_ptr : shape_factory.get_circles()) {
        Quad::insert(quad_ptr, std::dynamic_pointer_cast<const Geo::Entity>(circle_ptr)); 
    }

    for (auto& edge_ptr : shape_factory.get_edges()) {
        Quad::insert(quad_ptr, std::dynamic_pointer_cast<const Geo::Entity>(edge_ptr)); 
    }

    for (auto& grid_ptr : shape_factory.get_grids()) {
        Quad::insert(quad_ptr, std::dynamic_pointer_cast<const Geo::Entity>(grid_ptr)); 
    }
    return 0;
}