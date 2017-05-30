/** 
 * @file 
 * @author   Jason M. Carter (carterjm@ornl.gov)
 * @author   Aaron E. Ferber (ferberae@ornl.gov)
 * @date     April 2017
 * @version  0.1
 *
 * @copyright Copyright 2017 US DOT - Joint Program Office
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contributors:
 *    Oak Ridge National Laboratory, Center for Trustworthy Embedded Systems, UT Battelle.
 *
 * librdkafka - Apache Kafka C library
 *
 * Copyright (c) 2014, Magnus Edenhill
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "ppm.hpp"
#include <csignal>
#include <chrono>
#include <thread>

#ifndef _MSC_VER
#include <sys/time.h>
#endif

#ifdef _MSC_VER
#include "../win32/wingetopt.h"
#include <atltime.h>
#elif _AIX
#include <unistd.h>
#else
#include <getopt.h>
#include <unistd.h>
#endif

bool PPM::bsms_available = true;

void PPM::sigterm (int sig) {
    bsms_available = false;
}

PPM::PPM( const std::string& name, const std::string& description ) :
    Tool{ name, description },
    exit_eof{true},
    eof_cnt{0},
    partition_cnt{1},
    bsm_recv_count{0},
    bsm_send_count{0},
    bsm_filt_count{0},
    bsm_recv_bytes{0},
    bsm_send_bytes{0},
    bsm_filt_bytes{0},
    verbosity{0},
    pconf{},
    brokers{"localhost"},
    partition{RdKafka::Topic::PARTITION_UA},
    mode{""},
    debug{""},
    consumed_topics{},
    offset{RdKafka::Topic::OFFSET_BEGINNING},
    published_topic{},
    conf{nullptr},
    tconf{nullptr},
    qptr{},
    consumer{},
    consumer_timeout{500},
    producer{},
    filtered_topic{}
{
}

PPM::~PPM() 
{
    consumer->close();

    // free raw librdkafka pointers.
    if (tconf) delete tconf;
    if (conf) delete conf;

    // TODO: This librdkafka item seems wrong...
    RdKafka::wait_destroyed(5000);    // pause to let RdKafka reclaim resources.
}

void PPM::metadata_print (const std::string &topic, const RdKafka::Metadata *metadata) {

    std::cout << "Metadata for " << (topic.empty() ? "" : "all topics")
        << "(from broker "  << metadata->orig_broker_id()
        << ":" << metadata->orig_broker_name() << std::endl;

    /* Iterate brokers */
    std::cout << " " << metadata->brokers()->size() << " brokers:" << std::endl;

    for ( auto ib : *(metadata->brokers()) ) {
        std::cout << "  broker " << ib->id() << " at " << ib->host() << ":" << ib->port() << std::endl;
    }

    /* Iterate topics */
    std::cout << metadata->topics()->size() << " topics:" << std::endl;

    for ( auto& it : *(metadata->topics()) ) {

        std::cout << "  topic \""<< it->topic() << "\" with " << it->partitions()->size() << " partitions:";

        if (it->err() != RdKafka::ERR_NO_ERROR) {
            std::cout << " " << err2str(it->err());
            if (it->err() == RdKafka::ERR_LEADER_NOT_AVAILABLE) std::cout << " (try again)";
        }

        std::cout << std::endl;

        /* Iterate topic's partitions */
        for ( auto& ip : *(it->partitions()) ) {
            std::cout << "    partition " << ip->id() << ", leader " << ip->leader() << ", replicas: ";

            /* Iterate partition's replicas */
            RdKafka::PartitionMetadata::ReplicasIterator ir;
            for (ir = ip->replicas()->begin(); ir != ip->replicas()->end(); ++ir) {
                std::cout << (ir == ip->replicas()->begin() ? "":",") << *ir;
            }

            /* Iterate partition's ISRs */
            std::cout << ", isrs: ";
            RdKafka::PartitionMetadata::ISRSIterator iis;
            for (iis = ip->isrs()->begin(); iis != ip->isrs()->end() ; ++iis)
                std::cout << (iis == ip->isrs()->begin() ? "":",") << *iis;

            if (ip->err() != RdKafka::ERR_NO_ERROR)
                std::cout << ", " << RdKafka::err2str(ip->err()) << std::endl;
            else
                std::cout << std::endl;
        }
    }
}

