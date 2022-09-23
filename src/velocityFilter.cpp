#include "../include/velocityFilter.hpp"

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
