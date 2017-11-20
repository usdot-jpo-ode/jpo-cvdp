#!/usr/bin/env python3

from __future__ import print_function
from json import loads

import sys

def print_bsm_data(d):
    if d['metadata']['payloadType'] == 'us.dot.its.jpo.ode.model.OdeTIMPayload':
        speed = d['metadata']['receivedDetails']['location']['speed']
        lat = d['metadata']['receivedDetails']['location']['latitude']
        lng = d['metadata']['receivedDetails']['location']['longitude']

        print('Producing TIMS with speed={}, position={}, {}'.format(speed, lat, lng), file=sys.stderr)

        return
    elif d['metadata']['payloadType'] == 'us.dot.its.jpo.ode.model.OdeBsmPayload':
        id_ = d['payload']['data']['coreData']['id']
        speed = d['payload']['data']['coreData']['speed']
        lat = d['payload']['data']['coreData']['position']['latitude']
        lng = d['payload']['data']['coreData']['position']['longitude']

        if not d['payload']['data']['coreData']['size']: 
            print('Consuming BSM with ID={}, speed={}, position={}, {}'.format(id_, speed, lat, lng))

            return

        length = 0
        width = 0

        if d['payload']['data']['coreData']['size']['length']:
            length = d['payload']['data']['coreData']['size']['length']

        if d['payload']['data']['coreData']['size']['width']:
            width = d['payload']['data']['coreData']['size']['width']

        print('Producing BSM with ID={}, speed={}, position={}, {}, size=l:{}, w:{}'.format(id_, speed, lat, lng, length, width), file=sys.stderr)

        return

    raise IOError()


for l in sys.stdin:
    try:
        print_bsm_data(loads(l)) 
    except Exception as e:
        continue

    sys.stdout.write(l)
