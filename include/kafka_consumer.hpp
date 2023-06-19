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

class KafkaConsumer {
    public:
        bool ode_topic_available(const std::string& topic, std::shared_ptr<RdKafka::KafkaConsumer> consumer);
        RdKafka::ErrorCode msg_consume(RdKafka::Message* message, void* opaque, BSMHandler& handler);
        bool configure(const std::string& config_file, std::unordered_map<std::string,std::string>& pconf, RdKafka::Conf *conf, RdKafka::Conf *tconf);
        int execute(int argc, char **argv);
    
    private:
        std::shared_ptr<PpmLogger> logger = std::make_shared<PpmLogger>("log");
        bool exit_eof = false;
        int eof_cnt = 0;
        int partition_cnt = 0;
        long msg_cnt = 0;
        int64_t msg_bytes = 0;
        int verbosity = 1;

        const char* getEnvironmentVariable(const char* variableName);
};

#endif // CVDP_KAFKA_CONSUMER_H