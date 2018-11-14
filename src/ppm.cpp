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
#include "spdlog/spdlog.h"
#include <csignal>
#include <chrono>
#include <thread>

// for both windows and linux.
#include <sys/types.h>
#include <sys/stat.h>

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

bool PPM::bootstrap = true;
bool PPM::bsms_available = true;

void PPM::sigterm (int sig) {
    bsms_available = false;
    bootstrap = false;
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
    iloglevel{ spdlog::level::trace },
    eloglevel{ spdlog::level::err },
    pconf{},
    brokers{"localhost"},
    partition{RdKafka::Topic::PARTITION_UA},
    mode{""},
    debug{""},
    offset{RdKafka::Topic::OFFSET_BEGINNING},
    published_topic{},
    consumed_topic{},
    conf{nullptr},
    tconf{nullptr},
    qptr{},
    consumer{},
    consumer_timeout{500},
    producer{},
    raw_topic{},
    filtered_topic{},
    ilogger{},
    elogger{}
{
}

PPM::~PPM() 
{
    if (consumer) consumer->close();

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
            if ( r ) ilogger->info( "Topic: {} found in the kafka metadata.", topic );
            ++it;
        }
        if (!r) ilogger->warn( "Metadata did not contain topic: {}.", topic );

    } else {
        elogger->error( "cannot retrieve consumer metadata with error: {}.", err2str(err) );
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

    if ( optIsSet('v') ) {
        if ( "trace" == optString('v') ) {
            ilogger->set_level( spdlog::level::trace );
        } else if ( "debug" == optString('v') ) {
            ilogger->set_level( spdlog::level::trace );
        } else if ( "info" == optString('v') ) {
            ilogger->set_level( spdlog::level::trace );
        } else if ( "warning" == optString('v') ) {
            ilogger->set_level( spdlog::level::warn );
        } else if ( "error" == optString('v') ) {
            ilogger->set_level( spdlog::level::err );
        } else if ( "critical" == optString('v') ) {
            ilogger->set_level( spdlog::level::critical );
        } else if ( "off" == optString('v') ) {
            ilogger->set_level( spdlog::level::off );
        } else {
            elogger->warn("information logger level was configured but unreadable; using default.");
        }
    } // else it is already set to default.

    ilogger->trace("starting configure()");

    std::string line;
    std::string error_string;
    StrVector pieces;

    // configurations; global and topic (the names in these are fixed)
    conf  = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    // must use a configuration file.
    if ( !optIsSet('c') ) {
        elogger->error( "asked to use a configuration file, but option not set." );
        return false;
    }

    const std::string& cfile = optString('c');              // needed for error message.
    ilogger->info("using configuration file: {}", cfile );
    std::ifstream ifs{ cfile };

    if (!ifs) {
        elogger->error("cannot open configuration file: {}", cfile);
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
                    ilogger->info("kafka topic configuration: {} = {}", pieces[0], pieces[1]);
                    done = true;
                }

                if ( conf->set(pieces[0], pieces[1], error_string) == RdKafka::Conf::CONF_OK ) {
                    ilogger->info("kafka configuration: {} = {}", pieces[0], pieces[1]);
                    done = true;
                }

                if ( !done ) { 
                    ilogger->info("ppm configuration: {} = {}", pieces[0], pieces[1]);
                    // These configuration options are not expected by Kafka.
                    // Assume there are for the PPM.
                    pconf[ pieces[0] ] = pieces[1];
                }

            } else {
                elogger->warn("too many pieces in the configuration file line: {}", line);
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
            elogger->error("no map file specified; must fail.");
            return false;
        }
    }

    ilogger->info("ppm mapfile: {}", mapfile);

    qptr = BuildGeofence( mapfile );                // throws.

    if ( optIsSet('b') ) {
        // broker specified.
        ilogger->info("setting kafka broker to: {}", optString('b'));
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

    ilogger->info("kafka partition: {}", partition);

    if ( getOption('g').isSet() && conf->set("group.id", optString('g'), error_string) != RdKafka::Conf::CONF_OK) {
        // NOTE: there are some checks in librdkafka that require this to be present and set.
        elogger->error("kafka error setting configuration parameters group.id h: {}", error_string);
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

        ilogger->info("offset in partition set to byte: {}", o);
    }

    // Do we want to exit if a stream eof is sent.
    exit_eof = getOption('x').isSet();

    if (optIsSet('d') && conf->set("debug", optString('d'), error_string) != RdKafka::Conf::CONF_OK) {
        elogger->error("kafka error setting configuration parameter debug: {}", error_string);
        return false;
    }

    // librdkafka defined configuration.
    if (optIsSet('u')) {
        // this is the produced (filtered) topic.
        consumed_topic = optString( 'u' );
    } else {
        auto search = pconf.find("privacy.topic.consumer");
        if ( search != pconf.end() ) {
            consumed_topic = search->second;
        } else {
            elogger->error("no consumer topic was specified; must fail.");
            return false;
        }
    }

    ilogger->info("consumed topic: {}", consumed_topic);

    if (optIsSet('f')) {
        // this is the produced (filtered) topic.
        published_topic = optString( 'f' );

    } else {
        // maybe it was specified in the configuration file.
        auto search = pconf.find("privacy.topic.producer");
        if ( search != pconf.end() ) {
            published_topic = search->second;
        } else {
            elogger->error("no publisher topic was specified; must fail.");
            return false;
        }
    }

    ilogger->info("published topic: {}", published_topic);

    auto search = pconf.find("privacy.consumer.timeout.ms");
    if ( search != pconf.end() ) {
        try {
            consumer_timeout = std::stoi( search->second );
        } catch( std::exception& e ) {
            ilogger->info("using the default consumer timeout value.");
        }
    }

    ilogger->trace("ending configure()");
    return true;
}

