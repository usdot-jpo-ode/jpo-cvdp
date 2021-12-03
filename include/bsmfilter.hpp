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
 *     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef CVDP_BSM_FILTER_H
#define CVDP_BSM_FILTER_H

#include <string>
#include <stack>
#include <vector>
#include <random>
#include "rapidjson/document.h"
#include "cvlib.hpp"

/**
 * @mainpage
 *
 * @section JPO-CVDP
 * 
 * The United States Department of Transportation Joint Program Office (JPO)
 * Connected Vehicle Data Privacy (CVDP) Project is developing a variety of methods
 * to enhance the privacy of individuals who generated connected vehicle data.
 * 
 * Connected vehicle technology uses in-vehicle wireless transceivers to broadcast
 * and receive basic safety messages (BSMs) that include accurate spatiotemporal
 * information to enhance transportation safety. Integrated Global Positioning
 * System (GPS) measurements are included in BSMs.  Databases, some publicly
 * available, of BSM sequences, called trajectories, are being used to develop
 * safety and traffic management applications. BSMs do not contain explicit
 * identifiers that link trajectories to individuals; however, the locations they
 * expose may be sensitive and associated with a very small subset of the
 * population; protecting these locations from unwanted disclosure is extremely
 * important. Developing procedures that minimize the risk of associating
 * trajectories with individuals is the objective of this project.
 * 
 * @section The Operational Data Environment (ODE) Privacy Protection Module (PPM)
 * 
 * The PPM operates on streams of raw BSMs generated by the ODE. It determines
 * whether individual BSMs should be retained or suppressed (deleted) based on the
 * information in that BSM and auxiliary map information used to define a geofence.
 * BSM geoposition (latitude and longitude) and speed are used to determine the
 * disposition of each BSM processed. The PPM also redacts other BSM fields.
 *
 * @section Configuration
 *
 * @section Operation
 */

using ConfigMap = std::unordered_map<std::string,std::string>;            ///< An alias to a string key - value configuration for the privacy parameters.

/**
 * @brief Functor class to enable use of enums as keys to unordered_maps.
 */
struct EnumHash {
    template <typename T>
        /**
         * @brief Return the hash code of a type that can be cast to a size_t, e.g., an enum.
         *
         * @return the hash code.
         */
        std::size_t operator()(T t) const 
        {
            return static_cast<std::size_t>( t );
        }
};

/**
 * @brief An IdRedactor encapsulates whether IdRedaction should take place and how it is performed.
 *
 * If inclusion_set_ is false (the default for the default constructor), ALL IDS will be redacted.
 * If inclusion_set_ is true and the inclusion_set is empty, the NO IDS will be redacted.
 * If inclusion_set_ is true and the inclusion_set is non-empty, then those IDS in the set will be redacted.
 */
class IdRedactor {

    public:

        using InclusionSetType = std::unordered_set<std::string>;        ///< Alias for the inclusion set type.
        
        /**
         * @brief Default Id Redactor constructor.
         *
         * This constructor sets the following defaults:
         * - Redacts all ids.
         * - Sets the default redaction value to FFFFFFFF; this is easily changed in the configuration.
         */
        IdRedactor();

        /**
         * @brief Construct an Id Redactor using the provided configuration.
         *
         * Initializes itself with the default constructor prior to applying configuration settings.
         *
         * @param conf The privacy configuration with which to setup this IdRedactor.
         */
        IdRedactor( const ConfigMap& conf );

        /**
         * @brief Predicate indicating whether of not all ids are redacted.
         *
         * @return true if this IdRedactor uses inclusions and the set is non-empty.
         */
        bool HasInclusions() const;

        /**
         * @brief Return the size of the inclusions sets. 
         *
         * @return The size of the inclusions set; -1 if inclusions are not used.
         */
        int NumInclusions() const;

        /**
         * @brief Reset the state of this redactor to redact all ids; this also empties the inclusion set.
         */
        void RedactAll();

        /**
         * @brief Empty the inclusions set; this has the effect of NO LONGER PERFORMING REDACTION.
         *
         * @return true if the inclusions_set had some items to clear; false if already empty.
         */
        bool ClearInclusions();

