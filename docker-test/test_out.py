#!/usr/bin/env python3

from __future__ import print_function
from json import loads

import sys

def print_bsm_data(d):
    id_ = d['payload']['data']['coreData']['id']
    speed = d['payload']['data']['coreData']['speed']
    lat = d['payload']['data']['coreData']['position']['latitude']
    lng = d['payload']['data']['coreData']['position']['longitude']

    print('Consuming BSM with ID={}, speed={}, position={}, {}'.format(id_, speed, lat, lng))

for l in sys.stdin:
    try:
        print_bsm_data(loads(l)) 
    except Exception as e:
        continue
