#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "cvlib.hpp"

#include "bsmfilter.hpp"

TEST_CASE("Entity", "[entity test]") {
    SECTION("Conversions") {
        CHECK(Geo::to_degrees(0.0) == Approx(0.0));
        CHECK(Geo::to_degrees(.5 * Geo::kPi) == Approx(90.0));
        CHECK(Geo::to_degrees(Geo::kPi) == Approx(180.0));
        CHECK(Geo::to_degrees(2.0 * Geo::kPi) == Approx(360.0));
        CHECK(Geo::to_radians(0.0) == Approx(0.0));
        CHECK(Geo::to_radians(90.0) == Approx(.5 * Geo::kPi));
        CHECK(Geo::to_radians(180.0) == Approx(Geo::kPi));
        CHECK(Geo::to_radians(360.0) == Approx(2.0 * Geo::kPi));
    }

    // semi-circumference of Earth (meters)
    const double kSemiCircM = 20037508.3427892439;
    // Eiffel Tower to Titanic distance (meters)
    const double tToE = 4084152.4248618507;
    const double tToEApprox = 4167612.3240307244;
    // Titanic to Eiffel Tower bearing (degrees)
    const double tToEBearing = 60.5340135811;
    // Eiffel Tower to Titanic bearing (degrees)
    const double eToTBearing = 279.0319532512;

    Geo::Location loc_a(90.0, 180.0);  
    Geo::Location loc_b(90.0, 180.0, 1);  
    Geo::Location loc_c(-90.0, 180.0);
    Geo::Location loc_d(0.0, 0.0);
    Geo::Location loc_e(0.00001, 0.0);
    Geo::Location loc_f(0.0, 0.00001);
    Geo::Location loc_g(0.0001, 0.0);
    Geo::Location loc_h(0.0, 0.0001);
    // Eiffel Tower
    Geo::Location loc_i(48.857801, 2.295968);
    // Titanic
    Geo::Location loc_j(41.728342, -49.948810);

    SECTION("Location") {
        // Test the floating point comparisons.
        CHECK(loc_a == loc_a);
        CHECK_FALSE(loc_a == loc_b);
        CHECK_FALSE(loc_d == loc_e); 
        CHECK(loc_d.lat == Approx(loc_e.lat));
        CHECK(loc_d.lon == Approx(loc_f.lon));
        CHECK_FALSE(loc_d.lat == Approx(loc_g.lat));
        CHECK_FALSE(loc_d.lon == Approx(loc_h.lon));
        // Test the distance functions.
        CHECK(Geo::Location::distance(loc_a, loc_b) == Approx(0.0));
        CHECK(Geo::Location::distance(loc_a, loc_c) == Approx(kSemiCircM));
        CHECK(Geo::Location::distance(loc_i, loc_j) == Approx(tToEApprox));
        CHECK(Geo::Location::distance(90.0, 180.0, 90.0, 180.0) == Approx(0.0));
        CHECK(Geo::Location::distance(90.0, 180.0, -90.0, 180.0) == Approx(kSemiCircM));
        CHECK(Geo::Location::distance_haversine(loc_a, loc_a) == Approx(0.0));
        CHECK(Geo::Location::distance_haversine(loc_a, loc_c) == Approx(kSemiCircM));
        CHECK(Geo::Location::distance_haversine(loc_i, loc_j) == Approx(tToE));
        CHECK(Geo::Location::distance(90.0, 180.0, 90.0, 180.0) == Approx(0.0));
        CHECK(Geo::Location::distance(90.0, 180.0, -90.0, 180.0) == Approx(kSemiCircM));
        CHECK(loc_a.distance_to(loc_b) == Approx(0.0));
        CHECK(loc_a.distance_to(loc_c) == Approx(kSemiCircM));
        CHECK(loc_i.distance_to(loc_j) == Approx(tToEApprox));
        CHECK(loc_a.distance_to_haversine(loc_a) == Approx(0.0));
        CHECK(loc_a.distance_to_haversine(loc_c) == Approx(kSemiCircM));
        CHECK(loc_i.distance_to_haversine(loc_j) == Approx(tToE));
        // Test the projection functions.
        CHECK(Geo::Location::project_position(loc_a, 90.0, kSemiCircM).lat == Approx(-90.0));
        CHECK(Geo::Location::project_position(loc_a, 157.0, kSemiCircM / 2.0).lat == Approx(0.0));
        CHECK(Geo::Location::project_position(loc_a, 157.0, kSemiCircM / 2.0).lon == Approx(-157.0));
        CHECK(Geo::Location::project_position(loc_a, -45.0, kSemiCircM / 2.0).lat == Approx(0.0));
        CHECK(Geo::Location::project_position(loc_a, -45.0, kSemiCircM / 2.0).lon == Approx(45.0));
        CHECK(loc_a.project_position(90.0, kSemiCircM).lat == Approx(-90.0));
        CHECK(loc_a.project_position(157.0, kSemiCircM / 2.0).lat == Approx(0.0));
        CHECK(loc_a.project_position(157.0, kSemiCircM / 2.0).lon == Approx(-157.0));
        CHECK(loc_a.project_position(-45.0, kSemiCircM / 2.0).lat == Approx(0.0));
        CHECK(loc_a.project_position(-45.0, kSemiCircM / 2.0).lon == Approx(45.0));
        CHECK(Geo::Location::project_position(90.0, 180.0, 90.0, kSemiCircM).lat == Approx(-90.0));
        CHECK(Geo::Location::project_position(90.0, 180.0, 157.0, kSemiCircM / 2.0).lat == Approx(0.0));
        CHECK(Geo::Location::project_position(90.0, 180.0, 157.0, kSemiCircM / 2.0).lon == Approx(-157.0));
        CHECK(Geo::Location::project_position(90.0, 180.0, -45.0, kSemiCircM / 2.0).lat == Approx(0.0));
        CHECK(Geo::Location::project_position(90.0, 180.0, -45.0, kSemiCircM / 2.0).lon == Approx(45.0));
        // Test the midpoint functions.
        CHECK(Geo::Location::midpoint(loc_a, loc_c).lat == Approx(0.0));
        CHECK(Geo::Location::midpoint(loc_a, loc_c).lon == Approx(-180.0));
        CHECK(loc_a.midpoint(loc_c).lat == Approx(0.0));
        CHECK(loc_a.midpoint(loc_c).lon == Approx(-180.0));
        CHECK(Geo::Location::midpoint(90.0, 180.0, -90.0, 180.0).lat == Approx(0.0));
        CHECK(Geo::Location::midpoint(90.0, 180.0, -90.0, 180.0).lon == Approx(-180.0));
        // Test the bearing functions.
        CHECK(Geo::Location::bearing(loc_a, loc_c) == Approx(180.0));
        CHECK(Geo::Location::bearing(loc_a, loc_d) == Approx(0.0));
        CHECK(Geo::Location::bearing(loc_i, loc_j) == Approx(eToTBearing));
        CHECK(Geo::Location::bearing(loc_j, loc_i) == Approx(tToEBearing));
        CHECK(loc_a.bearing_to(loc_c) == Approx(180.0));
        CHECK(loc_a.bearing_to(loc_d) == Approx(0.0));
        CHECK(loc_i.bearing_to(loc_j) == Approx(eToTBearing));
        CHECK(loc_j.bearing_to(loc_i) == Approx(tToEBearing));
        CHECK(Geo::Location::bearing(90.0, 180.0, -90.0, 180.0) == Approx(180.0));
        CHECK(Geo::Location::bearing(90.0, 180.0, 0.0, 0.0) == Approx(0.0));
    }
    
    /*
    Geo::Vertex v_a(loc_a);
    Geo::Vertex v_b(loc_b);

    Geo::Edge edge_a(v_a, v_b);

    SECTION("Edge") {
        
    }
    */
}

