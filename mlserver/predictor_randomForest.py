#!/usr/bin/python2

import numpy as np
from sklearn import tree
import sys

from sklearn.ensemble import RandomForestClassifier

class Predictor(PredictorBase):
	def __init__(self, fninput = 'input', fnoutput = 'output'):
		PredictorBase.__init__(self, fninput, fnoutput)
		self.clf = RandomForestClassifier()
