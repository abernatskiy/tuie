#!/usr/bin/python2

import numpy as np
import sys

from sklearn.ensemble import RandomForestClassifier
from predictor_base import PredictorBase

class Predictor(PredictorBase):
	def __init__(self, fninput = 'input', fnoutput = 'output'):
		PredictorBase.__init__(self, fninput, fnoutput)
		self.clf = RandomForestClassifier()