        /** 
         * @brief Add an id to the set of Ids that require redaction.
         *
         * If the this redactor was previously set to including
         * everything, i.e., NOT use a redaction list, and you add an id using
         * this method, the previous behavior will stop and only the id you
         * specified in this call will be redacted.
         *
         * @param id an id that requires redaction.  @return true if it was new
         * to the set; false otherwise.
         */
        bool AddIdInclusion( const std::string& id );

        /**
         * @brief Remove an id from the set of Ids that require redaction.
         *
         * @param id an id that no longer should be redacted.
         * @return true if the id was removed; false if it wasn't in the set.
         */
        bool RemoveIdInclusion( const std::string& id );

        /**
         * @brief Build and return a new randomly generated unsigned 32-bit identifier in hex to replace the current identifier.
         *
         * @return a hexidecimal string of a random unsigned 32-bit integer (based on the J2735 specification).
         */
		std::string GetRandomId();

        /**
         * @brief Operator to redact (or retain) an id.
         *
         * @param id the id to redact.
         *
         * @return true the id was redacted (modified) in some way; false the id was not changed.
         */
        bool operator()( std::string& id );

        /**
         * @brief Return the value currently being used for redaction.
         *
         * @return a constanct reference to the value used to replace redacted values.
         */
        const std::string& redaction_value() const;


    private:
        std::mt19937 rgen_;                                     ///< random number generator (mersenne twister).
        std::uniform_int_distribution<uint32_t> dist_;
        InclusionSetType inclusion_set_;                        ///< The set of ids on which to perform redaction.
        std::string redacted_value_;                            ///< The value to assign to those ids that require redaction.
        bool inclusions_;                                       ///< Flag indicating whether this redactor will use the inclusion_set.
};

/**
 * @brief A functor used to determine whether a data element with a velocity field is outside of a specified interval
 * [min,max].
 */
class VelocityFilter {

    public:
        static constexpr double kDefaultMinVelocity = 2.2352;          ///< In meters per second = 5 mph.
        static constexpr double kDefaultMaxVelocity = 35.7632;         ///< In meters per second = 80 mph.
        
        /**
         * @brief Construct a velocity filter with the default velocities of min = 5mph and max = 80mph
         */
        VelocityFilter();

        /**
         * @brief Construct a velocity filter using the specified configuration
         *
         * Note: The default velocities will be set by the default constructor prior to updating those settings with the
         * configuration files, i.e., if you set one and not the other in the configuration the other one will be set to
         * the default value.
         *
         * @param conf The configuration with which to setup this Velocity Filter.
         */
        VelocityFilter( const ConfigMap& conf );

        /**
         * @brief Set the minimum velocity for the filter.
         *
         * @param min the velocity below which items will be filtered.
         */
        void set_min( double v );

        /**
         * @brief Set the maximum velocity for the filter.
         *
         * @param max the velocity above which items will be filtered.
         */
        void set_max( double v );

        /**
         * @brief Predicate function operator indicating whether this velocity should be filtered, i.e. suppressed.
         *
         * The retension interval is closed: [min_, max_]
         *
         * @return true = filter (suppres); false = retain.
         */
        bool operator()( double v );

        /**
         * @brief Predicate function operator indicating whether this velocity should be suppressed; surrogate for filter.
         *
         * The retention interval is closed: [min_, max_]
         *
         * @return true = filter (suppress); false = retain.
         */
        bool suppress( double v );

        /**
         * @brief Predicate function operator indicating whether this velocity should be retained.
         *
         * The retention interval is closed: [min_, max_]
         *
         * @return true = keep this BMS; false = suppress.
         */
        bool retain( double v );

    private:
        double min_;     ///< the minimum velocity for this filter.
        double max_;     ///< the maximum velocity for this filter.
        
};

/**
 * @brief A surrogate for a Basic Safety Message (BSM). Instances of this class carry the information needed to check whether
 * they are contained within a geofence defined using map data. 
 *
 * In anticipation of performaning more advanced privacy filtering or de-identification, the following fields are
 * captured:
 *
 * - velocity
 * - postition (latitude and longitude)
 * - identifier
 */
class BSM : public geo::Point {
    public:

        /**
         * @brief Construct a Basic Safety Message (BSM) surrogate instance to use for filtering.
         */
        BSM();

        /**
         * @brief Reset this BSM instance to the default values.
         *
         * We are reusing a single instance for the purpose of troubleshooting. When suppressed the data in this
         * instance is ignored. The reset method avoid confusion when there data is retained from the previous Bsm
         * instance to the next.
         *
         */
        void reset();

