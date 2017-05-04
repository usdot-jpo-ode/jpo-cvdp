#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "bsmfilter.hpp"

TEST_CASE( "Add Ids to Redactor", "[redactor]" ) {


    ConfigMap conf{};
    IdRedactor idr{ conf };
    idr.AddIdInclusion( "id1" );
    idr.AddIdInclusion( "id2" );

//    SECTION( "Has two ids" ) {
//
//        std::string r{"id1"};
//        r = idr( r );
//        REQUIRE( r == "id1" );
//
//    }
}