bool PPM::msg_consume(RdKafka::Message* message, void* opaque, BSMHandler& handler) {
    static std::string tsname;
    static RdKafka::MessageTimestamp ts;

    // payload is a void *
    // len is a size_t
    std::string payload(static_cast<const char*>(message->payload()), message->len());

    switch (message->err()) {
        case RdKafka::ERR__TIMED_OUT:
            ilogger->info("Waiting for more BSMs from the ODE producer.");
            break;

        case RdKafka::ERR_NO_ERROR:
            /* Real message */
            bsm_recv_count++;
            bsm_recv_bytes += message->len();

            ilogger->trace("Read message at byte offset: {}", message->offset() );

            ts = message->timestamp();

            if (ts.type != RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE) {
                if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME) {
                    tsname = "create time";
                } else if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_LOG_APPEND_TIME) {
                    tsname = "log append time";
                } else {
                    tsname = "unknown";
                }

                ilogger->trace("Message timestamp: {}, type: {}", tsname, ts.timestamp);
            }

            if ( message->key() ) {
                ilogger->trace("Message key: {}", *message->key() );
            }

            // Process the BSM payload.
            if ( handler.process( payload ) ) {
                // the complete BSM was parsed, so we have all the information.
                ilogger->info("BSM [RETAINED]: {}", handler.get_bsm().logString());
                return true;
                
            } else {
                // Suppressed BSM.
                ilogger->info("BSM [SUPPRESSED-{}]: {}", handler.get_result_string(), handler.get_bsm().logString());
                bsm_filt_count++;
                bsm_filt_bytes += message->len();
            } // return false;

            break;

        case RdKafka::ERR__PARTITION_EOF:
            ilogger->info("ODE BSM consumer partition end of file, but PPM still alive.");
            if (exit_eof) {
                eof_cnt++;

                if (eof_cnt == partition_cnt) {
                    ilogger->info("EOF reached for all {} partition(s)", partition_cnt);
                    bsms_available = false;
                }
            }
            break;

        case RdKafka::ERR__UNKNOWN_TOPIC:
            elogger->error("cannot consume due to an UNKNOWN consumer topic: {}", message->errstr());
            bsms_available = false;
            break;

        case RdKafka::ERR__UNKNOWN_PARTITION:
            elogger->error("cannot consume due to an UNKNOWN consumer partition: {}", message->errstr());
            bsms_available = false;
            break;

        default:
            elogger->error("cannot consume due to an error: {}", message->errstr());
            bsms_available = false;
    }

    return false;
}

Quad::Ptr PPM::BuildGeofence( const std::string& mapfile )  // throws
{
    geo::Point sw, ne;

    ilogger->trace("Starting BuildGeofence.");

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

    ilogger->trace("Completed BuildGeofence.");
    return qptr;
}

bool PPM::launch_producer()
{
    std::string error_string;

    if (!producer) {
        producer = std::shared_ptr<RdKafka::Producer>( RdKafka::Producer::create(conf, error_string) );

        if (!producer) {
            elogger->critical("Failed to create producer with error: {}.", error_string );
            return false;
        }
    }

    filtered_topic = std::shared_ptr<RdKafka::Topic>( RdKafka::Topic::create(producer.get(), published_topic, tconf, error_string) );
    if ( !filtered_topic ) {
        elogger->critical("Failed to create topic: {}. Error: {}.", published_topic, error_string );
        return false;
    } 

    ilogger->info("Producer: {} created using topic: {}.", producer->name(), published_topic);
    return true;
}

