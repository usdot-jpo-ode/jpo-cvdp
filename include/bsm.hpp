#ifndef CVDP_BSM_H
#define CVDP_BSM_H

#include <string>
#include <stack>
#include <vector>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <iomanip>
#include "rapidjson/document.h"
#include "cvlib.hpp"

using ConfigMap = std::unordered_map<std::string,std::string>;            ///< An alias to a string key - value configuration for the privacy parameters.


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
         * @brief Get the partII field for the BSM after redaction.
         *
         * @return a const reference to the partII field for the BSM after redaction. If partII redaction is disabled, this will be empty.
         */
        const std::string& get_partII() const;

        /**
         * @brief Set the coreData field for the BSM
         *
         * @param id the coreData field for the BSM.
         */
        void set_coreData( const std::string& s );

        /**
         * @brief Get the coreData field for the BSM after redaction.
         *
         * @return a const reference to the coreData field for the BSM after redaction. If partII redaction is disabled, this will be empty.
         */
        const std::string& get_coreData() const;

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
        std::string coreData_;                    ///< the coreData field of the BSM (after redaction)
        char* end_;                             ///< pointer to the last character parsed.
        std::string logstring_;           ///< a string to build for logging about the BSM.
};

#endif