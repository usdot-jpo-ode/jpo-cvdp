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

#include <librdkafka/rdkafkacpp.h>
#include <unordered_map>
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

#include "bsmfilter.hpp"
#include "cvlib.hpp"

static bool run = true;
static bool exit_eof = false;

static int eof_cnt = 0;
static int partition_cnt = 0;
static long msg_cnt = 0;
static int64_t msg_bytes = 0;

static int verbosity = 1;

static void sigterm (int sig) {
  run = false;
}

// JMC: Useful librdkafka reference code for metadata.
//static void metadata_print (const std::string &topic, const RdKafka::Metadata *metadata) {
//
//    std::cout << "Metadata for " << (topic.empty() ? "" : "all topics")
//        << "(from broker "  << metadata->orig_broker_id()
//        << ":" << metadata->orig_broker_name() << std::endl;
//
//    /* Iterate brokers */
//    std::cout << " " << metadata->brokers()->size() << " brokers:" << std::endl;
//
//    for ( auto ib : *(metadata->brokers()) ) {
//        std::cout << "  broker " << ib->id() << " at " << ib->host() << ":" << ib->port() << std::endl;
//    }
//
//    /* Iterate topics */
//    std::cout << metadata->topics()->size() << " topics:" << std::endl;
//
//    for ( auto& it : *(metadata->topics()) ) {
//
//        std::cout << "  topic \""<< it->topic() << "\" with " << it->partitions()->size() << " partitions:";
//
//        if (it->err() != RdKafka::ERR_NO_ERROR) {
//            std::cout << " " << err2str(it->err());
//            if (it->err() == RdKafka::ERR_LEADER_NOT_AVAILABLE) std::cout << " (try again)";
//        }
//
//        std::cout << std::endl;
//
//        /* Iterate topic's partitions */
//        for ( auto& ip : *(it->partitions()) ) {
//            std::cout << "    partition " << ip->id() << ", leader " << ip->leader() << ", replicas: ";
//
//            /* Iterate partition's replicas */
//            RdKafka::PartitionMetadata::ReplicasIterator ir;
//            for (ir = ip->replicas()->begin(); ir != ip->replicas()->end(); ++ir) {
//                std::cout << (ir == ip->replicas()->begin() ? "":",") << *ir;
//            }
//
//            /* Iterate partition's ISRs */
//            std::cout << ", isrs: ";
//            RdKafka::PartitionMetadata::ISRSIterator iis;
//            for (iis = ip->isrs()->begin(); iis != ip->isrs()->end() ; ++iis)
//                std::cout << (iis == ip->isrs()->begin() ? "":",") << *iis;
//
//            if (ip->err() != RdKafka::ERR_NO_ERROR)
//                std::cout << ", " << RdKafka::err2str(ip->err()) << std::endl;
//            else
//                std::cout << std::endl;
//        }
//    }
//}

static bool ode_topic_available( const std::string& topic, std::shared_ptr<RdKafka::KafkaConsumer> consumer ) {
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
                run = false;
            }
            break;

        case RdKafka::ERR__UNKNOWN_TOPIC:
        case RdKafka::ERR__UNKNOWN_PARTITION:
            std::cerr << "Consume failed: " << message->errstr() << "\n";
            run = false;
            break;

        default:
            /* Errors */
            std::cerr << "Consume failed: " << message->errstr() << "\n";
            run = false;
    }

    return message->err();
}

/* Use of this partitioner is pretty pointless since no key is provided
 * in the produce() call. */
class MyHashPartitionerCb : public RdKafka::PartitionerCb {
    public:
        int32_t partitioner_cb (const RdKafka::Topic *topic, const std::string *key,
                int32_t partition_cnt, void *msg_opaque) {
            return djb_hash(key->c_str(), key->size()) % partition_cnt;
        }
    private:

        static inline unsigned int djb_hash (const char *str, size_t len) {
            unsigned int hash = 5381;
            for (size_t i = 0 ; i < len ; i++)
                hash = ((hash << 5) + hash) + str[i];
            return hash;
        }
};

/**
 * NOTE: This is supposed to be a little more efficient.
 */
class ExampleConsumeCb : public RdKafka::ConsumeCb {
    public:
        void consume_cb (RdKafka::Message &msg, void *opaque) {
            //msg_consume(&msg, opaque);
        }
};