        /**
         * @brief Set the BSM's velocity
         *
         * @param v the BSM's velocity in meters per second.
         */
        void set_velocity( double v );

        /**
         * @brief Get the velocity set for this BSM.
         *
         * @return this BSM's velocity.
         */
        double get_velocity() const;

        /**
         * @brief Set the BSM's latitude.
         *
         * @param latitude the latitude in decimal degrees for the BSM.
         */
        void set_latitude( double latitude );

        /**
         * @brief Set the BSM's longitude.
         *
         * @param longitude the longitude in decimal degrees for the BSM.
         */
        void set_longitude( double longitude );

        /**
         * @brief Set the BSM's secMark.
         *
         * @param dsec the secMark (DSeconds) field of the BSM.
         */
        void set_secmark( uint16_t dsec );

        /**
         * @brief Get the BSM's secMark.
         *
         * @return the BSM's current secMark value.
         */
        uint16_t get_secmark() const;

        /**
         * @brief Set the temporary ID field for the BSM
         *
         * @param id the temporary id for the BSM.
         */
        void set_id( const std::string& s );

        /**
         * @brief Set the original id field for the BSM; for unit testing.
         *
         * @param the original id in the BSM.
         */
        void set_original_id( const std::string& s );

        /**
         * @brief Get the temporary ID field for the BSM
         *
         * @return a const reference to the temporary id for the BSM.
         */
        const std::string& get_id() const;

        /**
         * @brief Get the original ID field for the BSM; for unit testing.
         *
         * @return a const reference to the original id for the BSM.
         */
        const std::string& get_original_id() const;

        /**
         * @brief Set the partII field for the BSM
         *
         * @param id the partII field for the BSM.
         */
        void set_partII( const std::string& s );

        /**
         * @brief Get the partII field for the BSM
         *
         * @return a const reference to the partII field for the BSM.
         */
        const std::string& get_partII() const;

        /**
         * @brief Get a string representation of this BSM for the log.
         *
         * @return a string for the log that characterizes this BSM.
         */
        std::string logString();

        /**
         * @brief Write the BSM in readable form to the provided output stream.
         *
         * @param os the output stream to write the BSM to.
         * @param bsm the bsm instance.
         */
        friend std::ostream& operator<<( std::ostream& os, const BSM& bsm );

    private:
        double velocity_;                       ///< the velocity of the BSM.
        uint16_t dsec_;                         ///< the dsecond field if it exists.
        std::string id_;                        ///< the id of the BSM.
        std::string oid_;                        ///< the original id of the BSM.
        std::string partII_;                    ///< the partII field of the BSM (after redaction)
        char* end_;                             ///< pointer to the last character parsed.
        std::string logstring_;           ///< a string to build for logging about the BSM.
};

/** 
 * @brief A BSMHandler processes individual BSMs specified in JSON. While performing this parsing it updates (creates) a
 * BSM instance. A BSMHandler maintains state during the parsing and discontinues parsing if the BSM is determined to
 * be outside of the prescribed specification for BSM retention. Handlers are designed to be used repeatedly.  
 *
 * Currently BSMs are retained if the following conditions are met:
 *
 * - The velocity is within a specified interval [min,max].
 * - The position is within a prescribed geofence; the geofence is defined using OSM road segments.
 *
 * Currently the following BSM fields are redacted:
 *
 * - The id field is redacted for certain prescribed ids.
 *
 */
class BSMHandler {
    public:
        /**
         * records the status of the parsing including what caused parsing to stop, i.e., the point to be suppressed.
         */
        enum ResultStatus : uint16_t { SUCCESS, SPEED, GEOPOSITION, PARSE, MISSING, OTHER };

        using Ptr = std::shared_ptr<BSMHandler>;                                ///< Handle to pass this handler around efficiently.
        using ResultStringMap = std::unordered_map<ResultStatus,std::string,EnumHash>;   ///< Quick retrieval of result string.

        static ResultStringMap result_string_map;

        static constexpr uint32_t kVelocityFilterFlag = 0x1 << 0;
        static constexpr uint32_t kGeofenceFilterFlag = 0x1 << 1;
        static constexpr uint32_t kIdRedactFlag       = 0x1 << 2;
        static constexpr uint32_t kSizeRedactFlag     = 0x1 << 4;
        static constexpr uint32_t kPartIIRedactFlag   = 0x1 << 8;