bool PPM::topic_available( const std::string& topic ) {
    bool r = false;

    RdKafka::Metadata* md;
    RdKafka::ErrorCode err = consumer->metadata( true, nullptr, &md, 5000 );
    // TODO: Will throw a broker transport error (ERR__TRANSPORT = -195) if the broker is not available.

    if ( err == RdKafka::ERR_NO_ERROR ) {
        RdKafka::Metadata::TopicMetadataIterator it = md->topics()->begin();

        // search for the raw BSM topic.
        while ( it != md->topics()->end() && !r ) {
            // finish when we find it.
            r = ( (*it)->topic() == topic );
            ++it;
        }
    }
    
    return r;
}

void PPM::print_configuration() const
{
    std::cout << "# Global config" << "\n";
    std::list<std::string>* conf_list = conf->dump();

    int i = 0;
    for ( auto& v : *conf_list ) {
        if ( i%2==0 ) std::cout << v << " = ";
        else std::cout << v << '\n';
        ++i;
    }

    std::cout << "# Topic config" << "\n";
    conf_list = tconf->dump();
    i = 0;
    for ( auto& v : *conf_list ) {
        if ( i%2==0 ) std::cout << v << " = ";
        else std::cout << v << '\n';
        ++i;
    }

    std::cout << "# Privacy config \n";
    for ( const auto& m : pconf ) {
        std::cout << m.first << " = " << m.second << '\n';
    }
}

bool PPM::configure() {

    std::string line;
    std::string error_string;
    StrVector pieces;

    // configurations; global and topic (the names in these are fixed)
    conf  = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    // must use a configuration file.
    if ( !optIsSet('c') ) return false;

    const std::string& cfile = optString('c');              // needed for error message.
    std::ifstream ifs{ cfile };

    if (!ifs) {
        std::cerr << "cannot open configuration file: " << cfile << "\n";
        return false;
    }

    while (std::getline( ifs, line )) {
        line = string_utilities::strip( line );
        if ( !line.empty() && line[0] != '#' ) {
            pieces = string_utilities::split( line, '=' );
            bool done = false;
            if (pieces.size() == 2) {
                // in case the user inserted some spaces...
                string_utilities::strip( pieces[0] );
                string_utilities::strip( pieces[1] );
                // some of these configurations are stored in each...?? strange.
                if ( tconf->set(pieces[0], pieces[1], error_string) == RdKafka::Conf::CONF_OK ) {
                    //std::cout << "TOPIC CONF: " << pieces[0] << " = " << pieces[1] << '\n';
                    done = true;
                }

                if ( conf->set(pieces[0], pieces[1], error_string) == RdKafka::Conf::CONF_OK ) {
                    //std::cout << "CONF: " << pieces[0] << " = " << pieces[1] << '\n';
                    done = true;
                }

                if ( !done ) { 
                    // These configuration options are not expected by Kafka.
                    // Assume there are for the PPM.
                    pconf[ pieces[0] ] = pieces[1];
                }
            }
        } // otherwise: empty or comment line.
    }

    // All configuration file settings are overridden, if supplied, by CLI options.

    // fail first on mapfile.
    std::string mapfile;

    if ( optIsSet('m') ) {
        // map file is specified on command line.
        mapfile = optString('m');

    } else {
        auto search = pconf.find("privacy.filter.geofence.mapfile");
        if ( search != pconf.end() ) {
            mapfile = search->second;
        } else {
            std::cerr << "No map file specified; must fail.\n";
            return false;
        }
    }

    qptr = BuildGeofence( mapfile );                // throws.

    if ( optIsSet('b') ) {
        // broker specified.
        conf->set("metadata.broker.list", optString('b'), error_string);
    } 

    if ( optIsSet('p') ) {
        // number of partitions.
        partition = optInt( 'p' );

    } else {
        auto search = pconf.find("privacy.kafka.partition");
        if ( search != pconf.end() ) {
            partition = std::stoi(search->second);              // throws.    
        }  // otherwise leave at default; PARTITION_UA
    }

    if ( getOption('g').isSet() && conf->set("group.id", optString('g'), error_string) != RdKafka::Conf::CONF_OK) {
        // NOTE: there are some checks in librdkafka that require this to be present and set.
        std::cerr << error_string << "\n";
        return false;
    }

    if ( getOption('o').isSet() ) {
        // offset in the consumed stream.
        std::string o = optString( 'o' );

        if (o=="end") {
            offset = RdKafka::Topic::OFFSET_END;
        } else if (o=="beginning") {
            offset = RdKafka::Topic::OFFSET_BEGINNING;
        } else if (o== "stored") {
            offset = RdKafka::Topic::OFFSET_STORED;
        } else {
            offset = strtoll(o.c_str(), NULL, 10);              // throws
        }
    }

    // Do we want to exit if a stream eof is sent.
    exit_eof = getOption('e').isSet();

    if ( optIsSet('v') ) {
        // verbosity of messages is set.
        // TODO: Implement this with logging.
        verbosity = optInt('v');
    }

    if (optIsSet('d') && conf->set("debug", optString('d'), error_string) != RdKafka::Conf::CONF_OK) {
        // debug information. TODO: Implement this will logging story.
        std::cerr << error_string << "\n";
        return false;
    }

    // librdkafka defined configuration.
    conf->set("default_topic_conf", tconf, error_string);

    auto search = pconf.find("privacy.topic.consumer");
    if ( search != pconf.end() ) {
        consumed_topics.push_back( search->second );

    } else {
        
        std::cerr << "No consumer topic was specified; must fail.\n";
        return false;
    }

    if (optIsSet('t')) {
        // this is the produced (filtered) topic.
        published_topic = optString( 't' );

    } else {
        // maybe it was specified in the configuration file.
        auto search = pconf.find("privacy.topic.producer");
        if ( search != pconf.end() ) {
            published_topic = search->second;
        } else {
            std::cerr << "No publisher topic was specified; must fail.\n";
            return false;
        }
    }

    search = pconf.find("privacy.consumer.timeout.ms");
    if ( search != pconf.end() ) {
        try {
            consumer_timeout = std::stoi( search->second );
        } catch( std::exception& e ) {
            // use the default.
        }
    }

    return true;
}

