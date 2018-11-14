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

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "cvlib.hpp"
#include "bsmfilter.hpp"
#include "spdlog/spdlog.h"

/** Start IdRedactor */

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

/** Start VelocityFilter */

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

/** Start BSM */

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

/** Start BSMHandler */

BSMHandler::ResultStringMap BSMHandler::result_string_map{
            { ResultStatus::SUCCESS, "success" },
            { ResultStatus::SPEED, "speed" },
            { ResultStatus::GEOPOSITION, "geoposition" },
            { ResultStatus::PARSE, "parse" },
            { ResultStatus::MISSING, "missing" },
            { ResultStatus::OTHER, "other" }
        };

BSMHandler::BSMHandler(Quad::Ptr quad_ptr, const ConfigMap& conf ):
    activated_{0},
    result_{ ResultStatus::SUCCESS },
    bsm_{},
    quad_ptr_{quad_ptr},
    finalized_{ false },
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

    search = conf.find("privacy.redaction.size");
    if ( search != conf.end() && search->second=="ON" ) {
        activate<BSMHandler::kSizeRedactFlag>();
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

        }  else if (entity_ptr->get_type() == "circle") {
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

bool BSMHandler::process( const std::string& bsm_json ) {
    double speed = 0.0;
    double latitude = 0.0;
    double longitude = 0.0;
    std::string id;
    
    // JMC: Attempt to fix memory leak; build and destroy JSON object each time to ensure memory is reclaimed.
    rapidjson::Document document;

    finalized_ = false;
    result_ = ResultStatus::SUCCESS;
    
    // create the DOM
    // check for errors
    if (document.Parse(bsm_json.c_str()).HasParseError()) {
        result_ = ResultStatus::PARSE;

        return false;
    }

    if (!document.IsObject()) {
        result_ = ResultStatus::PARSE;

        return false;
    }

    if (!document.HasMember("metadata")) {
        result_ = ResultStatus::MISSING;

        return false;
    }

    rapidjson::Value& metadata = document["metadata"];

    // switch sanitized flag
    if (!metadata.HasMember("sanitized")) {
        result_ = ResultStatus::MISSING;

        return false;
    }

    if (!metadata["sanitized"].IsBool()) {
        result_ = ResultStatus::OTHER;

        return false;
    }

    metadata["sanitized"] = true;

    // get the payload type
    if (!metadata.HasMember("payloadType")) {
        result_ = ResultStatus::MISSING;

        return false;
    }

    if (!metadata["payloadType"].IsString()) {
        result_ = ResultStatus::OTHER;

        return false;
    }

    std::string payload_type_str = metadata["payloadType"].GetString();

    if (payload_type_str == "us.dot.its.jpo.ode.model.OdeBsmPayload") {
        if (!document.HasMember("payload")) {
            result_ = ResultStatus::MISSING;

            return false;
        }
    
        rapidjson::Value& payload = document["payload"];

        // handle BSM payload
        if (!payload.HasMember("data")) {
            result_ = ResultStatus::MISSING;

            return false;
        }

        rapidjson::Value& data = payload["data"];

        if (!data.HasMember("coreData")) {
            result_ = ResultStatus::MISSING;

            return false;
        }

        rapidjson::Value& core_data = data["coreData"];

        if (!core_data.HasMember("speed")) {
            result_ = ResultStatus::MISSING;

            return false;
        }
        
        if (!core_data["speed"].IsDouble()) {
            result_ = ResultStatus::OTHER;

            return false;
        }

        speed = core_data["speed"].GetDouble();
        bsm_.set_velocity(speed);

        if (is_active<kVelocityFilterFlag>() && vf_.suppress(speed)) {
            result_ = ResultStatus::SPEED;
        }

        if (!core_data.HasMember("position")) {
            result_ = ResultStatus::MISSING;

            return false;
        }

        rapidjson::Value& position = core_data["position"];

        if (!position.HasMember("latitude") || !position.HasMember("longitude")) {
            result_ = ResultStatus::MISSING;

            return false;
        }

        if (!position["latitude"].IsDouble() || !position["longitude"].IsDouble()) {
            result_ = ResultStatus::OTHER;

            return false;
        }
        
        latitude = position["latitude"].GetDouble();
        longitude = position["longitude"].GetDouble();

        bsm_.set_latitude(latitude); 
        bsm_.set_longitude(longitude); 

        if (is_active<kGeofenceFilterFlag>() && !isWithinEntity(bsm_)) {
            result_ = ResultStatus::GEOPOSITION;
        }

        if (!core_data.HasMember("id")) {
            result_ = ResultStatus::MISSING;

            return false;
        }

        if (!core_data["id"].IsString()) {
            result_ = ResultStatus::OTHER;

            return false;
        }

        id = core_data["id"].GetString();

        if (is_active<kIdRedactFlag>()) {
            bsm_.set_original_id(id);
            idr_(id);

            core_data["id"].SetString(id.c_str(), static_cast<rapidjson::SizeType>(id.size()), document.GetAllocator());
        }

        bsm_.set_id(id);

        // Check for BSM size.  
        // Size is a special case; if it's not included, then we do 
        // NOT return an error/suppress
        if (core_data.HasMember("size") && is_active<kSizeRedactFlag>()) {
            // size included
            rapidjson::Value& size = core_data["size"];
          
            if (size.HasMember("length")) {
                // length included; redact
                size["length"] = 0; 
            } 

            if (size.HasMember("width")) {
                // width included; redact
                size["width"] = 0; 
            } 
        }
    } else if (payload_type_str == "us.dot.its.jpo.ode.model.OdeTimPayload") {
        if (!metadata.HasMember("receivedMessageDetails")) {
            result_ = ResultStatus::MISSING;

            return false;
        } 

        rapidjson::Value& received_details = metadata["receivedMessageDetails"];
    
        if (!received_details.HasMember("locationData")) {
            result_ = ResultStatus::MISSING;

            return false;
        }

        rapidjson::Value& location = received_details["locationData"];

        if (!location.HasMember("latitude") || !location.HasMember("longitude") || !location.HasMember("speed")) {
            result_ = ResultStatus::MISSING;

            return false;
        }

        if (!location["latitude"].IsDouble() || !location["longitude"].IsDouble() || !location["speed"].IsDouble()) {
            result_ = ResultStatus::OTHER;

            return false;
        }
        
        latitude = location["latitude"].GetDouble();
        longitude = location["longitude"].GetDouble();
        speed = location["speed"].GetDouble();

        bsm_.set_latitude(latitude); 
        bsm_.set_longitude(longitude); 
        bsm_.set_velocity(speed); 

        if (is_active<kGeofenceFilterFlag>() && !isWithinEntity(bsm_)) {
            result_ = ResultStatus::GEOPOSITION;
        }

        if (is_active<kVelocityFilterFlag>() && vf_.suppress(speed)) {
            result_ = ResultStatus::SPEED;
        }
    } else {
        result_ = ResultStatus::MISSING;

        return false;
    }

    // JMC: Moving this here to finalize the json string instead of in get_json()
    // JMC: Go ahead and write out the BSM in redacted form using the document that we built in
    // JMC: this method.
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    json_ = buffer.GetString();

    // TODO: if we keep this model, this variable serves no purpose.
    finalized_ = true;
    
    return result_ == ResultStatus::SUCCESS;
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
    // rapidjson::StringBuffer buffer;
    // rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    // TODO: The leak is caused by REUSING this document_ instance.  Something in it isn't getting freed.
    // document_.Accept(writer);

    // json_ = buffer.GetString();

    // finalized_ = true;

    // JMC: The json_ string is set in the process method now to avoid the memory leak in RapidJSON.
    // IMPORTANT: Ensure you call the process method prior to attempting to call this method.
    // IMPORTANT: This is what happens in the main loop of the ppm code.
    return json_;
}

std::string::size_type BSMHandler::get_bsm_buffer_size() {
    // JMC: how we are using this it is always finalized now that I moved the document object.
    // if ( !finalized_ ) {
    //     get_json();
    // }

    // JMC: The json_ string is set in the process method now to avoid the memory leak in RapidJSON.
    // IMPORTANT: Ensure you call the process method prior to attempting to call this method.
    // IMPORTANT: This is what happens in the main loop of the ppm code.
    return json_.size();
}

const double BSMHandler::get_box_extension() const
{
    return box_extension_;
}

const VelocityFilter& BSMHandler::get_velocity_filter() const {
    return vf_;
}

const uint32_t BSMHandler::get_activation_flag() const {
    return activated_;
}

const IdRedactor& BSMHandler::get_id_redactor() const {
    return idr_;
}
