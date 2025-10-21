#!/usr/bin/env python3

from __future__ import print_function
from json import loads

import sys

def print_bsm_data(d):
    id_ = d['payload']['data']['value']['BasicSafetyMessage']['coreData']['id']
    speed = d['payload']['data']['value']['BasicSafetyMessage']['coreData']['speed'] * 0.02
    lat = d['payload']['data']['value']['BasicSafetyMessage']['coreData']['lat'] * 1e-7
    lng = d['payload']['data']['value']['BasicSafetyMessage']['coreData']['long'] * 1e-7

    if not d['payload']['data']['value']['BasicSafetyMessage']['coreData']['size']: 
        print('Consuming BSM with ID={}, speed={}, position={}, {}'.format(id_, speed, lat, lng))

        return

    length = 0
    width = 0

    if d['payload']['data']['value']['BasicSafetyMessage']['coreData']['size']['length']:
        length = d['payload']['data']['value']['BasicSafetyMessage']['coreData']['size']['length']

    if d['payload']['data']['value']['BasicSafetyMessage']['coreData']['size']['width']:
        width = d['payload']['data']['value']['BasicSafetyMessage']['coreData']['size']['width']

    print('Consuming BSM with ID={}, speed={}, position={}, {}, size=l:{}, w:{}'.format(id_, speed, lat, lng, length, width))

for l in sys.stdin:
    try:
        print_bsm_data(loads(l)) 
    except Exception as e:
        continue
