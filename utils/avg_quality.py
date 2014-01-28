#! /usr/bin/python2

import sys
import numpy as np

rawarr = []

f = open(sys.argv[1], 'r')
for line in f:
	rawarr.append(tuple(map(float, line.split())))
f.close()

arr = map(list, zip(*rawarr))
quality = []

for i in range(0, len(arr[0])):
	mingap = arr[4][i] if arr[4][i] < arr[5][i] else arr[5][i]
	quality.append(mingap/arr[6][i])

npq = np.array(quality)
print npq.mean()
