#!/usr/bin/env python2

import numpy as np
import matplotlib.pyplot as plt
import sys
import os.path

def printHelp():
	print "Plots a bird's-eye-plot of .ssv, .tsv or .csv file"
	print "Usage: uplot.py <filename>"

def parseArgs():
	args = sys.argv
	if len(args) != 2:
		printHelp()
		sys.exit()
	if os.path.exists(args[1]):
		return os.path.abspath(args[1])
	else:
		printHelp()
		sys.exit()

def parseFile(fileName):
	"""Parses a file with columns (.tsv, .csv, .ssv) into lists - one list per column. Return the list of lists."""
	rawarr = []
	f = open(fileName, "r")
	for line in f:
		rawarr.append(tuple(map(float, line.split())))
	f.close()
	arr = map(list, zip(*rawarr))
	return arr

def defaultPlot(arrayOfArrays):
	"""Generates a simple plot suitable for many files consisting of columns"""
	numvar = len(arrayOfArrays) - 1
	for i in range(1, numvar+1):
		plt.subplot(numvar, 1, i)
		plt.plot(arrayOfArrays[0], arrayOfArrays[i])

def tuieSummaryPlot3(arrayOfArrays):
	"""Generates a plot meaningful for TUIE summary data, 3-column ver"""
	numvar = len(arrayOfArrays) - 1
	if numvar	!= 3:
		raise Exception("File type detection error")

	plt.subplot(311)
	plt.title("History of most fit individuals")
	plt.ylabel("Fitness-error")
	threshold = np.ones(len(arrayOfArrays[0]))*0.139
	plt.plot(arrayOfArrays[0], arrayOfArrays[1], threshold, "r--")

	plt.subplot(312)
	plt.ylabel("Score")
	plt.plot(arrayOfArrays[0], arrayOfArrays[2])

	plt.subplot(313)
	plt.ylabel("Age")
	plt.xlabel("Generation")
	plt.plot(arrayOfArrays[0], arrayOfArrays[3])

def tuieSummaryPlot4(arrayOfArrays):
	"""Generates a plot meaningful for TUIE summary data, 4-column ver"""
	numvar = len(arrayOfArrays) - 1
	if numvar	!= 4:
		raise Exception("File type detection error")

	plt.subplot(411)
	plt.title("History of most fit individuals")
	plt.ylabel("Fitness-error")
	threshold = np.ones(len(arrayOfArrays[0]))*0.139
	plt.plot(arrayOfArrays[0], arrayOfArrays[1], threshold, "r--")

	plt.subplot(412)
	threshold = np.ones(len(arrayOfArrays[0]))
	plt.ylabel("Score")
	plt.ylim(0.0, 1.1)
	plt.plot(arrayOfArrays[0], arrayOfArrays[2], threshold, "r--")

	plt.subplot(413)
	plt.ylabel("Age")
	plt.plot(arrayOfArrays[0], arrayOfArrays[3])

	ax = plt.subplot(414)
	plt.ylabel("No robots above the barrier")
	plt.ylim(-0.5, 1.5)
	ax.set_yticklabels([" ", "false", " ", "true"])
	plt.xlabel("Generation")
	plt.plot(arrayOfArrays[0], arrayOfArrays[4])

def tuieSummaryPlot6(arrayOfArrays):
	"""Generates a plot meaningful for TUIE summary data, 5-column ver"""
	numvar = len(arrayOfArrays) - 1
	if numvar	!= 6:
		raise Exception("File type detection error")

	plt.subplot(511)
	plt.title("History of most fit individuals")
	plt.ylabel("Fitness-error")
	threshold = np.ones(len(arrayOfArrays[0]))*0.139
	plt.plot(arrayOfArrays[0], arrayOfArrays[1], threshold, "r--")

	plt.subplot(512)
	threshold = np.ones(len(arrayOfArrays[0]))
	plt.ylabel("Score")
	plt.ylim(0.0, 1.1)
	plt.plot(arrayOfArrays[0], arrayOfArrays[2], threshold, "r--")

	plt.subplot(513)
	plt.ylabel("Age")
	plt.plot(arrayOfArrays[0], arrayOfArrays[3])

	ax = plt.subplot(514)
	plt.ylabel("No robots above the barrier")
	plt.ylim(-0.5, 1.5)
	ax.set_yticklabels([" ", "false", " ", "true"])
	plt.plot(arrayOfArrays[0], arrayOfArrays[4])

	plt.subplot(515)
	plt.ylabel('y coord')
	plt.xlabel("Generation")
	plt.plot(arrayOfArrays[0], arrayOfArrays[5], color='blue', label='best ind')
	plt.plot(arrayOfArrays[0], arrayOfArrays[6], color='red', label='best Y')
	plt.legend(prop={'size':12}, frameon=False)