bool PPM::launch_consumer()
{
    std::string error_string;
    
    if (!consumer) {
        consumer = std::shared_ptr<RdKafka::KafkaConsumer>( RdKafka::KafkaConsumer::create(conf, error_string) );

        if (!consumer) {
            elogger->critical("Failed to create consumer with error: {}",  error_string );
            return false;
        }
    }

    //raw_topic = nullptr;

    std::vector<std::string> topics;

    while ( bsms_available ) {
        if ( topic_available( consumed_topic ) ) {
            ilogger->trace("Consumer topic: {} is available.", consumed_topic);
            //raw_topic = std::shared_ptr<RdKafka::Topic>( RdKafka::Topic::create(consumer.get(), consumed_topic, tconf, error_string) );
            topics.push_back(consumed_topic);
            RdKafka::ErrorCode err = consumer->subscribe(topics);

            if ( err ) {
                elogger->critical("Failed to subscribe to  topic: {}. Error: {}.", consumed_topic, RdKafka::err2str(err) );
                return false;
            } 

            break;
        }

        // topic is not available, wait for a second or two.
        std::this_thread::sleep_for( std::chrono::milliseconds( 1500 ) );
        ilogger->trace("Waiting for needed consumer topic: {}.", consumed_topic);
    }

    ilogger->info("Consumer: {} created using topic: {}.", consumer->name(), consumed_topic);
    return true;
}

/**
 * @brief predicate indicating whether a file exists on the filesystem.
 *
 * @return true if it exists, false otherwise.
 */
bool fileExists( const std::string& s ) {
    struct stat info;

    if (stat( s.c_str(), &info) != 0) {     // bad stat; doesn't exist.
        return false;
    } else if (info.st_mode & S_IFREG) {    // exists and is a regular file.
        return true;
    } 

    return false;
}

/**
 * @brief predicate indicating whether a path/directory exists on the filesystem.
 *
 * @return true if it exists, false otherwise.
 */
bool dirExists( const std::string& s ) {
    struct stat info;

    if (stat( s.c_str(), &info) != 0) {     // bad stat; doesn't exist.
        return false;
    } else if (info.st_mode & S_IFDIR) {    // exists and is a directory.
        return true;
    } 

    return false;
}

bool PPM::make_loggers( bool remove_files )
{
    // defaults.
    std::string path{ "logs/" };
    std::string ilogname{ "log.info" };
    std::string elogname{ "log.error" };

    if (getOption('D').hasArg()) {
        // replace default
        path = getOption('D').argument();
        if ( path.back() != '/' ) {
            path += '/';
        }
    }

    // if the directory specified doesn't exist, then make it.
    if (!dirExists( path )) {
#ifndef _MSC_VER
        if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) != 0)   // linux
#elif _MSC_VER 
        if (_mkdir(path.c_str()) != 0)                                          // windows
#else
                                                                                // some other strange os...
#endif
        {
            std::cerr << "Error making the logging directory.\n";
            return false;
        }
    }
    
    // ilog check for user-defined file locations and names.
    if (getOption('i').hasArg()) {
        // replace default.
        ilogname = string_utilities::basename<std::string>( getOption('i').argument() );
    }

    if (getOption('e').hasArg()) {
        // replace default.
        elogname = string_utilities::basename<std::string>( getOption('e').argument() );
    }
    
    ilogname = path + ilogname;
    elogname = path + elogname;

    if ( remove_files && fileExists( ilogname ) ) {
        if ( std::remove( ilogname.c_str() ) != 0 ) {
            std::cerr << "Error removing the previous information log file.\n";
            return false;
        }
    }

    if ( remove_files && fileExists( elogname ) ) {
        if ( std::remove( elogname.c_str() ) != 0 ) {
            std::cerr << "Error removing the previous error log file.\n";
            return false;
        }
    }

    // setup information logger.
    ilogger = spdlog::rotating_logger_mt("ilog", ilogname, ilogsize, ilognum);
    ilogger->set_pattern("[%C%m%d %H:%M:%S.%f] [%l] %v");
    ilogger->set_level( iloglevel );

    // setup error logger.
    elogger = spdlog::rotating_logger_mt("elog", elogname, elogsize, elognum);
    elogger->set_level( iloglevel );
    elogger->set_pattern("[%C%m%d %H:%M:%S.%f] [%l] %v");
    return true;
}

