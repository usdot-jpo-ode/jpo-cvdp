/*
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

/**
 * Apache Kafka consumer & producer example programs
 * using the Kafka driver from librdkafka
 * (https://github.com/edenhill/librdkafka)
 */

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>

#ifdef _MSC_VER
#include "../win32/wingetopt.h"
#elif _AIX
#include <unistd.h>
#else
#include <getopt.h>
#endif

#include <librdkafka/rdkafkacpp.h>
#include "../include/ppmLogger.hpp"

#include <sstream>

static std::shared_ptr<PpmLogger> logger = std::make_shared<PpmLogger>("info.log", "error.log");
static std::stringstream ss;

static void metadata_print (const std::string &topic, const RdKafka::Metadata *metadata) {
  ss << "Metadata for " << (topic.empty() ? "" : "all topics")
           << "(from broker "  << metadata->orig_broker_id()
           << ":" << metadata->orig_broker_name() << "\n";
  logger->info(ss.str());
  ss.str(""); ss.clear();

  /* Iterate brokers */
  ss << " " << metadata->brokers()->size() << " brokers:" << "\n";
  logger->info(ss.str());
  ss.str(""); ss.clear();

  RdKafka::Metadata::BrokerMetadataIterator ib;
  for (ib = metadata->brokers()->begin();
       ib != metadata->brokers()->end();
       ++ib) {
    ss << "  broker " << (*ib)->id() << " at "
              << (*ib)->host() << ":" << (*ib)->port() << "\n";
  }
  logger->info(ss.str());
  ss.str(""); ss.clear();

  /* Iterate topics */
  logger->info(metadata->topics()->size() + " topics:");
  RdKafka::Metadata::TopicMetadataIterator it;
  for (it = metadata->topics()->begin();
       it != metadata->topics()->end();
       ++it) {
    ss << "  topic \""<< (*it)->topic() << "\" with "
              << (*it)->partitions()->size() << " partitions:";

    if ((*it)->err() != RdKafka::ERR_NO_ERROR) {
      ss << " " << err2str((*it)->err());
      if ((*it)->err() == RdKafka::ERR_LEADER_NOT_AVAILABLE)
        ss << " (try again)";
    }
    logger->info(ss.str());
    ss.str(""); ss.clear();

    /* Iterate topic's partitions */
    RdKafka::TopicMetadata::PartitionMetadataIterator ip;
    for (ip = (*it)->partitions()->begin();
         ip != (*it)->partitions()->end();
         ++ip) {
      ss << "    partition " << (*ip)->id()
                << ", leader " << (*ip)->leader()
                << ", replicas: ";

      /* Iterate partition's replicas */
      RdKafka::PartitionMetadata::ReplicasIterator ir;
      for (ir = (*ip)->replicas()->begin();
           ir != (*ip)->replicas()->end();
           ++ir) {
        ss << (ir == (*ip)->replicas()->begin() ? "":",") << *ir;
      }

      /* Iterate partition's ISRs */
      ss << ", isrs: ";
      RdKafka::PartitionMetadata::ISRSIterator iis;
      for (iis = (*ip)->isrs()->begin(); iis != (*ip)->isrs()->end() ; ++iis)
        ss << (iis == (*ip)->isrs()->begin() ? "":",") << *iis;

      if ((*ip)->err() != RdKafka::ERR_NO_ERROR) {
        ss << ", " << RdKafka::err2str((*ip)->err()) << "\n";
        logger->error(ss.str());
      }
      else {
        logger->info(ss.str());
      }
      ss.str(""); ss.clear();
    }
  }
}

static bool run = true;
static bool exit_eof = false;

static void sigterm (int sig) {
  run = false;
}


class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb {
 public:
  void dr_cb (RdKafka::Message &message) {
    logger->error("Message delivery for (" + std::to_string(message.len()) + " bytes): " + message.errstr());
    if (message.key())
      logger->info("Key: " + *(message.key()) + ";");
  }
};