TEST_CASE( "Redactor Checks", "[redactor]" ) {

    ConfigMap conf{ 
        { "privacy.redaction.id.value", "---" },
        { "privacy.redaction.id.inclusions", "ON" },
        { "privacy.redaction.id.included", "ID1,ID2" },
    };

    std::string r;
    IdRedactor idr{ conf };

    SECTION( "Inclusion State Checks" ) {

        // checks from setup above.
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

TEST_CASE( "Velocity Filter", "[velocity filter]" ) {

    ConfigMap conf{ 
        { "privacy.filter.velocity.min", "5" },
        { "privacy.filter.velocity.max", "100" }
    };

    VelocityFilter vf{ conf };

    SECTION( "Using Configuration" ) {
        CHECK( vf( 4.999 ) );
        CHECK( vf( 100.001 ) );
        CHECK_FALSE( vf( 10 ) );

        CHECK( vf.suppress( 4.999 ) );
        CHECK( vf.suppress( 100.001 ) );
        CHECK_FALSE( vf.suppress( 10 ) );

        CHECK( vf.retain( 5 ) );
        CHECK( vf.retain( 100 ) );
        CHECK( vf.retain( 10 ) );
        CHECK_FALSE( vf.retain( 4.999 ) );
        CHECK_FALSE( vf.retain( 100.001 ) );
    }

    SECTION( "Changing the filter's min and max" ) {
        vf.set_min( 3.0 );
        vf.set_max( 200.0 );
        CHECK( vf( 2.999 ) );
        CHECK( vf( 200.001 ) );
        CHECK_FALSE( vf( 10 ) );

        CHECK( vf.suppress( 2.999 ) );
        CHECK( vf.suppress( 200.001 ) );
        CHECK_FALSE( vf.suppress( 10 ) );

        CHECK( vf.retain( 3 ) );
        CHECK( vf.retain( 200 ) );
        CHECK( vf.retain( 10 ) );
        CHECK_FALSE( vf.retain( 2.999 ) );
        CHECK_FALSE( vf.retain( 200.001 ) );
    }
}

TEST_CASE( "BSM Checks", "[bsm]" ) {

    BSM bsm;

    SECTION( "Default Construction" ) {
        CHECK( bsm.lat == 90.0 );
        CHECK( bsm.lon == 180.0 );
        CHECK( bsm.get_velocity() == -1.0 );
        CHECK( bsm.get_id() == "UNASSIGNED" );
    }

    SECTION( "Change and Reset" ) {

        bsm.set_latitude( 22.0 );
        CHECK( bsm.lat == 22.0 );
        bsm.set_longitude( 22.0 );
        CHECK( bsm.lon == 22.0 );
        bsm.set_id( "XXX" );
        CHECK( bsm.get_id() == "XXX" );
        bsm.set_velocity( 456 );
        CHECK( bsm.get_velocity() == 456.0 );

        SECTION( "Reset" ) {
            bsm.reset();
            CHECK( bsm.lat == 90.0 );
            CHECK( bsm.lon == 180.0 );
            CHECK( bsm.get_velocity() == -1.0 );
            CHECK( bsm.get_id() == "UNASSIGNED" );
        }
    }
}

/*
TEST_CASE( "Parse Shape File Data", "[quadtree]" ) {

    // Edge Specification:
    // - line_parts[0] : "edge"
    // - line_parts[1] : unique 64-bit integer identifier
    // - line_parts[2] : A sequence of two colon-split points; 
    //      - each point is semi-colon split.
    //      - Point: <uid>;latitude;longitude
    // - line_parts[3] : A sequence of colon-split key=value attributes.
    //      - Attribute Pair: <attribute>=<value>
    //
    // Circle Specification:
    // - line_parts[0] : "circle"
    // - line_parts[1] : unique 64-bit integer identifier
    // - line_parts[2] : A sequence of colon-split elements that define the center.
    //      - Center: <lat>:<lon>:<radius in meters>
    // 
    // Grid Specification:
    // - line_parts[0] : "grid"
    // - line_parts[1] : A '_' split row-column pair.
    // - line_parts[2] : A sequence of 4 colon-split elements defining the grid position.
    //      - Point: <sw lat>:<sw lon>:<ne lat>:<ne lon>
    //
    

//type,id,geography,attributes

    std::array<std::string,13> edge_tests = {
        "",
        "edge, 1",
        "edge, 1, 4, 5, 3",

        "edge, 2, 0, 0",
        "edge, 3, 0, 0, 0, 0, 0, x, y",
        "edge, 4, 0;2.4;5.5:3.2",
        "edge, 0, 0;41.24789403:XXX:1;41.24746145;-111.0455124, way_type=primary:way_id=80",
        "edge, 0, 0;41.24789403:-111.0467118:1;41.24746145;-111.0455124,wayt_type=primary:way_id=80",
        "edge, 0, 0;41.24789403:-111.0467118:1;fdsdfsdf;-111.0455124,way_type=primary:way_id=80",
        "edge, 0, 0;41.24789403:-111.0467118:1;41.24746145;-111.0455124,wayt_type=primary:way_id=80",
        "edge, 70296,62616666   ;42.2930519   ;-83.7353919:62616669;42.293553;-83.734748,way_type=secondary:way_id=234816700",
        "edge, 70296,62616666   ;42.2930519   ;-83.7353919:62616669;42.293553;-83.734748,way_type=secondary:way_id=234816700",
        "edge, 70296, 62616666;42.2930519;-83.7353919 : 62616669;42.293553;-83.734748 : 62616669;42.293553;-83.734748 , way_type=xxxx:way_id=234816700"
    };

//    std::array<std::string,4> circle_tests = {
//        "circle,2,42.2930519:-83.7353919:2",
//        "circle,1,42.2930519:-83.7353919:,way_type=xxxx:way_id=234816700",
//        "circle,3,42.2930519::3,way_type=xxxx:way_id=234816700",
//        "circle,,::4,way_type=xxxx:way_id=234816700"
//    };
//
//    std::array<std::string,2> grid_tests = {
//        "grid,1_2,42.29:-83.73:43.29:-85.73",
//        "grid,2_9,12.29:-23.73:42.29:-83.73"
//    };

    Shapes::CSVInputFactory sf{};
    for ( int i = 0; i < 3; ++i ) {
        StrVector parts = string_utilities::split(edge_tests[i], ',');
        REQUIRE_THROWS_AS( sf.make_edge( parts ), std::logic_error );
    }

    for ( auto& et : edge_tests ) {
        StrVector parts = string_utilities::split(et, ',');
        
        try {
            sf.make_edge(parts);
            // catch invalid_argument exceptions for fewer than 3 and more than 4 args.
            // catch invalid_way_exception; throw out way_types in blacklist.
            // empty key or values will do nothing, but shouldn't change atts.
            // undefined way types will result in OTHER for way type.
        } catch (std::exception& e) {
            // Deal with all the exceptions thrown from the make_<shape> methods.
            // Skip the specification and move to the next shape.
            // TODO: need some logging here.
            std::cerr << "Failed to make shape: " << e.what() << std::endl;
        }

        sf.make_edge( parts );
    }
}
*/
/**
TEST_CASE( "Build Quad Tree", "[quadtree]" ) {

    const std::string map_data_file = "./data/plymouth_rd.data";
    Quad::Ptr quad_ptr = std::make_shared<Quad>(sw, ne);
    Shapes::CSVInputFactory shape_factory( map_data_file );

    shape_factory.make_shapes();

        // Add all the shapes to the quad.
        for (auto& circle_ptr : shape_factory.get_circles()) {
            Quad::insert(quad_ptr, std::dynamic_pointer_cast<const Geo::Entity>(circle_ptr)); 
        }

        for (auto& edge_ptr : shape_factory.get_edges()) {
            Quad::insert(quad_ptr, std::dynamic_pointer_cast<const Geo::Entity>(edge_ptr)); 
        }

        for (auto& grid_ptr : shape_factory.get_grids()) {
            Quad::insert(quad_ptr, std::dynamic_pointer_cast<const Geo::Entity>(grid_ptr)); 
        }
        return 0;
}

TEST_CASE( "BSMHandler Checks", "[bsm handler]" ) {

    // keep in mind that the individual filters/checkers have been checked, so
    // this should at a higher level abstraction.
    
    // process is the big check; hardcode some BSM json here.
    // 1. Actual BSM
    // 2. Junk BSM
    
    // Check situations that generate all results (and the string corresponds)
    // 1. SUCCESS
    // 2. SPEED
    // 3. GEOPOSITION
    // 4. PARSE
    // 5. OTHER

    // Check all the SAX handlers?

    // Must check the output JSON against the input JSON with nothing turned on.

    // Check isWithinEntity...
    //
    
        // Setup the quad.
        Geo::Point sw{ 42.17, -83.91 };
        Geo::Point ne{ 42.431, -83.54 };

        // use the plymouth_rd.data file for testing.


}
*/
