#!/usr/bin/env python3

from __future__ import print_function
from json import loads

import sys

def print_bsm_data(d):
    if d['metadata']['payloadType'] == 'us.dot.its.jpo.ode.model.OdeTIMPayload':
        speed = d['metadata']['receivedDetails']['location']['speed']
        lat = d['metadata']['receivedDetails']['location']['latitude']
        lng = d['metadata']['receivedDetails']['location']['longitude']

        print('Consuming TIMS with speed={}, position={}, {}'.format(speed, lat, lng))

        return
    elif d['metadata']['payloadType'] == 'us.dot.its.jpo.ode.model.OdeBsmPayload':
        id_ = d['payload']['data']['coreData']['id']
        speed = d['payload']['data']['coreData']['speed']
        lat = d['payload']['data']['coreData']['position']['latitude']
        lng = d['payload']['data']['coreData']['position']['longitude']

        print('Consuming BSM with ID={}, speed={}, position={}, {}'.format(id_, speed, lat, lng))

        return

    raise IOError()

for l in sys.stdin:
    try:
        print_bsm_data(loads(l)) 
    except Exception as e:
        continue
