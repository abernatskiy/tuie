#!/usr/bin/python2

import sys
import re
import os
from predictor_decisionTree import Predictor
#from predictor_logistic import Predictor

if __name__=="__main__":
	if len(sys.argv) > 1:
		suffix = sys.argv[1]
	else:
		suffix = ''
	path_to_mlserver = re.sub('mlserver.py', '', os.path.abspath(sys.argv[0]))
	pred = Predictor(fninput=path_to_mlserver+'input'+suffix, fnoutput=path_to_mlserver+'output'+suffix)
	pred.serve()