def tuieSummaryPlot7(arrayOfArrays):
	"""Generates a plot meaningful for TUIE summary data, 5-column ver"""
	numvar = len(arrayOfArrays) - 1
	if numvar	!= 7:
		raise Exception("File type detection error")

	plt.subplot(511)
	plt.title("History of most fit individuals")
	plt.ylabel("Fitness-error")
	threshold = np.ones(len(arrayOfArrays[7]))*0.139
	plt.plot(arrayOfArrays[7], arrayOfArrays[1])
	plt.plot(arrayOfArrays[7], threshold, "r--")

	plt.subplot(512)
	threshold = np.ones(len(arrayOfArrays[7]))
	plt.ylabel("Score")
	plt.ylim(0.0, 1.1)
	plt.plot(arrayOfArrays[7], arrayOfArrays[2])
	plt.plot(arrayOfArrays[7], threshold, "r--")

	plt.subplot(513)
	plt.ylabel("Age")
	plt.plot(arrayOfArrays[7], arrayOfArrays[3])

	ax = plt.subplot(514)
	plt.ylabel("No robots above the barrier")
	plt.ylim(-0.5, 1.5)
	ax.set_yticklabels([" ", "false", " ", "true"])
	plt.plot(arrayOfArrays[7], arrayOfArrays[4])

	plt.subplot(515)
	plt.ylabel('y coord')
	plt.xlabel("Time, s")
	plt.plot(arrayOfArrays[7], arrayOfArrays[5], color='blue', label='best ind')
	plt.plot(arrayOfArrays[7], arrayOfArrays[6], color='red', label='best Y')
	plt.legend(prop={'size':12}, frameon=False)

def tuieSummaryPlot(arrayOfArrays):
	"""Generates a plot meaningful for TUIE summary data"""
	numvar = len(arrayOfArrays) - 1
	if numvar	== 3:
		tuieSummaryPlot3(arrayOfArrays)
	elif numvar == 4:
		tuieSummaryPlot4(arrayOfArrays)
	elif numvar == 6:
		tuieSummaryPlot6(arrayOfArrays)
	elif numvar == 7:
		tuieSummaryPlot7(arrayOfArrays)
	else:
		raise Exception("File type detection error")

def tuieScoreTypePlot4(arrayOfArrays):
	"""Generates a plot meaningful for TUIE score type plots"""
	numvar = len(arrayOfArrays) - 1
	if numvar	!= 4:
		raise Exception("File type detection error")

	plt.subplot(211)

	plt.axis([0.0, 2000.0, 0.0, 0.5])
	ax = plt.gca()
	ax.set_autoscale_on(False)

	plt.title("History of TAU model errors and predictive behaviour switching")
	plt.ylabel("TAU error")
	plt.plot(arrayOfArrays[0], arrayOfArrays[1], color="blue", label="tau0")
	plt.plot(arrayOfArrays[0], arrayOfArrays[2], color="cyan", label="tau1")
	plt.plot(arrayOfArrays[0], arrayOfArrays[3], color="red", label="common tau")
	plt.legend(prop={'size':12}, frameon=False)


	ax = plt.subplot(212)
	plt.ylabel("Value used as a user score est.")
	plt.ylim(0,6)
	ax.set_yticklabels([" ", "tau0", "tau1", "max", "com", " "])
	plt.xlabel("Generation")
	plt.plot(arrayOfArrays[0], arrayOfArrays[4])

def tuieScoreTypePlot7(arrayOfArrays):
	"""Generates a plot meaningful for TUIE score type plots"""
	numvar = len(arrayOfArrays) - 1
	if numvar	!= 7:
		raise Exception("File type detection error")

	plt.subplot(211)

	plt.axis([0.0, 2000.0, 0.0, 0.5])
	ax = plt.gca()
	ax.set_autoscale_on(False)

	plt.title("History of TAU model errors and predictive behaviour switching")
	plt.ylabel("TAU error")
	plt.plot(arrayOfArrays[0], arrayOfArrays[1], color="blue", label="tau0")
	plt.plot(arrayOfArrays[0], arrayOfArrays[2], color="cyan", label="tau1")
	plt.plot(arrayOfArrays[0], arrayOfArrays[3], color="red", label="common tau")
	plt.legend(prop={'size':12}, frameon=False)


	ax = plt.subplot(212)
	plt.ylabel("Value used as a user score est.")
	plt.ylim(0,6)
	ax.set_yticklabels([" ", "tau0", "tau1", "max", "com", " "])
	plt.xlabel("Generation")
	plt.plot(arrayOfArrays[0], arrayOfArrays[7])

def tuieScoreTypePlot9(arrayOfArrays):
	"""Generates a plot meaningful for TUIE score type plots"""
	numvar = len(arrayOfArrays) - 1
	if numvar	!= 9: # change
		raise Exception("File type detection error")

	plt.subplot(311)

#	plt.axis([0.0, 2000.0, 0.0, 0.5])
#	ax = plt.gca()
#	ax.set_autoscale_on(False)

#	plt.xlim(0,500)
	plt.title("History of TAU model errors and predictive behaviour switching")
	plt.ylabel("TAU error")
