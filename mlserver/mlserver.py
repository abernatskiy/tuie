#!/usr/bin/python2

import sys
import re
import os

if len(sys.argv) > 2:
	if sys.argv[2] == 'decisionTree':
		print 'Using predictor ' + sys.argv[2]
		from predictor_decisionTree import Predictor
	elif sys.argv[2] == 'logistic':
		print 'Using predictor ' + sys.argv[2]
		from predictor_logistic import Predictor
	elif sys.argv[2] == 'linearSVC':
		print 'Using predictor ' + sys.argv[2]
		from predictor_linearSVC import Predictor
	elif sys.argv[2] == 'predictor_nearestNeighb':
		print 'Using predictor ' + sys.argv[2]
		from predictor_nearestNeighb import Predictor
	elif sys.argv[2] == 'predictor_rbfSVC':
		print 'Using predictor ' + sys.argv[2]
		from predictor_rbfSVC import Predictor
	elif sys.argv[2] == 'randomForest':
		print 'Using predictor ' + sys.argv[2]
		from predictor_randomForest import Predictor
	else:
		raise ValueError('I dont have such predictor')
else:
	print 'Defaulting to logistic regression'
	from predictor_logistic import Predictor

if __name__=="__main__":
	if len(sys.argv) > 1:
		suffix = sys.argv[1]
	else:
		suffix = ''
	path_to_mlserver = re.sub('mlserver.py', '', os.path.abspath(sys.argv[0]))
	pred = Predictor(fninput=path_to_mlserver+'input'+suffix, fnoutput=path_to_mlserver+'output'+suffix)
	pred.serve()
