#include "bsm.hpp"

BSM::BSM() :
    geo::Point{90.0, 180.0},
    velocity_{ -1 },
    dsec_{0},
    id_{""},
    oid_{""},
    partII_{""},
    logstring_{}
{}

void BSM::reset() {
    lat = 90.0;
    lon = 180.0;
    velocity_ = -1.0;
    dsec_ = 0;
    id_ = "";
    oid_ = "";
    partII_ = "";
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

void BSM::set_partII( const std::string& s ) {
    partII_ = s;
}

const std::string& BSM::get_partII() const {
    return partII_;
}

void BSM::set_coreData( const std::string& s ) {
    coreData_ = s;
}

const std::string& BSM::get_coreData() const {
    return coreData_;
}

std::ostream& operator<<( std::ostream& os, const BSM& bsm )
{
    os  << std::setprecision(16) << "Pos: (" << bsm.lat << ", " << bsm.lon << "), ";
    os  << "Spd: "  << bsm.velocity_ << " ";
    os  << "Id: "  <<  bsm.id_;
    return os;
}