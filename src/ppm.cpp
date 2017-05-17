/** 
 * @file 
 * @author   Jason M. Carter (carterjm@ornl.gov)
 * @author   Aaron E. Ferber (ferberae@ornl.gov)
 * @date     April 2017
 * @version  0.1
 *
 * @copyright Copyright 2017 US DOT - Joint Program Office
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#include <librdkafka/rdkafkacpp.h>
#include <csignal>

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

#include "tool.hpp"
#include "bsmfilter.hpp"
#include "cvlib.hpp"

class PPM : public tool::Tool {

    public:

        static void sigterm (int sig) {
            bsms_available = false;
        }

        PPM( const std::string& name, const std::string& description ) :
            Tool{ name, description },
            exit_eof{true},
            eof_cnt{0},
            partition_cnt{0},
            msg_cnt{0},
            msg_bytes{0},
            verbosity{0},
            pconf{},
            brokers{"localhost"},
            partition{RdKafka::Topic::PARTITION_UA},
            mode{""},
            debug{""},
            region_file{""},
            topics{},
            offset{RdKafka::Topic::OFFSET_BEGINNING},
            conf{nullptr},
            tconf{nullptr},
            qptr{},
            consumer{},
            producer{},
            filtered_topic{}
        {
        }

        ~PPM() 
        {
            consumer->close();

            if (tconf) delete tconf;
            if (conf) delete conf;

            // This seems wrong...
            RdKafka::wait_destroyed(5000);    // pause to let RdKafka reclaim resources.
        }

        /**
         *
         */
        void metadata_print (const std::string &topic, const RdKafka::Metadata *metadata) {

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

        bool ode_topic_available( const std::string& topic ) {
            bool r = false;

            RdKafka::Metadata* md;

            RdKafka::ErrorCode err = consumer->metadata( true, nullptr, &md, 5000 );
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

        void print_configuration() const
        {
            std::cout << "# Global config" << "\n";
            for (std::list<std::string>::iterator it = conf->dump()->begin(); it != conf->dump()->end(); ) {
                std::cout << *it << " = ";
                it++;
                std::cout << *it << "\n";
                it++;
            }

            std::cout << "# Topic config" << "\n";
            for (std::list<std::string>::iterator it = tconf->dump()->begin(); it != tconf->dump()->end(); ) {
                std::cout << *it << " = ";
                it++;
                std::cout << *it << "\n";
                it++;
            }

            std::cout << "# Privacy config \n";
            for ( const auto& m : pconf ) {
                std::cout << m.first << " = " << m.second << '\n';
            }
        }

        /**
         *
         */
        bool configure() {

            std::string line;
            std::string error_string;
            StrVector pieces;

            // configurations; global and topic (the names in these are fixed)
            conf  = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
            tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

            if ( !optIsSet('c') ) return false;

            const std::string& cfile = optString('c');
            std::ifstream ifs{ cfile };

            if (!ifs) {
                std::cerr << "cannot open configuration file: " << cfile << "\n";
                return false;
            }

            while (std::getline( ifs, line )) {
                line = string_utilities::strip( line );
                if ( !line.empty() && line[0] != '#' ) {
                    pieces = string_utilities::split( line, '=' );
                    if (pieces.size() == 2) {
                        if ( conf->set(pieces[0], pieces[1], error_string) != RdKafka::Conf::CONF_OK ) {
                            // These configuration options are not expected by Kafka.
                            // Assume there are for the PPM.
                            pconf[ pieces[0] ] = pieces[1];
                        }
                    }
                }
            }

            if ( optIsSet('b') ) {
                // Overrides what is in the configuration file.
                conf->set("metadata.broker.list", optString('b'), error_string);
            } 

            if ( optIsSet('p') ) {
                partition = optInt( 'p' );

            } else {
                auto search = pconf.find("privacy.kafka.partition");
                if ( search != pconf.end() ) {
                    partition = std::stoi(search->second);
                }  // otherwise leave at default; PARTITION_UA
            }

            if ( getOption('g').isSet() && conf->set("group.id", optString('g'), error_string) != RdKafka::Conf::CONF_OK) {
                std::cerr << error_string << "\n";
                return false;
            }

            if ( getOption('o').isSet() ) {
                std::string o = optString( 'o' );

                if (o=="end") {
                    offset = RdKafka::Topic::OFFSET_END;
                } else if (o=="beginning") {
                    offset = RdKafka::Topic::OFFSET_BEGINNING;
                } else if (o== "stored") {
                    offset = RdKafka::Topic::OFFSET_STORED;
                } else {
                    offset = strtoll(o.c_str(), NULL, 10);
                }
            }

            exit_eof = getOption('e').isSet();

            if ( optIsSet('m') ) {
                region_file = optString('m');
            }

            if ( optIsSet('v') ) {
                verbosity = optInt('v');
            }

            if (optIsSet('d') && conf->set("debug", optString('d'), error_string) != RdKafka::Conf::CONF_OK) {
                // TODO: a way to do this in the config file.
                std::cerr << error_string << "\n";
                return false;
            }

            // librdkafka defined configuration.
            conf->set("default_topic_conf", tconf, error_string);

            // TODO: the below should happen in the configure method.
            if ( optIsSet('m') ) {
                region_file = optString('m');

            } else {
                auto search = pconf.find("privacy.filter.geofence.mapfile");
                if ( search != pconf.end() ) {
                    region_file = search->second;
                } else {
                    std::cerr << "No map file specified.\n";
                    return false;
                }
            }

            return true;
        }

        /**
         * @brief
         */
        RdKafka::ErrorCode msg_consume(RdKafka::Message* message, void* opaque, BSMHandler& handler) {

            // payload is a void *
            // len is a size_t
            std::string payload(static_cast<const char*>(message->payload()), message->len());

            switch (message->err()) {
                case RdKafka::ERR__TIMED_OUT:
                    break;

                case RdKafka::ERR_NO_ERROR:
                    /* Real message */
                    msg_cnt++;
                    msg_bytes += message->len();
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

                    if ( handler.process( payload ) ) {
                        return RdKafka::ERR_NO_ERROR;
                    } else {
                        return RdKafka::ERR_INVALID_MSG;
                    }

                    break;

                case RdKafka::ERR__PARTITION_EOF:
                    /* Last message */
                    if (exit_eof && ++eof_cnt == partition_cnt) {
                        std::cerr << "%% EOF reached for all " << partition_cnt << " partition(s)" << "\n";
                        bsms_available = false;
                    }
                    break;

                case RdKafka::ERR__UNKNOWN_TOPIC:
                case RdKafka::ERR__UNKNOWN_PARTITION:
                    std::cerr << "Consume failed: " << message->errstr() << "\n";
                    bsms_available = false;
                    break;

                default:
                    /* Errors */
                    std::cerr << "Consume failed: " << message->errstr() << "\n";
                    bsms_available = false;
            }

            return message->err();
        }

        Quad::Ptr BuildGeofence()  // throws
        {
            Geo::Point sw, ne;

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

            // Declare a quad with the given bounds.
            Quad::Ptr qptr = std::make_shared<Quad>(sw, ne);

            // Read the file and parse the shapes.
            Shapes::CSVInputFactory shape_factory(region_file);
            shape_factory.make_shapes();
            // Add all the shapes to the quad.
            for (auto& circle_ptr : shape_factory.get_circles()) {
                Quad::insert(qptr, std::dynamic_pointer_cast<const Geo::Entity>(circle_ptr)); 
            }

            for (auto& edge_ptr : shape_factory.get_edges()) {
                Quad::insert(qptr, std::dynamic_pointer_cast<const Geo::Entity>(edge_ptr)); 
            }

            for (auto& grid_ptr : shape_factory.get_grids()) {
                Quad::insert(qptr, std::dynamic_pointer_cast<const Geo::Entity>(grid_ptr)); 
            }

            return qptr;
        }

        /**
         * Runner function.
         */
        int operator()(void) {

            std::string error_string;
            RdKafka::ErrorCode status;

            // may need to catch.
            if ( !configure() ) return false;

            signal(SIGINT, sigterm);
            signal(SIGTERM, sigterm);

            try {

                qptr = BuildGeofence();

            } catch ( std::exception& e ) {
                std::cerr << "Problem building geofence: " << e.what() << '\n';
                std::exit(EXIT_FAILURE);
            }

            BSMHandler handler{qptr, pconf};
            consumer = std::shared_ptr<RdKafka::KafkaConsumer>( RdKafka::KafkaConsumer::create(conf, error_string) );

            if (!consumer) {
                std::cerr << "Failed to create consumer: " << error_string << "\n";
                std::exit(EXIT_FAILURE);
            }

            std::cout << ">> Created Consumer: " << consumer->name() << "\n";

            // grab the topic off the configuration.
            auto search = pconf.find("privacy.topic.consumer");
            if ( search != pconf.end() ) {
                topics.push_back( search->second );
            } else {
                std::cerr << "Failure to use configured consumer topic: " << error_string << "\n";
                std::exit(EXIT_FAILURE);
            }

            for ( const std::string& topic : topics ) {
                if ( !ode_topic_available( topic )) {
                    std::cerr << "The ODE Topic: " << topic << " is not available. This topic must be readable.\n";
                    std::exit(EXIT_FAILURE);
                }
            }

            // subscribe to the J2735BsmJson topic (or test)
            status = consumer->subscribe(topics);
            if (status) {
                std::cerr << "Failed to subscribe to " << topics.size() << " topics: " << RdKafka::err2str(status) << "\n";
                std::exit(EXIT_FAILURE);
            }

            // Producer setup: will take the filtered BSMs and send them back to the ODE (or a test java consumer).
            producer = std::shared_ptr<RdKafka::Producer>( RdKafka::Producer::create(conf, error_string) );
            if (!producer) {
                std::cerr << "Failed to create producer: " << error_string << "\n";
                std::exit(EXIT_FAILURE);
            }

            std::cout << ">> Created Producer: " << producer->name() << "\n";

            std::string topic_str;
            if (optIsSet('t')) {
                    // this is the produced (filtered) topic.
                    topic_str = optString( 't' );

            } else {
                // maybe it was specified in the configuration file.
                auto search = pconf.find("privacy.topic.producer");
                if ( search != pconf.end() ) {
                    topic_str = search->second;
                } else {
                    std::cerr << "Topic String Empty!\n";
                    std::exit(EXIT_FAILURE);
                }
            }

            filtered_topic = std::shared_ptr<RdKafka::Topic>( RdKafka::Topic::create(producer.get(), topic_str, tconf, error_string) );
            if ( !filtered_topic ) {
                std::cerr << "Failed to create topic: " << error_string << "\n";
                return false;
            } 

            // consume-produce loop.
            while (bsms_available) {

                std::unique_ptr<RdKafka::Message> msg{ consumer->consume(1000) };
                //RdKafka::Message *msg = consumer->consume(1000);
                status = msg_consume(msg.get(), NULL, handler);

                if ( status == RdKafka::ERR_NO_ERROR ) {
                    const BSM& bsm = handler.get_bsm();
                    std::cout << "Retaining BSM: " << bsm << "\n";

                    // if we still have a message in the handler, we send it back out to the producer we have made above.
                    status = producer->produce(filtered_topic.get(), partition, RdKafka::Producer::RK_MSG_COPY, (void *)handler.get_json().c_str(), handler.get_bsm_buffer_size(), NULL, NULL);
                    //RdKafka::ErrorCode resp = producer->produce(topic, partition, RdKafka::Producer::RK_MSG_COPY, (void *)handler.get_json().c_str(), handler.get_bsm_buffer_size(), NULL, NULL);
                    if (status != RdKafka::ERR_NO_ERROR) {
                        std::cerr << "% Produce failed: " << RdKafka::err2str( status ) << "\n";
                    } 

                } else if ( status == RdKafka::ERR_INVALID_MSG ) {
                    const BSM& bsm = handler.get_bsm();
                    std::cout << "Filtering BSM [" << handler.get_result_string() << "] : " << bsm << "\n";

                }
            }

            std::cout << "\nFinished Consuming BSMs.\n";
            std::cout << "Consumed " << msg_cnt << " messages (" << msg_bytes << " bytes)" << '\n';

            return true;
        }

    private:

        static bool bsms_available;

        bool exit_eof;

        int eof_cnt;
        int partition_cnt;
        long msg_cnt;
        int64_t msg_bytes;

        int verbosity;

        std::string brokers;
        int32_t partition;
        std::string mode;
        std::string debug;
        int64_t offset;

        // configurations; global and topic (the names in these are fixed)
        std::unordered_map<std::string, std::string> pconf;
        RdKafka::Conf *conf;
        RdKafka::Conf *tconf;

        std::string region_file;                                        ///> map data for geofence.
        Quad::Ptr qptr;

        std::vector<std::string> topics;                                ///> consumer topics.
        std::shared_ptr<RdKafka::KafkaConsumer> consumer;
        std::shared_ptr<RdKafka::Producer> producer;
        std::shared_ptr<RdKafka::Topic> filtered_topic;
};

bool PPM::bsms_available = true;

#ifndef _PPM_TESTS

int main( int argc, char* argv[] )
{
    PPM ppm{"ppm","Privacy Protection Module"};

    ppm.addOption( 'c', "config", "Configuration for Kafka and Privacy Protection Module.", true );
    ppm.addOption( 't', "topic", "topic.", true );
    ppm.addOption( 'p', "partition", "topic partition from which to consume.", true );
    ppm.addOption( 'g', "group", "Consumer group identifier", true );
    ppm.addOption( 'b', "broker", "Broker address (localhost:9092)", true );
    ppm.addOption( 'o', "offset", "starting read offset in the partition.", true );
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

    std::exit( ppm.run() );
}

#endif
