#!/usr/bin/python2

import os
import sys

filenames = []

for ent in os.listdir(sys.argv[1]):
	if 'surrogate' in ent:
		filenames.append(ent)

prefs = {}
coords = {}

for filename in filenames:
	prefs[filename] = {}
	coords[filename] = {}
	fd = open(sys.argv[1] + '/' + filename, 'r')
	for line in fd:
		vals = line.split()
		prefs[filename][(int(vals[0]),int(vals[1]))] = int(vals[4])
		coords[filename][(int(vals[0]),int(vals[1]))] = vals[2] + ' ' + vals[3]
	fd.close()

allied = 0
opposing = 0

inv_allied = 0
inv_opposing = 0

for filename in prefs:
	others = list(prefs.keys())
	others.remove(filename)
	for pair in prefs[filename]:
		for other in others:
			value = prefs[filename][pair]
			if pair in prefs[other]:
				if prefs[other][pair] == value:
					allied += 1
				elif prefs[other][pair] != value:
					opposing += 1
#					print('Found identical pairs ' + str(pair) + '. Coordinates: ' + coords[filename][pair] + ' in ' + filename + ', ' + coords[other][pair] + ' in ' + other)
				else:
					raise ValueError('Not equal nor unequal')
			cont1, cont2 = pair
			antipair = (cont2, cont1)
			antivalue = 0 if value==1 else 1
			if antipair in prefs[other]:
				if prefs[other][antipair] == antivalue:
					allied += 1
					inv_allied += 1
				elif prefs[other][antipair] != antivalue:
					opposing += 1
					inv_opposing += 1
#					print('Found inverse pairs ' + str(pair) + ', ' + str(antipair) + \
#						'. Coordinates:\n ' + \
#						coords[filename][pair] + ' in ' + filename + ', score ' + str(prefs[filename][pair]) + '\n ' + \
#						coords[other][antipair] + ' in ' + other + ', score ' + str(prefs[other][antipair]))
				else:
					raise ValueError('Not equal nor unequal')

print("Scores: allied - " + str(allied) + "; opposing - " + str(opposing) + ", among them " + str(inv_allied) + " allied and " + str(inv_opposing) + " opposing are found for inverted pairs")
