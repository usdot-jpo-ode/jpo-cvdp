#ifndef CVDP_MY_HASH_PARTITIONER_CB_H
#define CVDP_MY_HASH_PARTITIONER_CB_H

#include "librdkafka/rdkafkacpp.h"

/* Use of this partitioner is pretty pointless since no key is provided
 * in the produce() call. */
class MyHashPartitionerCb : public RdKafka::PartitionerCb {
    public:
        int32_t partitioner_cb (const RdKafka::Topic *topic, const std::string *key,int32_t partition_cnt, void *msg_opaque);
    
    private:
        inline unsigned int djb_hash (const char *str, size_t len);
};

#endif // CVDP_MY_HASH_PARTITIONER_CB_H