#	plt.plot(arrayOfArrays[0], arrayOfArrays[1], 'b', label="tau0", arrayOfArrays[2], 'c', label="tau1", arrayOfArrays[3], 'r', label="common tau")
	plt.plot(arrayOfArrays[0], arrayOfArrays[1], color="blue", label="tau0")
	plt.plot(arrayOfArrays[0], arrayOfArrays[2], color="cyan", label="tau1")
	plt.plot(arrayOfArrays[0], arrayOfArrays[3], color="red", label="common tau")
	plt.plot(arrayOfArrays[0], arrayOfArrays[4], ":", color="blue", label="tau0 gap") # comment
	plt.plot(arrayOfArrays[0], arrayOfArrays[5], ":", color="cyan", label="tau1 gap") # commment
	plt.plot(arrayOfArrays[0], arrayOfArrays[6], ":", color="red", label="common tau gap") # comment
#	plt.ylim(0,0.4)
	plt.legend(prop={'size':12}, frameon=False)

	ax = plt.subplot(312)

#	plt.xlim(0,500)

	plt.ylabel("Conflicts/ambiguities")
	plt.ylim(-0.5, 3.5)
#	ax.set_yticklabels([" ", "tau0", "tau1", "max", "common", " "])
	plt.plot(arrayOfArrays[0], arrayOfArrays[7], color="blue", label="ambiguities") # change
	plt.plot(arrayOfArrays[0], arrayOfArrays[8], color="red", label="conflicts") # change
	plt.legend(prop={'size':12}, frameon=False)

	ax = plt.subplot(313)

#	plt.xlim(0,500)

	plt.ylabel("Value used as a user score est.")
	plt.ylim(0,6)
	ax.set_yticklabels([" ", "tau0", "tau1", "max", "com", " "])

	plt.xlabel("Generation")
	plt.plot(arrayOfArrays[0], arrayOfArrays[9]) # change

def tuieScoreTypePlot10(arrayOfArrays):
	"""Generates a plot meaningful for TUIE score type plots"""
	numvar = len(arrayOfArrays) - 1
	if numvar	!= 10: 
		raise Exception("File type detection error")

	plt.subplot(311)

	plt.title("History of TAU model errors and predictive behaviour switching")
	plt.ylabel("TAU error")
	plt.plot(arrayOfArrays[10], arrayOfArrays[1], color="blue", label="tau0")
	plt.plot(arrayOfArrays[10], arrayOfArrays[2], color="cyan", label="tau1")
	plt.plot(arrayOfArrays[10], arrayOfArrays[3], color="red", label="common tau")
	plt.plot(arrayOfArrays[10], arrayOfArrays[4], ":", color="blue", label="tau0 gap") # comment
	plt.plot(arrayOfArrays[10], arrayOfArrays[5], ":", color="cyan", label="tau1 gap") # commment
	plt.plot(arrayOfArrays[10], arrayOfArrays[6], ":", color="red", label="common tau gap") # comment
	plt.legend(prop={'size':12}, frameon=False)

	ax = plt.subplot(312)

	plt.ylabel("Conflicts/ambiguities")
	plt.ylim(-0.5, 3.5)
	plt.plot(arrayOfArrays[10], arrayOfArrays[7], color="blue", label="ambiguities") # change
	plt.plot(arrayOfArrays[10], arrayOfArrays[8], color="red", label="conflicts") # change
	plt.legend(prop={'size':12}, frameon=False)

	ax = plt.subplot(313)

#	plt.xlim(0,500)

	plt.ylabel("Value used as a user score est.")
	plt.ylim(0,6)
	ax.set_yticklabels([" ", "tau0", "tau1", "max", "com", " "])

	plt.xlabel("Time, s")
	plt.plot(arrayOfArrays[10], arrayOfArrays[9]) # change

def tuieScoreTypePlot(arrayOfArrays):
	"""Generates a plot meaningful for TUIE score type plots"""
	numvar = len(arrayOfArrays) - 1
	if numvar	== 10:
		tuieScoreTypePlot10(arrayOfArrays)
	elif numvar	== 9:
		tuieScoreTypePlot9(arrayOfArrays)
	elif numvar == 7:
		tuieScoreTypePlot7(arrayOfArrays)
	elif numvar == 4:
		tuieScoreTypePlot4(arrayOfArrays)
	else:
		raise Exception("File type detection error")

def tuiePlotArray(arrayOfArrays, fileName):
	if ("summary.log" in fileName):
		tuieSummaryPlot(arrayOfArrays)
	elif ("score_type.log"):
		tuieScoreTypePlot(arrayOfArrays)
	else:
		defaultPlot(arrayOfArrays)

def plotArray(arrayOfArrays, fileName):
	if ("tuie" in fileName):
		tuiePlotArray(arrayOfArrays, fileName)
	else:
		defaultPlot(arrayOfArrays)
	plt.show()

if __name__=="__main__":
	inputFileName = parseArgs()
	input = parseFile(inputFileName)
	plotArray(input, inputFileName)
