#ifndef CVDP_KAFKA_CONSUMER_H
#define CVDP_KAFKA_CONSUMER_H

#include "librdkafka/rdkafkacpp.h"
#include <unordered_map>
#include <csignal>

#ifndef _MSC_VER
#include <sys/time.h>
#endif

#ifdef _MSC_VER
#include "../../win32/wingetopt.h"
#include <atltime.h>
#elif _AIX
#include <unistd.h>
#else
#include <getopt.h>
#include <unistd.h>
#endif

#include "bsmHandler.hpp"
#include "ppmLogger.hpp"
#include "cvlib.hpp"

#include <sstream>

/* Use of this partitioner is pretty pointless since no key is provided
 * in the produce() call. */
class MyHashPartitionerCb : public RdKafka::PartitionerCb {
    public:
        int32_t partitioner_cb (const RdKafka::Topic *topic, const std::string *key,int32_t partition_cnt, void *msg_opaque);
    
    private:
        inline unsigned int djb_hash (const char *str, size_t len);
};

/**
 * NOTE: This is supposed to be a little more efficient.
 */
class ExampleConsumeCb : public RdKafka::ConsumeCb {
    public:
        void consume_cb(RdKafka::Message &msg, void *opaque);
};

class KafkaConsumer {
    public:
        bool ode_topic_available(const std::string& topic, std::shared_ptr<RdKafka::KafkaConsumer> consumer);
        RdKafka::ErrorCode msg_consume(RdKafka::Message* message, void* opaque, BSMHandler& handler);
        bool configure(const std::string& config_file, std::unordered_map<std::string,std::string>& pconf, RdKafka::Conf *conf, RdKafka::Conf *tconf);
        int execute(int argc, char **argv);
    
    private:
        std::shared_ptr<PpmLogger> logger = std::make_shared<PpmLogger>("log");
        bool run = true;
        bool exit_eof = false;
        int eof_cnt = 0;
        int partition_cnt = 0;
        long msg_cnt = 0;
        int64_t msg_bytes = 0;
        int verbosity = 1;

        void sigterm(int sig);
        const char* getEnvironmentVariable(const char* variableName);
};

#endif // CVDP_KAFKA_CONSUMER_H