#include "myHashPartitionerCb.h"

int32_t MyHashPartitionerCb::partitioner_cb (const RdKafka::Topic *topic, const std::string *key, int32_t partition_cnt, void *msg_opaque) {
    return djb_hash(key->c_str(), key->size()) % partition_cnt;
}

inline unsigned int MyHashPartitionerCb::djb_hash (const char *str, size_t len) {
    unsigned int hash = 5381;
    for (size_t i = 0 ; i < len ; i++)
        hash = ((hash << 5) + hash) + str[i];
    return hash;
}