int PPM::operator()(void) {

    std::string error_string;
    RdKafka::ErrorCode status;

    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);

    try {

        // throws for mapfile and other items.
        if ( !configure() ) return EXIT_FAILURE;

    } catch ( std::exception& e ) {

        // don't use logger in case we cannot configure it correctly.
        std::cerr << "Fatal Exception: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    while (bootstrap) {
        // reset flag here, or else nothing works below
        bsms_available = true;

        if (!launch_consumer()) {
            std::this_thread::sleep_for( std::chrono::milliseconds( 1500 ) );

            continue;
        }

        if (!launch_producer()) {
            std::this_thread::sleep_for( std::chrono::milliseconds( 1500 ) );

            continue;
        }

        // JMC: There was leak in here caused by RapidJSON.  It has been fixed.  The notes are in that class's code.
        BSMHandler handler{qptr, pconf};

        std::vector<RdKafka::TopicPartition*> partitions;
        RdKafka::ErrorCode err = consumer->position(partitions);

        if (err) {
            ilogger->info("err {}", RdKafka::err2str(err));
        } else {
            for (auto *partition : partitions) {
                ilogger->info("topar {} {}", partition->topic(), partition->offset());
            }
        }

        // consume-produce loop.
        while (bsms_available) {

            std::unique_ptr<RdKafka::Message> msg{ consumer->consume( consumer_timeout ) };

            if ( msg_consume(msg.get(), NULL, handler) ) {

                status = producer->produce(filtered_topic.get(), partition, RdKafka::Producer::RK_MSG_COPY, (void *)handler.get_json().c_str(), handler.get_bsm_buffer_size(), NULL, NULL);

                if (status != RdKafka::ERR_NO_ERROR) {
                    elogger->error("failed to produce retained BSM because: {}", RdKafka::err2str( status ));

                } else {
                    // successfully sent; update counters.
                    bsm_send_count++;
                    bsm_send_bytes += msg->len();
                    ilogger->trace("produced BSM successfully.");
                }
            } 

            // NOTE: good for troubleshooting, but bad for performance.
            elogger->flush();
            ilogger->flush();
        }
    }

    ilogger->info("PPM operations complete; shutting down...");
    ilogger->info("PPM consumed  : {} BSMs and {} bytes", bsm_recv_count, bsm_recv_bytes);
    ilogger->info("PPM published : {} BSMs and {} bytes", bsm_send_count, bsm_send_bytes);
    ilogger->info("PPM suppressed: {} BSMs and {} bytes", bsm_filt_count, bsm_filt_bytes);
    return EXIT_SUCCESS;
}

#ifndef _PPM_TESTS

int main( int argc, char* argv[] )
{
    PPM ppm{"ppm","Privacy Protection Module"};

    ppm.addOption( 'c', "config", "Configuration for Kafka and Privacy Protection Module.", true );
    ppm.addOption( 'C', "config-check", "Check the configuration and output the settings.", false );
    ppm.addOption( 'u', "unfiltered-topic", "The unfiltered consume topic.", true );
    ppm.addOption( 'f', "filtered-topic", "The unfiltered produce topic.", true );
    ppm.addOption( 'p', "partition", "Consumer topic partition from which to read.", true );
    ppm.addOption( 'g', "group", "Consumer group identifier", true );
    ppm.addOption( 'b', "broker", "List of broker addresses (localhost:9092)", true );
    ppm.addOption( 'o', "offset", "Byte offset to start reading in the consumed topic.", true );
    ppm.addOption( 'x', "exit", "Exit consumer when last message in partition has been received.", false );
    ppm.addOption( 'd', "debug", "debug level.", true );
    ppm.addOption( 'm', "mapfile", "Map data file to specify the geofence.", true );
    ppm.addOption( 'v', "log-level", "The info log level [trace,debug,info,warning,error,critical,off]", true );
    ppm.addOption( 'D', "log-dir", "Directory for the log files.", true );
    ppm.addOption( 'R', "log-rm", "Remove specified/default log files if they exist.", false );
    ppm.addOption( 'i', "ilog", "Information log file name.", true );
    ppm.addOption( 'e', "elog", "Error log file name.", true );
    ppm.addOption( 'h', "help", "print out some help" );

    if (!ppm.parseArgs(argc, argv)) {
        ppm.usage();
        std::exit( EXIT_FAILURE );
    }

    if (ppm.optIsSet('h')) {
        ppm.help();
        std::exit( EXIT_SUCCESS );
    }

    // can set levels if needed here.
    if ( !ppm.make_loggers( ppm.optIsSet('R') )) {
        std::exit( EXIT_FAILURE );
    }

    // configuration check.
    if (ppm.optIsSet('C')) {
        try {
            if (ppm.configure()) {
                ppm.print_configuration();
                std::exit( EXIT_SUCCESS );
            } else {
                std::cerr << "Current configuration settings do not work.\n";
                ppm.ilogger->error( "current configuration settings do not work; exiting." );
                std::exit( EXIT_FAILURE );
            }
        } catch ( std::exception& e ) {
            std::cerr << "Fatal Exception: " << e.what() << '\n';
            ppm.elogger->error( "exception: {}", e.what() );
            std::exit( EXIT_FAILURE );
        }
    }

    std::exit( ppm.run() );
}

#endif
