import sys

sys.stdout.write('type,id,geography,attributes\n')

ID = sys.argv[1]

try:
    offset = int(sys.argv[2]) if len(sys.argv) > 2 else 0
except Exception:
    offset = 0

next(sys.stdin)
prevs = next(sys.stdin).strip().split(',')
prev_lat = prevs[2]
prev_lng = prevs[3]

for i, l in enumerate(sys.stdin):
    edgeid = offset + i
    currs = l.strip().split(',')
    curr_lat = currs[2]
    curr_lng = currs[3]
    sys.stdout.write('edge,{},{};{};{}:{};{};{},way_type=user_defined:way_id={}\n'.format(edgeid, edgeid, prev_lat, prev_lng, edgeid + 1, curr_lat, curr_lng, ID))
    prev_lat = curr_lat
    prev_lng = curr_lng
