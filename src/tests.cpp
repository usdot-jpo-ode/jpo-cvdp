#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "bsmfilter.hpp"

TEST_CASE( "Redactor Checks", "[redactor]" ) {

    ConfigMap conf{ 
        { "privacy.redaction.id.value", "---" },
        { "privacy.redaction.id.inclusions", "ON" },
        { "privacy.redaction.id.included", "ID1,ID2" },
    };

    std::string r;
    IdRedactor idr{ conf };

    SECTION( "Inclusion State Checks" ) {

        CHECK( idr.HasInclusions() );
        CHECK( idr.NumInclusions() == 2 );
        idr.ClearInclusions();
        CHECK( idr.NumInclusions() == 0 );
        idr.RedactAll();
        CHECK_FALSE( idr.HasInclusions() );
        CHECK( idr.NumInclusions() == -1 );
    }

    SECTION( "Inclusion Redaction" ) {

        r = "ID1";
        CHECK( idr(r) );
        CHECK( r == "---" );
    }

    SECTION( "Exclusion No Redact" ) {

        r = "IDX";
        CHECK_FALSE( idr(r) );
        CHECK( r == "IDX" );
    }

    SECTION( "Add Id" ) {

        idr.AddIdInclusion( "ID3" );
        CHECK( idr.HasInclusions() );
        CHECK( idr.NumInclusions() == 3 );

        SECTION( "Inclusion Redaction" ) {
            r = "ID3";
            CHECK( idr(r) );
            CHECK( r == "---" );
        }
    }

    SECTION( "Remove Id" ) {

        idr.RemoveIdInclusion( "ID1" );
        CHECK( idr.HasInclusions() );
        CHECK( idr.NumInclusions() == 1 );

        SECTION( "Inclusion Redaction" ) {
            r = "ID2";
            CHECK( idr(r) );
            CHECK( r == "---" );
        }
        SECTION( "Exclusion No Redaction" ) {
            r = "ID1";
            CHECK_FALSE( idr(r) );
            CHECK( r == "ID1" );
        }
    }

    SECTION( "Reset to Redact All" ) {

        idr.RedactAll();
        r = "IDX";
        CHECK( idr(r) );
        CHECK( r == "---" );
    }

    SECTION( "Clear Inclusions - Redact NOTHING" ) {

        idr.ClearInclusions();
        r = "ID1";
        CHECK_FALSE( idr(r) );
        CHECK( r == "ID1" );
        r = "ID2";
        CHECK_FALSE( idr(r) );
        CHECK( r == "ID2" );

    }
}

