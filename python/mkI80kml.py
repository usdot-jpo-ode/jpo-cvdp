import sys

from kmlutil import *

next(sys.stdin)
prevs = next(sys.stdin).strip().split(',')
prev_lat = prevs[2]
prev_lng = prevs[3]

kml = initkml('I_80')
style = initstyle(kml, 'edges')
addlinestyle(style, '#ffff0000', 7)

for i, l in enumerate(sys.stdin):
    currs = l.strip().split(',')
    curr_lat = currs[2]
    curr_lng = currs[3]
    addsegment(kml, 'edges', float(prev_lat), float(prev_lng), float(curr_lat), float(curr_lng), name=str(i))
    prev_lat = curr_lat
    prev_lng = curr_lng


writekml(kml, 'I_80.kml')