RdKafka::ErrorCode PPM::msg_consume(RdKafka::Message* message, void* opaque, BSMHandler& handler) {

    // payload is a void *
    // len is a size_t
    std::string payload(static_cast<const char*>(message->payload()), message->len());

    switch (message->err()) {
        case RdKafka::ERR__TIMED_OUT:
            break;

        case RdKafka::ERR_NO_ERROR:
            /* Real message */
            bsm_recv_count++;
            bsm_recv_bytes += message->len();

            if (verbosity >= 3) {
                std::cerr << "Read msg at offset " << message->offset() << "\n";
            }

            RdKafka::MessageTimestamp ts;
            ts = message->timestamp();

            if (verbosity >= 2 && ts.type != RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE) {
                std::string tsname = "?";

                if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME) {
                    tsname = "create time";
                } else if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_LOG_APPEND_TIME) {
                    tsname = "log append time";
                }

                std::cout << "Timestamp: " << tsname << " " << ts.timestamp << "\n";
            }

            if (verbosity >= 2 && message->key()) {
                std::cout << "Key: " << *message->key() << "\n";
            }

            // Process the BSM payload.
            if ( handler.process( payload ) ) {
                return RdKafka::ERR_NO_ERROR;
            } else {
                return RdKafka::ERR_INVALID_MSG;
            }

            break;

        case RdKafka::ERR__PARTITION_EOF:
            /* Last message */
            if (exit_eof && (++eof_cnt == partition_cnt)) {
                std::cerr << "%% EOF reached for all " << partition_cnt << " partition(s)" << "\n";
                bsms_available = false;
            }
            break;

        case RdKafka::ERR__UNKNOWN_TOPIC:
            std::cerr << "Consume failed due to an UNKNOWN consumer topic: " << message->errstr() << "\n";
            bsms_available = false;
            break;

        case RdKafka::ERR__UNKNOWN_PARTITION:
            std::cerr << "Consume failed due to an UNKNOWN consumer partition: " << message->errstr() << "\n";
            bsms_available = false;
            break;

        default:
            /* Errors */
            std::cerr << "Consume failed: " << message->errstr() << "\n";
            bsms_available = false;
    }

    return message->err();
}

