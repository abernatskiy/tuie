#!/usr/bin/env python2

# calculates mean value and standard deviation of a single-column file contents

import fileinput
import numpy as np

arr = []
for line in fileinput.input():
	arr.append(float(line))

narr = np.array(arr)
print str(narr.mean()) + " " + str(narr.std())
