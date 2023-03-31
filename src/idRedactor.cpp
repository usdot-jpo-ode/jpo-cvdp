#include "idRedactor.hpp"

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