        // must be static const to compose these flags and use in template specialization.
        static const unsigned flags = rapidjson::kParseDefaultFlags | rapidjson::kParseNumbersAsStringsFlag;


        /**
         * @brief Construct a BSMHandler instance using a quad tree of the map data defining the geofence and user-specified
         * configuration.
         *
         * @param quad_ptr the quad tree containing the map elements.
         * @param conf the user-specified configuration.
         */
        BSMHandler(Quad::Ptr quad_ptr, const ConfigMap& conf );

        /**
         * @brief Predicate indicating whether the BSM's position is within the prescribed geofence.
         *
         * @todo: entities use string type values; numeric types would be faster.
         *
         * @param bsm the BSM to be checked.
         * @return true if the BSM is within the geofence; false otherwise.
         */
        bool isWithinEntity(BSM &bsm) const;

        /** 
         * @brief Process a BSM presented as a JSON string; the string should not have any newlines in it.
         *
         * The result of the processing besides SAX fail/succeed status can be obtained using the #get_result method.
         *
         * @param bsm_json a JSON string of the BSM.  
         * @return true if the SAX parser did not encounter any errors during parsing; false otherwise.
         *
         */
        bool process( const std::string& bsm_json );

        /**
         * @brief Return the result of the most recent BSM processing.
         *
         * @return the parsing result status including success or if failure which element caused the failure.
         */
        const BSMHandler::ResultStatus get_result() const;

        /**
         * @brief Return the result of the most recent BSM processing as a string.
         *
         * @return the parsing result status including success or if failure which element caused the failure as a
         * string.
         */
        const std::string& get_result_string() const;

        /**
         * @brief Return a reference to the BSM instance generated during processing of a JSON string.
         *
         * @return a reference to the BSM instance.
         */
        BSM& get_bsm();

        /**
         * @brief Return the processed BSM as a JSON string including any changes made due to redaction of fields. This string
         * is suitable for output and does not contain any newlines.
         *
         * @return a constant reference to the processed BSM as a JSON string.
         */
        const std::string& get_json();

        /**
         * @brief Return the size in characters (bytes) of the JSON represented of the processed BSM.
         *
         * @return the size of the JSON string in chars or bytes.
         */
        std::string::size_type get_bsm_buffer_size(); 

        /**
         * @brief This method converts a variable of rapidjson::Value& type to a string.
         * 
         * @param value The rapidjson::Value& variable to convert.
         * @return string form of the rapidjson::Value& variable
         */
        std::string convertRapidjsonValueToString(rapidjson::Value& value);

        template<uint32_t FLAG>
        bool is_active() {
            return activated_ & FLAG;
        }

        template<uint32_t FLAG>
        const uint32_t activate() {
            activated_ |= FLAG;
            return activated_;
        }

        template<uint32_t FLAG>
        const uint32_t deactivate() {
            activated_ &= ~FLAG;
            return activated_;
        }

        const uint32_t get_activation_flag() const;
        const VelocityFilter& get_velocity_filter() const;
        const IdRedactor& get_id_redactor() const;

        /**
         * @brief for unit testing only.
         */
        const double get_box_extension() const;
        
    private:

        // JMC: The leak seems to be caused by re-using the RapidJSON document instance.
        // JMC: We will use a unique instance for each message.
        // rapidjson::Document document_;              ///< JSON DOM

        uint32_t activated_;                        ///< A flag word indicating which features of the privacy protection are activiated.

        bool finalized_;                            ///< Indicates the JSON string after redaction has been created and retrieved.
        ResultStatus result_;                       ///< Indicates the current state of BSM parsing and what causes failure.
        BSM bsm_;                                   ///< The BSM instance that is being built through parsing.
        Quad::Ptr quad_ptr_;                        ///< A pointer to the quad tree containing the map elements.
        bool get_value_;                            ///< Indicates the next value should be saved.
        std::string json_;                          ///< The JSON string after redaction.

        VelocityFilter vf_;                         ///< The velocity filter functor instance.
        IdRedactor idr_;                            ///< The ID Redactor to use during parsing of BSMs.

        double box_extension_;                      ///< The number of meters to extend the boxes that surround edges and define the geofence.
};

#endif
