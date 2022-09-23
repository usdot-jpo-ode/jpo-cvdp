#ifndef CVDP_ID_REDACTOR_H
#define CVDP_ID_REDACTOR_H

#include <string>
#include <stack>
#include <vector>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include "rapidjson/document.h"

using ConfigMap = std::unordered_map<std::string,std::string>;            ///< An alias to a string key - value configuration for the privacy parameters.

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

#endif