Quad::Ptr PPM::BuildGeofence( const std::string& mapfile )  // throws
{
    geo::Point sw, ne;

    auto search = pconf.find("privacy.filter.geofence.sw.lat");
    if ( search != pconf.end() ) {
        sw.lat = std::stod(search->second);
    }

    search = pconf.find("privacy.filter.geofence.sw.lon");
    if ( search != pconf.end() ) {
        sw.lon = std::stod(search->second);
    }

    search = pconf.find("privacy.filter.geofence.ne.lat");
    if ( search != pconf.end() ) {
        ne.lat = std::stod(search->second);
    }

    search = pconf.find("privacy.filter.geofence.ne.lon");
    if ( search != pconf.end() ) {
        ne.lon = std::stod(search->second);
    }

    Quad::Ptr qptr = std::make_shared<Quad>(sw, ne);

    // Read the file and parse the shapes.
    shapes::CSVInputFactory shape_factory( mapfile );
    shape_factory.make_shapes();

    // Add all the shapes to the quad.
    // NOTE: we are only using Edges right now.
    for (auto& circle_ptr : shape_factory.get_circles()) {
        Quad::insert(qptr, std::dynamic_pointer_cast<const geo::Entity>(circle_ptr)); 
    }

    for (auto& edge_ptr : shape_factory.get_edges()) {
        Quad::insert(qptr, std::dynamic_pointer_cast<const geo::Entity>(edge_ptr)); 
    }

    for (auto& grid_ptr : shape_factory.get_grids()) {
        Quad::insert(qptr, std::dynamic_pointer_cast<const geo::Entity>(grid_ptr)); 
    }

    return qptr;
}

bool PPM::launch_producer()
{
    std::string error_string;

    producer = std::shared_ptr<RdKafka::Producer>( RdKafka::Producer::create(conf, error_string) );
    if (!producer) {
        std::cerr << "Failed to create producer: " << error_string << "\n";
        return false;
    }

    filtered_topic = std::shared_ptr<RdKafka::Topic>( RdKafka::Topic::create(producer.get(), published_topic, tconf, error_string) );
    if ( !filtered_topic ) {
        std::cerr << "Failed to create topic: " << error_string << "\n";
        return false;
    } 

    std::cout << ">> Created Producer: " << producer->name() << " using topic: " << published_topic << "\n";
    return true;
}

bool PPM::launch_consumer()
{
    std::string error_string;

    consumer = std::shared_ptr<RdKafka::KafkaConsumer>( RdKafka::KafkaConsumer::create(conf, error_string) );
    if (!consumer) {
        std::cerr << "Failed to create consumer: " << error_string << "\n";
        return false;
    }

    // wait on the topics we specified to become available for subscription.
    // loop terminates with a signal (CTRL-C) or when all the topics are available.
    int tcount = 0;
    for ( auto& topic : consumed_topics ) {
        while ( bsms_available && tcount < consumed_topics.size() ) {
            if ( topic_available(topic) ) {
                // count it and attempt to get the next one if it exists.
                ++tcount;
                break;
            }
            // topic is not available, wait for a second or two.
            std::this_thread::sleep_for( std::chrono::milliseconds( 1500 ) );
            std::cerr << "Waiting for topic: " << topic << " to become available.\n";
        }
    }

    if ( tcount == consumed_topics.size() ) {
        // all the needed topics are available for subscription.
        RdKafka::ErrorCode status = consumer->subscribe(consumed_topics);
        if (status) {
            std::cerr << "Failed to subscribe to " << consumed_topics.size() << " topics: " << RdKafka::err2str(status) << "\n";
            return false;
        }
    } else {
        return false;
    }

    std::cout << ">> Created Consumer: " << consumer->name() << " using topic(s): ";
    for ( auto& topic : consumed_topics ) 
        std::cout << topic << ", ";
    std::cout << std::endl;

    return true;
}


