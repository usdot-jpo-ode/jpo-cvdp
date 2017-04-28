#!/usr/bin/env python3

from __future__ import print_function
from json import loads

import sys

def print_bsm_data(d):
    id_ = d['coreData']['id']
    speed = d['coreData']['speed']
    lat = d['coreData']['position']['latitude']
    lng = d['coreData']['position']['longitude']

    print('Producing BSM with ID={}, speed={}, position={}, {}'.format(id_, speed, lat, lng), file=sys.stderr)

for l in sys.stdin:
    try:
        print_bsm_data(loads(l)) 
    except Exception as e:
        continue

    sys.stdout.write(l)
