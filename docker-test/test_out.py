#!/usr/bin/env python3

from __future__ import print_function
from json import loads

import sys

def print_bsm_data(d):
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

    print('Consuming BSM with ID={}, speed={}, position={}, {}, size=l:{}, w:{}'.format(id_, speed, lat, lng, length, width))

for l in sys.stdin:
    try:
        print_bsm_data(loads(l)) 
    except Exception as e:
        continue
