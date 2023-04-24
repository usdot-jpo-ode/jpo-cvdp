#ifndef CVDP_EXAMPLE_CONSUME_CB_H
#define CVDP_EXAMPLE_CONSUME_CB_H

#include "librdkafka/rdkafkacpp.h"

/**
 * NOTE: This is supposed to be a little more efficient.
 */
class ExampleConsumeCb : public RdKafka::ConsumeCb {
    public:
        void consume_cb(RdKafka::Message &msg, void *opaque);
};

#endif // CVDP_EXAMPLE_CONSUME_CB_H