/** 
 * @file 
 * @author Jason M. Carter
 * @author Aaron E. Ferber
 * @date April 2017
 * @version
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
 *    Oak Ridge National Laboratory.
 */

#include <librdkafka/rdkafkacpp.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>
#include <limits>

#include "rapidjson/reader.h"

#include "cvlib.hpp"
#include "bsmfilter.hpp"
#include "spdlog/spdlog.h"

IdRedactor::IdRedactor() :
    inclusion_set_{},
    redacted_value_{"FFFFFFFF"},                    // default value.
    inclusions_{false}                              // redact everything.
{
    // setup random number generator.
    std::random_device rd;
    rgen_ = std::mt19937{ rd() };
    dist_ = std::uniform_int_distribution<uint32_t>{ 0, std::numeric_limits<uint32_t>::max() };
}

IdRedactor::IdRedactor( const ConfigMap& conf ) :
    IdRedactor{}
{
    // TODO: The redaction value is deprecated (it is randomly assigned now).
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

bool IdRedactor::HasInclusions() const
{
    return inclusions_;
}

int IdRedactor::NumInclusions() const
{
    if ( inclusions_ ) {
        return inclusion_set_.size();
    }
    return -1;
}

void IdRedactor::RedactAll()
{
    inclusion_set_.clear();
    inclusions_ = false;
}

bool IdRedactor::ClearInclusions()
{
    bool r = inclusion_set_.size() > 0;
    inclusion_set_.clear();
    return r;
}

bool IdRedactor::AddIdInclusion( const std::string& id )
{
    auto result = inclusion_set_.insert( id );
    if ( !inclusions_ && result.second ) {
        // previously redacting everything, not we are building the inclusion list.
        inclusions_ = true;
    }
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

std::string IdRedactor::GetRandomId()
{
	std::stringstream ss;
    uint32_t v = dist_(rgen_);
    ss << std::hex << std::setfill('0') << std::setw(sizeof(uint32_t)*2) << v;                                                 
	return ss.str();
}

bool IdRedactor::operator()( std::string& id )
{
    if ( inclusions_ ) {
        auto search = inclusion_set_.find( id );
        if ( search == inclusion_set_.end() ) {
            // Case 1: Using inclusion set, but not found; do NOT redact.
            return false;
        }
        // Case 2: Found this id in the inclusions set; redact.
    } // Case 3: Not using inclusion set; redact everything.

    // Case 2 and 3: Overwrite existing id with redaction id.
    //id = redacted_value_;
    id = GetRandomId();
    return true;
}

const std::string& IdRedactor::redaction_value() const
{
    return redacted_value_;
}

VelocityFilter::VelocityFilter() :
    min_{kDefaultMinVelocity},
    max_{kDefaultMaxVelocity}
{}

VelocityFilter::VelocityFilter( const ConfigMap& conf ) :
    VelocityFilter{}
{
    auto search = conf.find("privacy.filter.velocity.min");
    if ( search != conf.end() ) {
        min_ = std::stod( search->second );
    }

    search = conf.find("privacy.filter.velocity.max");
    if ( search != conf.end() ) {
        max_ = std::stod( search->second );
    }
}

void VelocityFilter::set_min( double v ) {
    min_ = v;
}

void VelocityFilter::set_max( double v ) {
    max_ = v;
}

bool VelocityFilter::operator()( double v ) {
    // true = filter it!
    return v < min_ || v > max_;
}

bool VelocityFilter::suppress( double v ) {
    return (*this)(v);
}

bool VelocityFilter::retain( double v ) {
    return !(*this)(v);
}

BSM::BSM() :
    geo::Point{90.0, 180.0},
    velocity_{ -1 },
    dsec_{0},
    id_{""},
    oid_{""},
    logstring_{}
{}

void BSM::reset() {
    lat = 90.0;
    lon = 180.0;
    velocity_ = -1.0;
    dsec_ = 0;
    id_ = "";
    oid_ = "";
}

std::string BSM::logString() {
    logstring_ = "(" + id_;
    logstring_ += "," + std::to_string(dsec_);
    logstring_ += "," + std::to_string(lat);
    logstring_ += "," + std::to_string(lon);
    logstring_ += "," + std::to_string(velocity_);
    logstring_ += ")";
    return logstring_;
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

void BSM::set_secmark( uint16_t dsec ) {
    dsec_ = dsec;
}

uint16_t BSM::get_secmark() const {
    return dsec_;
}

void BSM::set_id( const std::string& s ) {
    id_ = s;
}

const std::string& BSM::get_id() const {
    return id_;
}

// for testing.
void BSM::set_original_id( const std::string& s ) {
    oid_ = s;
}

// for testing.
const std::string& BSM::get_original_id() const {
    return oid_;
}

std::ostream& operator<<( std::ostream& os, const BSM& bsm )
{
    os  << std::setprecision(16) << "Pos: (" << bsm.lat << ", " << bsm.lon << "), ";
    os  << "Spd: "  << bsm.velocity_ << " ";
    os  << "Id: "  <<  bsm.id_;
    return os;
}

BSMHandler::ResultStringMap BSMHandler::result_string_map{
            { ResultStatus::SUCCESS, "success" },
            { ResultStatus::SPEED, "speed" },
            { ResultStatus::GEOPOSITION, "geoposition" },
            { ResultStatus::PARSE, "parse" },
            { ResultStatus::OTHER, "other" }
        };

BSMHandler::BSMHandler(Quad::Ptr quad_ptr, const ConfigMap& conf ):
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
    idr_{ conf },
    box_extension_{ 10.0 }
{
    auto search = conf.find("privacy.filter.velocity");
    if ( search != conf.end() && search->second=="ON" ) {
        activate<BSMHandler::kVelocityFilterFlag>();
    }

    search = conf.find("privacy.filter.geofence");
    if ( search != conf.end() && search->second=="ON" ) {
        activate<BSMHandler::kGeofenceFilterFlag>();
    }

    search = conf.find("privacy.redaction.id");
    if ( search != conf.end() && search->second=="ON" ) {
        activate<BSMHandler::kIdRedactFlag>();
    }

    search = conf.find("privacy.filter.geofence.extension");
    if ( search != conf.end() ) {
        box_extension_ = std::stod( search->second );
    }
}

bool BSMHandler::isWithinEntity(BSM &bsm) const {
    geo::Circle::CPtr circle_ptr = nullptr;
    geo::EdgeCPtr edge_ptr = nullptr;
    geo::Grid::CPtr grid_ptr = nullptr;
    geo::AreaPtr area_ptr = nullptr;

    geo::Entity::PtrList entity_set = quad_ptr_->retrieve_elements(bsm); 

    for (auto& entity_ptr : entity_set) {

        edge_ptr = std::static_pointer_cast<const geo::Edge>(entity_ptr); 

        if (entity_ptr->get_type() == "edge") {
            edge_ptr = std::static_pointer_cast<const geo::Edge>(entity_ptr); 
            area_ptr = edge_ptr->to_area(box_extension_);

            if (area_ptr->contains(bsm)) {
                return true;
            }

        }   else    if (entity_ptr->get_type() == "circle") {
            circle_ptr = std::static_pointer_cast<const geo::Circle>(entity_ptr);

            if (circle_ptr->contains(bsm)) {
                return true;
            }

        } else  if (entity_ptr->get_type() == "grid") {
            grid_ptr = std::static_pointer_cast<const geo::Grid>(entity_ptr); 

            if (grid_ptr->contains(bsm)) {
                return true;
            }

        }
    }
    return false;
}

void BSMHandler::reset() {
    tokens_.clear();
    object_stack_.clear();
    finalized_ = false;
    get_value_ = false;
    current_key_ = "";
    json_ = "";
    result_ = ResultStatus::SUCCESS;
    bsm_.reset();
}

bool BSMHandler::process( const std::string& bsm_json ) {

    reset();

    // TODO: if payload could be assured to be '\0' terminated we could pass the void * cast to
    // this ss and same the creation of the string.
    rapidjson::StringStream ss{ bsm_json.c_str() };

    rapidjson::ParseResult r = reader_.Parse<BSMHandler::flags>(ss, *this );
    if ( result_ == ResultStatus::SUCCESS && r.Code() != rapidjson::kParseErrorNone ) {
        // nothing triggered a change in status, but we still failed. Parse error.
        result_ = ResultStatus::PARSE;
    }
 
    // successful processing is no parse errors; result carries filter/redaction results.
    return (r.Code() == rapidjson::kParseErrorNone );
}

const BSMHandler::ResultStatus BSMHandler::get_result() const {
    return result_;
}

const std::string& BSMHandler::get_result_string() const {
    return result_string_map[ result_ ];
}

BSM& BSMHandler::get_bsm() {
    return bsm_;
}

const std::string& BSMHandler::get_json() {

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

const uint32_t BSMHandler::get_activation_flag() const {
    return activated_;
}

const std::string& BSMHandler::get_current_key() const {
    return current_key_;
}

const StrVector& BSMHandler::get_object_stack() const {
    return object_stack_;
}

const StrVector& BSMHandler::get_tokens() const {
    return tokens_;
}

const VelocityFilter& BSMHandler::get_velocity_filter() const {
    return vf_;
}

const IdRedactor& BSMHandler::get_id_redactor() const {
    return idr_;
}

std::string::size_type BSMHandler::get_bsm_buffer_size() {
    
    if ( !finalized_ ) {
        get_json();
    }
    return json_.size();
}

const double BSMHandler::get_box_extension() const
{
    return box_extension_;
}

bool BSMHandler::get_next_value() const
{
    return get_value_;
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

    get_value_ = false;

    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::Int(int i) 
{
    tokens_.push_back( std::to_string(i) );
    get_value_ = false;

    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::Uint(unsigned u) 
{
    tokens_.push_back( std::to_string(u) );
    get_value_ = false;

    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::Int64(int64_t i) 
{
    tokens_.push_back( std::to_string(i) );
    get_value_ = false;

    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::Uint64(uint64_t u) 
{
    tokens_.push_back( std::to_string(u) );
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
    static char* end;
    
    tokens_.push_back( std::string(str,length) );

    if (get_value_) {
        get_value_ = false;

        if ("latitude" == current_key_) {
            double v = std::strtod( str, &end );            // if cannot be converted 0 is returned.
            bsm_.set_latitude( v );

        } else if ("longitude" == current_key_) {
            double v = std::strtod( str, &end );            // if cannot be converted 0 is returned.
            bsm_.set_longitude( v );

        } else if ("speed" == current_key_) {
            double v = std::strtod( str, &end );            // if cannot be converted 0 is returned.
            bsm_.set_velocity( v );
            if ( is_active<kVelocityFilterFlag>() && vf_.suppress(v) ) {
                // Velocity filtering is activated and filtering failed -> suppress.
                result_ = ResultStatus::SPEED;
            }

        } else if ( "secMark" == current_key_ ) {
            // This is only set for bookeeping and logging purposes.
            uint16_t v = 65535;  // default for j2735 unavailable value in range.

            try {
                v = static_cast<uint16_t>( std::stoi(str) );
            } catch ( std::logic_error& e ) {
                // keep the default.
            }

            bsm_.set_secmark( v );
        } 
    }

    return result_ == ResultStatus::SUCCESS;
}

bool BSMHandler::String(const char* str, rapidjson::SizeType length, bool copy) { 
    static char* end;
    static uint16_t v;

    std::string s{ str, length };

    if ( get_value_ ) {
        get_value_ = false;
        // the previously seen key indicated we need to use this value.
    
        if ( current_key_ == "id" ) {
            if ( is_active<kIdRedactFlag>() ) {
                bsm_.set_original_id(s);        // for unit testing.
                idr_(s);                        // use the redactor.
            }

            // This is set no matter what for logging and bookeeping; it may not have changed and it is not needed for
            // the output json since that is retained in the stack.
            bsm_.set_id(s);

        } 
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

    if ( (top=="position") && is_active<kGeofenceFilterFlag>() && !isWithinEntity(bsm_) ) {
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

    // save in case we need to "produce" this bsm.
    tokens_.push_back("\"" + std::string( str, length) + "\":");
    // get the enclosing json object.
    std::string top = object_stack_.back();

    // sets state, so we can handle the data correctly.
    current_key_ = str;

    if ( top == "position" ) {
        get_value_ = ("latitude"==current_key_ ||  "longitude"==current_key_);
    } else if ( top == "coreData" ) {
        get_value_ = ("id"==current_key_ || "speed"==current_key_ || "secMark"==current_key_ );
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

