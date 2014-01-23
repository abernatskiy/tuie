#! /usr/bin/python2

import sys

rawarr = []

f = open(sys.argv[1], 'r')
for line in f:
	rawarr.append(tuple(map(float, line.split())))
f.close()

arr = map(list, zip(*rawarr))

for i in range(len(arr[0])-1, 0, -1):
	if arr[4][i] != 0.0:
		winidx = i
		wintime = arr[7][i]
		print wintime
		break
