# Data Flow Diagram
The purpose of this diagram is to show how data flows through the Privacy Protection Module.

## Key Explanation
- The blue rectangles are C++ classes that belong to this repository.
- The yellow ovals are kafka topics that the PPM consumes from and produces to.

## Data Flow Explanation
1. The PPM pulls from the Asn1 Input Topic and passes the messge to the BSMHandler class.
1. Throughout the flow, if the BSMHandler encounters a parse error or missing data, the message will be dropped.
1. If the velocity filter flag is active, the message will be passed to the VelocityFilter class.
1. If the velocity is outside the expected range, the message will be dropped.
1. If the geofence filter flag is active and the message is outside the geofence, it will be dropped.
1. If the id redact flag is active, the message will be passed to the IdRedactor class.
1. The IdRedactor class will redact the id if it is found.
1. The message is pushed to the Asn1 Output topic.

### PartII Redaction
It should be noted that while not shown here, the partII field undergoes redaction if the partII redact flag is active. The relevant code lives in the BSMHandler class at this time.