class ExampleEventCb : public RdKafka::EventCb {
 public:
  void event_cb (RdKafka::Event &event) {
    switch (event.type())
    {
      case RdKafka::Event::EVENT_ERROR:
        logger->error("ERROR (" + RdKafka::err2str(event.err()) + "): " + event.str());
        if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
          run = false;
        break;

      case RdKafka::Event::EVENT_STATS:
        logger->error("\"STATS\": " + event.str());
        break;

      case RdKafka::Event::EVENT_LOG:
        logger->error("LOG-" + std::to_string(event.severity()) + "-" + event.fac().c_str() + ": " + event.str().c_str());
        break;

      default:
        ss << "EVENT " << event.type() << " (" << RdKafka::err2str(event.err()) << "): " << event.str() << "\n";
        logger->error(ss.str());
        ss.str(""); ss.clear();
        break;
    }
  }
};


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

void msg_consume(RdKafka::Message* message, void* opaque) {
  switch (message->err()) {
    case RdKafka::ERR__TIMED_OUT:
      break;

    case RdKafka::ERR_NO_ERROR:
      /* Real message */
      logger->info("Read msg at offset " + std::to_string(message->offset()));
      if (message->key()) {
        logger->info("Key: " + *message->key());
      }
      logger->info(std::to_string(static_cast<int>(message->len())) + std::string(static_cast<const char *>(message->payload())));
      break;

    case RdKafka::ERR__PARTITION_EOF:
      /* Last message */
      if (exit_eof) {
        run = false;
      }
      break;

    case RdKafka::ERR__UNKNOWN_TOPIC:
    case RdKafka::ERR__UNKNOWN_PARTITION:
      logger->error("Consume failed: " + message->errstr());
      run = false;
      break;

    default:
      /* Errors */
      logger->error("Consume failed: " + message->errstr());
      run = false;
  }
}


class ExampleConsumeCb : public RdKafka::ConsumeCb {
 public:
  void consume_cb (RdKafka::Message &msg, void *opaque) {
    msg_consume(&msg, opaque);
  }
};



