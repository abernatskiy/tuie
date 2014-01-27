#!/usr/bin/env python2

# calculates mean value and standard deviation of a single-column file contents

import fileinput
import numpy as np

arr = []
for line in fileinput.input():
	arr.append(float(line))

narr = np.array(arr)
print "Mean " + str(narr.mean()) + " std " + str(narr.std())

tmarr = np.array([])

for i in range(0,100):
	parr = np.random.permutation(narr)
	marr = np.array(np.minimum(narr, parr))
	tmarr = np.append(tmarr, marr)

print "100 mins with a self-permutation: mean " + str(tmarr.mean()) + " std " + str(tmarr.std())
