#!/usr/bin/python2

import numpy as np
import sklearn.linear_model
import sys

from predictor_base import PredictorBase

class Predictor(PredictorBase):
	def __init__(self, fninput = 'input', fnoutput = 'output'):
		PredictorBase.__init__(self, fninput, fnoutput)
		self.clf = sklearn.linear_model.LogisticRegression()