int main (int argc, char **argv) {
  std::string brokers = "localhost";
  std::string errstr;
  std::string topic_str;
  std::string mode;
  std::string debug;
  int32_t partition = RdKafka::Topic::PARTITION_UA;
  int64_t start_offset = RdKafka::Topic::OFFSET_BEGINNING;
  bool do_conf_dump = false;
  int opt;
  MyHashPartitionerCb hash_partitioner;
  int use_ccb = 0;

  /*
   * Create configuration objects
   */
  RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
  RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);


  while ((opt = getopt(argc, argv, "PCLt:p:b:z:qd:o:eX:AM:f:")) != -1) {
    switch (opt) {
    case 'P':
    case 'C':
    case 'L':
      mode = opt;
      break;
    case 't':
      topic_str = optarg;
      break;
    case 'p':
      if (!strcmp(optarg, "random"))
        /* default */;
      else if (!strcmp(optarg, "hash")) {
        if (tconf->set("partitioner_cb", &hash_partitioner, errstr) !=
            RdKafka::Conf::CONF_OK) {
          logger->error(errstr);
          exit(1);
        }
      } else
        partition = std::atoi(optarg);
      break;
    case 'b':
      brokers = optarg;
      break;
    case 'z':
      if (conf->set("compression.codec", optarg, errstr) !=
	  RdKafka::Conf::CONF_OK) {
	logger->error(errstr);
	exit(1);
      }
      break;
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
    case 'M':
      if (conf->set("statistics.interval.ms", optarg, errstr) !=
          RdKafka::Conf::CONF_OK) {
        logger->error(errstr);
        exit(1);
      }
      break;
    case 'X':
      {
	char *name, *val;

	if (!strcmp(optarg, "dump")) {
	  do_conf_dump = true;
	  continue;
	}

	name = optarg;
	if (!(val = strchr(name, '='))) {
    logger->error("Expected -X property=value, not " + std::string(name));
	  exit(1);
	}

	*val = '\0';
	val++;

	/* Try "topic." prefixed properties on topic
	 * conf first, and then fall through to global if
	 * it didnt match a topic configuration property. */
        RdKafka::Conf::ConfResult res;
	if (!strncmp(name, "topic.", strlen("topic.")))
          res = tconf->set(name+strlen("topic."), val, errstr);
        else
	  res = conf->set(name, val, errstr);

	if (res != RdKafka::Conf::CONF_OK) {
          logger->error(errstr);
	  exit(1);
	}
      }
      break;

      case 'f':
        if (!strcmp(optarg, "ccb"))
          use_ccb = 1;
        else {
          logger->error("Unknown option: " + std::string(optarg));
          exit(1);
        }
        break;

    default:
      goto usage;
    }
  }

  if (mode.empty() || (topic_str.empty() && mode != "L") || optind != argc) {
  usage:
	  std::string features;
	  conf->get("builtin.features", features);
    fprintf(stderr,
            "Usage: %s [-C|-P] -t <topic> "
            "[-p <partition>] [-b <host1:port1,host2:port2,..>]\n"
            "\n"
            "librdkafka version %s (0x%08x, builtin.features \"%s\")\n"
            "\n"
            " Options:\n"
            "  -C | -P         Consumer or Producer mode\n"
            "  -L              Metadata list mode\n"
            "  -t <topic>      Topic to fetch / produce\n"
            "  -p <num>        Partition (random partitioner)\n"
            "  -p <func>       Use partitioner:\n"
            "                  random (default), hash\n"
            "  -b <brokers>    Broker address (localhost:9092)\n"
            "  -z <codec>      Enable compression:\n"
            "                  none|gzip|snappy\n"
            "  -o <offset>     Start offset (consumer)\n"
            "  -e              Exit consumer when last message\n"
            "                  in partition has been received.\n"
            "  -d [facs..]     Enable debugging contexts:\n"
            "                  %s\n"
            "  -M <intervalms> Enable statistics\n"
            "  -X <prop=name>  Set arbitrary librdkafka "
            "configuration property\n"
            "                  Properties prefixed with \"topic.\" "
            "will be set on topic object.\n"
            "                  Use '-X list' to see the full list\n"
            "                  of supported properties.\n"
            "  -f <flag>       Set option:\n"
            "                     ccb - use consume_callback\n"
            "\n"
            " In Consumer mode:\n"
            "  writes fetched messages to stdout\n"
            " In Producer mode:\n"
            "  reads messages from stdin and sends to broker\n"
            "\n"
            "\n"
            "\n",
	    argv[0],
	    RdKafka::version_str().c_str(), RdKafka::version(),
		features.c_str(),
	    RdKafka::get_debug_contexts().c_str());
	exit(1);
  }


  /*
   * Set configuration properties
   */
  conf->set("metadata.broker.list", brokers, errstr);

  if (!debug.empty()) {
    if (conf->set("debug", debug, errstr) != RdKafka::Conf::CONF_OK) {
      logger->error(errstr);
      exit(1);
    }
  }

  ExampleEventCb ex_event_cb;
  conf->set("event_cb", &ex_event_cb, errstr);

  if (do_conf_dump) {
    int pass;

    for (pass = 0 ; pass < 2 ; pass++) {
      std::list<std::string> *dump;
      if (pass == 0) {
        dump = conf->dump();
        logger->info("# Global config");
      } else {
        dump = tconf->dump();
        logger->info("# Topic config");
      }

      for (std::list<std::string>::iterator it = dump->begin(); it != dump->end(); ) {
        ss << *it << " = ";
        it++;
        ss << *it << "\n";
        it++;
      }
      logger->info(ss.str());
      ss.str(""); ss.clear();
    }
    exit(0);
  }

  signal(SIGINT, sigterm);
  signal(SIGTERM, sigterm);


  if (mode == "P") {
    /*
     * Producer mode
     */

    if(topic_str.empty())
      goto usage;

    ExampleDeliveryReportCb ex_dr_cb;

    /* Set delivery report callback */
    conf->set("dr_cb", &ex_dr_cb, errstr);

    /*
     * Create producer using accumulated global configuration.
     */
    RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
      logger->error("Failed to create producer: " + errstr);
      exit(1);
    }

    logger->error("% Created producer " + producer->name());

    /*
     * Create topic handle.
     */
    RdKafka::Topic *topic = RdKafka::Topic::create(producer, topic_str, tconf, errstr);
    if (!topic) {
      logger->error("Failed to create topic: " + errstr);
      exit(1);
    }

    /*
     * Read messages from stdin and produce to broker.
     */
    for (std::string line; run && std::getline(std::cin, line);) {
      if (line.empty()) {
        producer->poll(0);
	      continue;
      }

      /*
       * Produce message
       */
      RdKafka::ErrorCode resp = producer->produce(
        topic, 
        partition, 
        RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
			  const_cast<char *>(line.c_str()), 
        line.size(),
			  NULL, 
        NULL
      );

      if (resp != RdKafka::ERR_NO_ERROR)
	      logger->error("% Produce failed: " + RdKafka::err2str(resp));
      else
	      logger->error("% Produced message (" + std::to_string(line.size()) + " bytes)");

      producer->poll(0);
    }
    run = true;

    while (run && producer->outq_len() > 0) {
      logger->error("Waiting for " + std::to_string(producer->outq_len()));
      producer->poll(1000);
    }

    delete topic;
    delete producer;


  } else if (mode == "C") {
    /*
     * Consumer mode
     */

    if(topic_str.empty())
      goto usage;

    /*
     * Create consumer using accumulated global configuration.
     */
    RdKafka::Consumer *consumer = RdKafka::Consumer::create(conf, errstr);
    if (!consumer) {
      logger->error("Failed to create consumer: " + errstr);
      exit(1);
    }

    logger->error("% Created consumer " + consumer->name());

    /*
     * Create topic handle.
     */
    RdKafka::Topic *topic = RdKafka::Topic::create(consumer, topic_str,
						   tconf, errstr);
    if (!topic) {
      logger->error("Failed to create topic: " + errstr);
      exit(1);
    }

    /*
     * Start consumer for topic+partition at start offset
     */
    RdKafka::ErrorCode resp = consumer->start(topic, partition, start_offset);
    if (resp != RdKafka::ERR_NO_ERROR) {
      logger->error("Failed to start consumer: " + RdKafka::err2str(resp));
      exit(1);
    }

    ExampleConsumeCb ex_consume_cb;

    /*
     * Consume messages
     */
    while (run) {
      if (use_ccb) {
        consumer->consume_callback(topic, partition, 1000,
                                   &ex_consume_cb, &use_ccb);
      } else {
        RdKafka::Message *msg = consumer->consume(topic, partition, 1000);
        msg_consume(msg, NULL);
        delete msg;
      }
      consumer->poll(0);
    }

    /*
     * Stop consumer
     */
    consumer->stop(topic, partition);

    consumer->poll(1000);

    delete topic;
    delete consumer;
  } else {
    /* Metadata mode */

    /*
     * Create producer using accumulated global configuration.
     */
    RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
      logger->error("Failed to create producer: " + errstr);
      exit(1);
    }

    logger->error("% Created producer " + producer->name());

    /*
     * Create topic handle.
     */
    RdKafka::Topic *topic = NULL;
    if(!topic_str.empty()) {
      topic = RdKafka::Topic::create(producer, topic_str, tconf, errstr);
      if (!topic) {
        logger->error("Failed to create topic: " + errstr);
        exit(1);
      }
    }

    while (run) {
      class RdKafka::Metadata *metadata;

      /* Fetch metadata */
      RdKafka::ErrorCode err = producer->metadata(topic!=NULL, topic,
                              &metadata, 5000);
      if (err != RdKafka::ERR_NO_ERROR) {
        logger->error("%% Failed to acquire metadata: " + RdKafka::err2str(err));
              run = 0;
              break;
      }

      metadata_print(topic_str, metadata);

      delete metadata;
      run = 0;
    }

  }


  /*
   * Wait for RdKafka to decommission.
   * This is not strictly needed (when check outq_len() above), but
   * allows RdKafka to clean up all its resources before the application
   * exits so that memory profilers such as valgrind wont complain about
   * memory leaks.
   */
  RdKafka::wait_destroyed(5000);

  return 0;
}
