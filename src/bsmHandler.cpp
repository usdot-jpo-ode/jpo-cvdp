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

#include "librdkafka/rdkafkacpp.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>
#include <limits>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "cvlib.hpp"
#include "bsmHandler.hpp"
#include "spdlog/spdlog.h"
#include "redactionPropertiesManager.hpp"

BSMHandler::ResultStringMap BSMHandler::result_string_map{
            { ResultStatus::SUCCESS, "success" },
            { ResultStatus::SPEED, "speed" },
            { ResultStatus::GEOPOSITION, "geoposition" },
            { ResultStatus::PARSE, "parse" },
            { ResultStatus::MISSING, "missing" },
            { ResultStatus::OTHER, "other" }
        };

BSMHandler::BSMHandler(Quad::Ptr quad_ptr, const ConfigMap& conf, std::shared_ptr<PpmLogger> logger ):
    activated_{0},
    result_{ ResultStatus::SUCCESS },
    bsm_{},
    quad_ptr_{quad_ptr},
    finalized_{ false },
    json_{},
    vf_{ conf },
    idr_{ conf },
    box_extension_{ 10.0 },
    logger_{ logger }
{
    if (logger_ == nullptr) {
        std::cerr << "BSMHandler::BSMHandler(): Logger is null! Returning." << std::endl; // cannot use logger here because it is null
        return;
    }
    
    logger_->trace("BSMHandler::BSMHandler(): Constructor called");

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

    search = conf.find("privacy.redaction.general");
    if ( search != conf.end() && search-> second=="ON") {
        activate<BSMHandler::kGeneralRedactFlag>();
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

bool BSMHandler::process( const std::string& message_json ) {
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
    if (document.Parse(message_json.c_str()).HasParseError()) {
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
    
    if (metadata.HasMember("asn1")) {
        metadata["asn1"].SetString("", document.GetAllocator());
    }

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

        handleGeneralRedaction(document); // uses fieldsToRedact.txt
    }
    else {
        // Unsupported payload type
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

void BSMHandler::handleGeneralRedaction(rapidjson::Document& document) {
    if (is_active<kGeneralRedactFlag>()) {
        for (std::string memberPath : rpm.getFields()) {
            bool memberRedacted = rapidjsonRedactor.redactMemberByPath(document, memberPath.c_str());
            if (!memberRedacted) {
                logger_->info("Member not found while handling general redaction! Path: '" + memberPath + "'");
            }
        }

        // attempt to store the redacted coreData and partII in the BSM object
        if (document["payload"]["data"].HasMember("coreData")) {
            std::string coreDataString = rapidjsonRedactor.stringifyValue(document["payload"]["data"]["coreData"]);
            bsm_.set_coreData(coreDataString);
        }

        if (document["payload"]["data"].HasMember("partII")) {
            std::string partIIString = rapidjsonRedactor.stringifyValue(document["payload"]["data"]["partII"]);
            bsm_.set_partII(partIIString);
        }
    }
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

RapidjsonRedactor& BSMHandler::getRapidjsonRedactor() {
    return rapidjsonRedactor;
}