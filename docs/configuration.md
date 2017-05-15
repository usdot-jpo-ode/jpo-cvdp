# PPM Operation

The PPM suppresses BSMs and redacts BSM fields based on several conditions. These conditions are determined by a set of configuration parameters. The following conditions will result in a BSM being suppressed, or deleted, from the stream.

1. BSM JSON record cannot be parsed.
2. BSM speed is outside of prescribed limits.
3. BSM location is outside of a prescribed geofence.
4. BSM TemporaryID can be redacted (rendered indistinct).

# PPM Deployment

Once the PPM is installed and configured it operates as a background service. The following command will start the service:

```
$ ./bsmjson_privacy -c <configuration file>
```

# PPM Configuration

The PPM configuration file is a text file with a specific format. It can be used to configure Kafka as well as the PPM.
Comments can be added to the configuration file by starting a line with the '#' character. Configuration lines consist
of two strings separated by a '=' character; lines are terminated by newlines. The names of configuration files can be
anything; extensions do not matter.

The following is an example of a portion of a configuration file:

    # Configuration details for privacy ID redaction.
    privacy.redaction.id=ON
    privacy.redaction.id.value=FFFFFFFF
    privacy.redaction.id.inclusions=ON
    privacy.redaction.id.included=BEA10000,BEA10001

Example configuration files can be found in the [jpo-cvdp/config](../config) directory, e.g., [example.properties](../config/example.properties) is an example of a complete configuration file.

The details of the settings and how they affect the function of the PPM follow:

## Velocity Filtering

- `privacy.filter.velocity` : enables or disables BSM filtering based on the speed within the BSM.
    - `ON` : enables BSM filtering.
    - Any other value : disables BSM filtering.

- `privacy.filter.velocity.min` : *When velocity fitering is enabled*, BSMs having velocities below this value will be
  suppressed. The units are in meters per second.

- `privacy.filter.velocity.max` : *When velocity fitering is enabled*, BSMs having velocities above this value will be
  suppressed. The units are in meters per second.

## Identifier Redaction

- `privacy.redaction.id` : enables or disables the PPM's redaction function for the BSM `id` field (`TemporaryID` field in J2735).
    - `ON` : enables redaction
    - Any other value : disables redaction.

- `privacy.redaction.id.value` : *If redaction is enabled*, this value will replace the current value in the `id` field of the raw BSM.
    - According to the J2735, this value is 4 hexidecimal-encoded bytes. The configured value should **NOT** be
      enclosed in quotes or be preceded by 0x.

- `privacy.redaction.id.inclusions` : *If redaction is enabled*, this parameter enables or disables the ability to specify
   **which** identifier values should be redacted.
    - `ON` : enables use of a redaction *inclusion* set. The values in the set are defined in
      the `privacy.redaction.id.included` configuration parameter.
    - Any other value : **causes all identifiers to be redacted.** Ignores the inclusion set.

- `privacy.redaction.id.included` : *If redaction and redaction inclusions are enabled*, the parameter is the list of BSM
   identifiers (right now TemporaryID) that **will be redacted**; BSMs having identifiers that are not in this set will remain in
   the BSM output by the PPM if retained.
    - Similar to the `privacy.redaction.id.value`, these are 4 hexadecimal-encoded bytes.
    - More than one id can be specified by separating them by commas.

## Geofencing

BSM records can be suppressed based on their J2735 Part I latitude and longitude
attributes. If this capability is turned one through the configuration file,
each edge defined in the map file is used to infer a *component* geofence that
surrounds that segment of the road. The image below illustrates how a *rectange*
is drawn to form the segment's geofence.  The aforementioned edge attributes and
PPM configuration parameters determine the size of the rectange.

![Road Segment Geofence Dimensions](graphics/geofence-dimensions.png){ width:50%; }

- `privacy.filter.geofence` : enables or disables geofence-based filtering.
    - `ON` : enables the geofence.
    - Any other value : disables geofence filtering.

- `privacy.filter.geofence.mapfile` : *If geofence filtering is enabled*, specifies the absolute or relative path and filename of a file that contains the
  map information needed to define the geofence.

- `privacy.filter.geofence.extension` : *If geofence filtering is enabled*, this is one
  of the controls that determines the size of the component geofences that
  surround road segments. See the [Map Files](#geofencing) section.

### Geofence Region Boundaries

Geofence Boundary Configuration Parameters: The geofence is stored in a geographically-defined data structured called
a quadtree. The following bounding box coordinates define the quadtree's region. The data that is stored in this data
structure is limited to those segments provided in the mapfile, e.g., `privacy.filter.geofence.mapfile`. As an example
of this relationship, the coordinates specified below could bound the entire state of Wyoming; however, only the
segments for the I-80 corridor would be stored within a quadtree covering Wyoming and used to define the geofence. One
the other hand, these coordinates can be used to **further restrict** which segments are used to define the geofence
instead of having to modify the mapfile.

- `privacy.filter.geofence.sw.lat` : The latitude of the lower-left corner of the quadtree region.
- `privacy.filter.geofence.sw.lon` : The longitude of the lower-left corner of the quadtree region.
- `privacy.filter.geofence.ne.lat` : The latitude of the upper-right corner of the quadtree region.
- `privacy.filter.geofence.ne.lon` : The longitude of the upper-right corner of the quadtree region.

## ODE Kafka Interface

- `privacy.topic.consumer` : The Kafka topic name used by the Operational Data Environment (or other BSM JSON producer) that will be
  consumed by the PPM. The source of the data stream to be filtered by the PPM.
- `privacy.topic.producer` : The Kafka topic name where the PPM will write the filtered BSMs.

- `group.id` : The group identifier for the PPM consumer.  Consumers label
  themselves with a consumer group name, and each record published to a topic is
  delivered to one consumer instance within each subscribing consumer group.
  Consumer instances can be in separate processes or on separate machines.

- `privacy.kafka.partition` : The partition(s) that this PPM will consume records from. A Kafka topic can be divided,
  or partitioned, into several "parallel" streams. A topic may have many partitions so it can handle an arbitrary
  amount of data.

- `metadata.broker.list` : This is the IP address of the Kafka topic broker leader.

- `compression.type` : The type of compression to use for writing to Kafka topics. Currently, this should be set to none.

# Map Files

The map file is used to define the geofence. It defines a set of shapes, one
per line. For road geofence use, the edge shape is used. The map file for the
I-80 WYDOT corridor is located in the [jpo-cvdp/data](../data) directory; it is named: `I_80.edges`

The following is a small portion of the `I_80.edges` file:

```bash
type,id,geography,attributes
edge,0,0;41.24789403;-111.0467118:1;41.24746145;-111.0455124,way_type=user_defined:way_id=80
edge,1,1;41.24746145;-111.0455124:2;41.24733395;-111.0451337,way_type=user_defined:way_id=80
edge,2,2;41.24733395;-111.0451337:3;41.24726205;-111.044904,way_type=user_defined:way_id=80
edge,3,3;41.24726205;-111.044904:4;41.24713975;-111.0444827,way_type=user_defined:way_id=80
```

This file has four comma-separated elements:

- type : `edge`
- shape identifier : unique 64-bit integer identifier
- geography : A sequence of colon-split triples representing points; each point is semi-colon split as follows:
    - `<point uid>;<latitude>;<longitude>`
- attributes : A sequence of colon-split `key=value` attributes.
    - The attribute `way_type` determines the width of the geofence around a road segment.

For the WYDOT use case, WYDOT provided a set of edge definitions for I-80 that were converted into the above format.