int PPM::operator()(void) {

    std::string error_string;
    RdKafka::ErrorCode status;
    bool pulse = false;

    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);

    try {

        // throws for mapfile and other items.
        if ( !configure() ) return EXIT_FAILURE;

    } catch ( std::exception& e ) {

        std::cerr << "Fatal Exception: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    if ( !launch_consumer() ) return false;
    if ( !launch_producer() ) return false;

    BSMHandler handler{qptr, pconf};

    // consume-produce loop.
    while (bsms_available) {

        std::unique_ptr<RdKafka::Message> msg{ consumer->consume( consumer_timeout ) };
        status = msg_consume(msg.get(), NULL, handler);

        if ( status == RdKafka::ERR_NO_ERROR ) {
            // Retain BSM.
            const BSM& bsm = handler.get_bsm();
            if (pulse) {
                std::cerr << "\n";
                pulse = false;
            }
            std::cout << "Retaining BSM: " << bsm << "\n";

            status = producer->produce(filtered_topic.get(), partition, RdKafka::Producer::RK_MSG_COPY, (void *)handler.get_json().c_str(), handler.get_bsm_buffer_size(), NULL, NULL);
            if (status != RdKafka::ERR_NO_ERROR) {
                if (pulse) {
                    std::cerr << "\n";
                    pulse = false;
                }
                std::cerr << "% Produce failed: " << RdKafka::err2str( status ) << "\n";
            } else {

                // successfully sent; update counters.
                bsm_send_count++;
                bsm_send_bytes += msg->len();
            }

        } else if ( status == RdKafka::ERR_INVALID_MSG ) {
            // Filter BSM.
            const BSM& bsm = handler.get_bsm();
            if (pulse) {
                std::cerr << "\n";
                pulse = false;
            }
            std::cout << "Filtering BSM [" << handler.get_result_string() << "] : " << bsm << "\n";
            
            // filtered; update counters.
            bsm_filt_count++;
            bsm_filt_bytes += msg->len();

        } else if ( status == RdKafka::ERR__TIMED_OUT ) {

            if ( !pulse ) {
                std::cerr << "ODE BSM Consumer Waiting: .";
            } else {
                std::cerr << ".";
            }

            pulse = true;

        } else if ( status == RdKafka::ERR__PARTITION_EOF ) {

            if (pulse) std::cerr << "\n";
            std::cerr << "ODE BSM Consumer: partition end of file, but PPM still alive.\n";
            pulse = false;

        } else {

            if (pulse) std::cerr << "\n";
            std::cerr << "% Consumer Problem: " << RdKafka::err2str( status ) << '\n';
            pulse = false;
        }
    }

    std::cout << "\nPPM Operations Complete.\n";
    std::cout << "Consumption\n";
    std::cout << "\t" << bsm_recv_count << " BSMs (" << bsm_recv_bytes << ").\n";
    std::cout << "Publishing\n";
    std::cout << "\t" << bsm_send_count << " BSMs (" << bsm_send_bytes << ")\n";
    std::cout << "Suppression\n";
    std::cout << "\t" << bsm_filt_count << " BSMs (" << bsm_filt_bytes << ").\n";

    return EXIT_SUCCESS;
}

#ifndef _PPM_TESTS

int main( int argc, char* argv[] )
{
    PPM ppm{"ppm","Privacy Protection Module"};

    ppm.addOption( 'c', "config", "Configuration for Kafka and Privacy Protection Module.", true );
    ppm.addOption( 'C', "config-check", "Check the configuration and output the settings.", false );
    ppm.addOption( 't', "produce-topic", "topic.", true );
    ppm.addOption( 'p', "partition", "Consumer topic partition from which to read.", true );
    ppm.addOption( 'g', "group", "Consumer group identifier", true );
    ppm.addOption( 'b', "broker", "Broker address (localhost:9092)", true );
    ppm.addOption( 'o', "offset", "Byte offset to start reading in the consumed topic.", true );
    ppm.addOption( 'e', "exit", "Exit consumer when last message in partition has been received.", false );
    ppm.addOption( 'd', "debug", "debug level.", true );
    ppm.addOption( 'm', "mapfile", "Map data file to specify the geofence.", true );
    ppm.addOption( 'v', "verbose", "verbosity level.", true );
    ppm.addOption( 'h', "help", "print out some help" );

    if (!ppm.parseArgs(argc, argv)) {
        ppm.usage();
        std::exit( EXIT_FAILURE );
    }

    if (ppm.optIsSet('h')) {
        ppm.help();
        std::exit( EXIT_SUCCESS );
    }

    // configuration check.
    if (ppm.optIsSet('C')) {
        try {
            if (ppm.configure()) {
                ppm.print_configuration();
                std::exit( EXIT_SUCCESS );
            } else {
                std::cerr << "Current configuration settings do not work.\n";
                std::exit( EXIT_FAILURE );
            }
        } catch ( std::exception& e ) {
            std::cerr << "Fatal Exception: " << e.what() << '\n';
            std::exit( EXIT_FAILURE );
        }
    }

    std::exit( ppm.run() );
}

#endif