bool configure( const std::string& config_file, std::unordered_map<std::string,std::string>& pconf, RdKafka::Conf *conf, RdKafka::Conf *tconf ) {

    std::string line;
    std::string errstr;
    std::vector<std::string> pieces;
    std::ifstream ifs( config_file );

    if (!ifs ) {
        std::cerr << "cannot open configuration file: " << config_file << "\n";
        return false;
    }

    while (std::getline( ifs, line )) {
        line = string_utilities::strip( line );
        if ( !line.empty() && line[0] != '#' ) {
            pieces = string_utilities::split( line, '=' );
            if (pieces.size() == 2) {
                if ( conf->set(pieces[0], pieces[1], errstr) != RdKafka::Conf::CONF_OK ) {
                    pconf[ pieces[0] ] = pieces[1];
                }
            }
        }
    }
    return true;
}

#ifndef _PPM_TESTS

int main (int argc, char **argv) {

    std::unordered_map<std::string, std::string> pconf;

    std::string brokers = "localhost";
    std::string errstr;

    std::string topic_str;                              // producer topic.
    int32_t partition = RdKafka::Topic::PARTITION_UA;   // producer partition.

    std::string config_file;
    std::string mode;
    std::string debug;

    std::string region_file;                            // map data for geofence.

    std::vector<std::string> topics;                    // consumer topics.

    int opt;                                            // command line option.

    int use_ccb = 0;                                    // consumer callback use flag.
    bool do_conf_dump = false;

    int64_t start_offset = RdKafka::Topic::OFFSET_BEGINNING;

    // configurations; global and topic (the names in these are fixed)
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    while ((opt = getopt(argc, argv, "c:t:p:g:b:z:qd:o:eX:AM:f:F:v")) != -1) {
        switch (opt) {

            case 'c':
                config_file = optarg;
                break;

            case 't':
                topic_str = optarg;
                break;

            case 'p':
                partition = std::atoi(optarg);
                break;

            case 'g':
                if (conf->set("group.id",  optarg, errstr) != RdKafka::Conf::CONF_OK) {
                    std::cerr << errstr << "\n";
                    exit(1);
                }
                break;

            case 'b':
                brokers = optarg;
                break;

//            case 'z':
//                if (conf->set("compression.codec", optarg, errstr) != RdKafka::Conf::CONF_OK) {
//                    std::cerr << errstr << "\n";
//                    exit(1);
//                }
//                break;

            case 'o':
                if (!strcmp(optarg, "end"))
                    start_offset = RdKafka::Topic::OFFSET_END;
                else if (!strcmp(optarg, "beginning"))
                    start_offset = RdKafka::Topic::OFFSET_BEGINNING;
                else if (!strcmp(optarg, "stored"))
                    start_offset = RdKafka::Topic::OFFSET_STORED;
                else
                    start_offset = strtoll(optarg, NULL, 10);
                break;

            case 'e':
                exit_eof = true;
                break;

            case 'd':
                debug = optarg;
                break;

//            case 'M':
//                if (conf->set("statistics.interval.ms", optarg, errstr) !=
//                        RdKafka::Conf::CONF_OK) {
//                    std::cerr << errstr << "\n";
//                    exit(1);
//                }
//                break;

//            case 'X':
//                {
//                    char *name, *val;
//
//                    if (!strcmp(optarg, "dump")) {
//                        do_conf_dump = true;
//                        continue;
//                    }
//
//                    name = optarg;
//
//                    if (!(val = strchr(name, '='))) {
//                        std::cerr << "%% Expected -X property=value, not " << name << "\n";
//                        std::exit(EXIT_FAILURE);
//                    }
//
//                    *val = '\0';
//                    val++;
//
//                    /* Try "topic." prefixed properties on topic
//                     * conf first, and then fall through to global if
//                     * it didnt match a topic configuration property. */
//                    RdKafka::Conf::ConfResult res = RdKafka::Conf::CONF_UNKNOWN;
//
//                    if (!strncmp(name, "topic.", strlen("topic.")))
//                        res = tconf->set(name+strlen("topic."), val, errstr);
//
//                    if (res == RdKafka::Conf::CONF_UNKNOWN)
//                        res = conf->set(name, val, errstr);
//
//                    if (res != RdKafka::Conf::CONF_OK) {
//                        std::cerr << errstr << "\n";
//                        exit(1);
//                    }
//                }
//                break;

//            case 'f':
//                if (!strcmp(optarg, "ccb"))
//                    use_ccb = 1;
//                else {
//                    std::cerr << "Unknown option: " << optarg << "\n";
//                    exit(1);
//                }
//                break;
            case 'F':
                // the file that defines the geofence indirectly.
                region_file = optarg;
                break;

            case 'q':
                verbosity--;
                break;

            case 'v':
                verbosity++;
                break;

            default:
                ;
                //goto usage;
        }
    }

    // build a list of the operands on the command line.
    for (; optind < argc ; optind++)
        topics.push_back(std::string(argv[optind]));


    if (config_file.empty() && (topics.empty() || optind != argc || region_file.empty())) {
        // bad command line: no topics; no region file; the number of arguments processed doesn't match spec.
usage:
        fprintf(stderr,
                "Usage: %s -g <group-id> -F <region-file> [options] topic1 topic2..\n"
                "\n"
                "librdkafka version %s (0x%08x)\n"
                "\n"
                " Options:\n"
                "  -c <configuration file>   Configuration for Kafka and Privacy\n"
                "  -F <region-file>          Region file containing geo-fence data.\n"
                "  -g <group-id>             Consumer group id\n"
                "  -b <brokers>              Broker address (localhost:9092)\n"
                "  -z <codec>                Enable compression:\n"
                "                            none|gzip|snappy\n"
                "  -e                        Exit consumer when last message\n"
                "                            in partition has been received.\n"
                "  -d [facs..]               Enable debugging contexts:\n"
                "                            %s\n"
                "  -M <intervalms>           Enable statistics\n"
                "  -X <prop=name>            Set arbitrary librdkafka configuration property\n"
                "                            Properties prefixed with \"topic.\" "
                "                            will be set on topic object.\n"
                "                            Use '-X list' to see the full list\n"
                "                            of supported properties.\n"
                "  -f <flag>                 Set option:\n"
                "                               ccb - use consume_callback\n"
                "  -q                        Quiet / Decrease verbosity\n"
                "  -v                        Increase verbosity\n"
                "\n"
                "\n",
            argv[0],
            RdKafka::version_str().c_str(), 
            RdKafka::version(),
            RdKafka::get_debug_contexts().c_str());
        std::exit(EXIT_FAILURE);
    }

    // this function will override configurations on the command line.
    configure( config_file, pconf, conf, tconf );

    // command line configurations will override the file.
    auto search = pconf.find("metadata.broker.list");
    if ( search == pconf.end() ) {
        // no broker set, use the command line or the default.
        conf->set("metadata.broker.list", brokers, errstr);
    } else {
        brokers = search->second;
    }

    if (!debug.empty()) {
        if (conf->set("debug", debug, errstr) != RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << "\n";
            exit(1);
        }
    }

    ExampleConsumeCb ex_consume_cb;

    if(use_ccb) {
        conf->set("consume_cb", &ex_consume_cb, errstr);
    }

    // ExampleEventCb ex_event_cb;
    // conf->set("event_cb", &ex_event_cb, errstr);

    if (do_conf_dump) {
    //if (true) {
        // dump the configuration and then exit.
        // TODO: build a dump method into the conf..?
        for (int pass = 0 ; pass < 2 ; pass++) {
            std::list<std::string> *dump;           // generic handle for both dumps.
            if (pass == 0) {
                dump = conf->dump();
                std::cout << "# Global config" << "\n";
            } else {
                dump = tconf->dump();
                std::cout << "# Topic config" << "\n";
            }

            for (std::list<std::string>::iterator it = dump->begin(); it != dump->end(); ) {
                std::cout << *it << " = ";
                it++;
                std::cout << *it << "\n";
                it++;
            }
            std::cout << "\n";
        }

        std::cout << "# Privacy config \n";
        for ( const auto& m : pconf ) {
            std::cout << m.first << " = " << m.second << '\n';
        }

        std::exit(EXIT_SUCCESS);
    }

    // librdkafka defined configuration.
    conf->set("default_topic_conf", tconf, errstr);

    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);

    search = pconf.find("privacy.filter.geofence.mapfile");
    if ( search != pconf.end() ) {
        region_file = search->second;
    } else {
        std::cerr << "No map file specified.\n";
        std::exit(EXIT_FAILURE);
    }

    Geo::Point sw, ne;

    // Build the quad.
    try {

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

    } catch ( std::exception& e ) {

        std::cout << e.what() << "\n";
        exit(0);

    }

    // Declare a quad with the given bounds.
    Quad::Ptr quad_ptr = std::make_shared<Quad>(sw, ne);

    try {
        // Read the file and parse the shapes.
        Shapes::CSVInputFactory shape_factory(region_file);
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


    } catch ( std::exception& e ) {
        std::cerr << "Problem building geofence: " << e.what() << '\n';
        delete tconf;
        delete conf;
        std::exit(EXIT_FAILURE);
    }

    BSMHandler handler{quad_ptr, pconf};

    // Consumer setup: bring in topic.J2735BsmRawJSON stream from the ODE (or a pipe to java producer).
    std::shared_ptr<RdKafka::KafkaConsumer> consumer{RdKafka::KafkaConsumer::create(conf, errstr)};

    if (!consumer) {
        std::cerr << "Failed to create consumer: " << errstr << "\n";
        std::exit(EXIT_FAILURE);
    }

    std::cout << ">> Created Consumer: " << consumer->name() << "\n";

    // grab the topic off the configuration.
    search = pconf.find("privacy.topic.consumer");
    if ( search != pconf.end() ) {
        topics.push_back( search->second );
    } else {
        std::cerr << "Failure to use configured consumer topic: " << errstr << "\n";
        std::exit(EXIT_FAILURE);
    }

    for ( const std::string& topic : topics ) {
        if ( !ode_topic_available( topic, consumer )) {
        std::cerr << "The ODE Topic: " << topic << " is not available. This topic must be readable.\n";
        std::exit(EXIT_FAILURE);
        }
    }

    // subscribe to the J2735BsmJson topic (or test)
    RdKafka::ErrorCode err = consumer->subscribe(topics);
    if (err) {
        std::cerr << "Failed to subscribe to " << topics.size() << " topics: " << RdKafka::err2str(err) << "\n";
        std::exit(EXIT_FAILURE);
    }

    // Producer setup: will take the filtered BSMs and send them back to the ODE (or a test java consumer).
    RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
        std::cerr << "Failed to create producer: " << errstr << "\n";
        std::exit(EXIT_FAILURE);
    }

    std::cout << ">> Created Producer: " << producer->name() << "\n";

    if (topic_str.empty()) {
        // maybe it was specified in the configuration file.
        auto search = pconf.find("privacy.topic.producer");
        if ( search != pconf.end() ) {
            topic_str = search->second;
        } else {
            std::cerr << "Topic String Empty!\n";
            std::exit(EXIT_FAILURE);
        }
    } 

    // The topic we are publishing filtered BSMs to.
    RdKafka::Topic *topic = RdKafka::Topic::create(producer, topic_str, tconf, errstr);
    if (!topic) {
        std::cerr << "Failed to create topic: " << errstr << "\n";
        std::exit(EXIT_FAILURE);
    } 

    RdKafka::ErrorCode status;

    // consume-produce loop.
    while (run) {           // run is modified by signals.

        RdKafka::Message *msg = consumer->consume(1000);

        if (!use_ccb) {

            status = msg_consume(msg, NULL, handler);

            switch (status) {
                case RdKafka::ERR__TIMED_OUT:
                    break;

                case RdKafka::ERR_NO_ERROR:
                    {
                        const BSM& bsm = handler.get_bsm();
                        std::cout << "Retaining BSM: " << bsm << "\n";

                        // if we still have a message in the handler, we send it back out to the producer we have made above.
                        status = producer->produce(topic, partition, RdKafka::Producer::RK_MSG_COPY, (void *)handler.get_json().c_str(), handler.get_bsm_buffer_size(), NULL, NULL);
                        //RdKafka::ErrorCode resp = producer->produce(topic, partition, RdKafka::Producer::RK_MSG_COPY, (void *)handler.get_json().c_str(), handler.get_bsm_buffer_size(), NULL, NULL);
                        if (status != RdKafka::ERR_NO_ERROR) {
                            std::cerr << "% Produce failed: " << RdKafka::err2str( status ) << "\n";
                        } 
                        // else {
                        //     std::cerr << "% Produced message (" << handler.get_bsm_buffer_size() << " bytes)" << "\n";
                        // }
                        //break;
                    }
                    break;

                case RdKafka::ERR_INVALID_MSG:
                    {
                        const BSM& bsm = handler.get_bsm();
                        std::cout << "Filtering BSM [" << handler.get_result_string() << "] : " << bsm << "\n";
                    }
                    break;

                default:
                    ;
            }

        }
        delete msg;
        // producer->poll(0);          // needed when using callbacks.
    }

    //    while (run && producer->outq_len() > 0) {
    //      std::cerr << "Waiting for " << producer->outq_len() << "\n";
    //      producer->poll(1000);         // needed when using callbacks.
    //    }

    delete tconf;
    delete conf;
    delete topic;
    delete producer;     // not needed with unique pointer.

#ifndef _MSC_VER
    alarm(10);
#endif

    consumer->close();

    std::cout << ">> Consumed " << msg_cnt << " messages (" << msg_bytes << " bytes)" << "\n";

    /*
     * Wait for RdKafka to decommission.
     * This is not strictly needed (with check outq_len() above), but
     * allows RdKafka to clean up all its resources before the application
     * exits so that memory profilers such as valgrind wont complain about
     * memory leaks.
     */
    RdKafka::wait_destroyed(5000);

    return 0;
}

#endif
