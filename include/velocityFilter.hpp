#ifndef CVDP_VELOCITY_FILTER_H
#define CVDP_VELOCITY_FILTER_H

#include <string>
#include <stack>
#include <vector>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include "rapidjson/document.h"

using ConfigMap = std::unordered_map<std::string,std::string>;            ///< An alias to a string key - value configuration for the privacy parameters.


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

#endif