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

/**
 * @brief This class is used to consume messages from Kafka
 * 
 */
class KafkaConsumer {
    public:
        /**
         * @brief Check if topic is available
         * 
         * @param topic The name of the topic
         * @param consumer The RdKafka::KafkaConsumer object
         * @return true if topic is available
         * @return false if topic is not available
         */
        bool ode_topic_available(const std::string& topic, std::shared_ptr<RdKafka::KafkaConsumer> consumer);

        /**
         * @brief Consume message
         * 
         * @param message The RdKafka::Message object to be consumed
         * @param opaque The opaque object
         * @param handler The BSMHandler object
         * @return RdKafka::ErrorCode The error code
         */
        RdKafka::ErrorCode msg_consume(RdKafka::Message* message, void* opaque, BSMHandler& handler);

        /**
         * @brief Configure Kafka consumer
         * 
         * @param config_file The path to the configuration file
         * @param pconf The unordered map of configuration parameters
         * @param conf The RdKafka::Conf object
         * @param tconf The RdKafka::Conf object
         * @return true if configuration is successful
         * @return false if configuration is not successful
         */
        bool configure(const std::string& config_file, std::unordered_map<std::string,std::string>& pconf, RdKafka::Conf *conf, RdKafka::Conf *tconf);
        
        /**
         * @brief Execute Kafka consumer
         * 
         * @param argc The number of arguments
         * @param argv The array of arguments
         * @return int reflecting the exit status
         */
        int execute(int argc, char **argv);
    
    private:
        /**
         * @brief Pointer to the PPM logger instance
         * 
         */
        std::shared_ptr<PpmLogger> logger = std::make_shared<PpmLogger>("log");
        
        /**
         * @brief Exit flag
         * 
         */
        bool exit_eof = false;

        /**
         * @brief Counter for EOF
         * 
         */
        int eof_cnt = 0;

        /**
         * @brief Counter for partition
         * 
         */
        int partition_cnt = 0;

        /**
         * @brief Counter for messages
         * 
         */
        long msg_cnt = 0;

        /**
         * @brief Number of bytes in messages
         * 
         */
        int64_t msg_bytes = 0;

        /**
         * @brief Verbosity level
         * 
         */
        int verbosity = 1;

        /**
         * @brief Retrieve environment variable
         * 
         * @param variableName The name of the environment variable
         * @return const char* The value of the environment variable
         */
        const char* getEnvironmentVariable(const char* variableName);
};

#endif // CVDP_KAFKA_CONSUMER_H