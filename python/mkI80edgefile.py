import sys

sys.stdout.write('type,id,geography,attributes\n')

next(sys.stdin)
prevs = next(sys.stdin).strip().split(',')
prev_lat = prevs[1]
prev_lng = prevs[2]

for i, l in enumerate(sys.stdin):
    currs = l.strip().split(',')
    curr_lat = currs[1]
    curr_lng = currs[2]
    sys.stdout.write('edge,{},{};{};{}:{};{};{},way_type=user_defined:way_id=80\n'.format(i, i, prev_lat, prev_lng, i + 1, curr_lat, curr_lng))
    prev_lat = curr_lat
    prev_lng = curr_lng
