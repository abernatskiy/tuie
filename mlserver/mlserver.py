#!/usr/bin/python2

import numpy as np
import sklearn.linear_model
import sys
import re
import os

class Predictor:
	def __init__(self, fninput = 'input', fnoutput = 'output'):
		print 'Init...'
		self.fninput = fninput
		self.fnoutput = fnoutput
		self.clf = sklearn.linear_model.LogisticRegression()

	def train(self):
		print 'Training...'
		trd = np.loadtxt(self.finput, dtype=np.double)
		self.finput.close()
		X = trd[:,:-1]
		Y = trd[:,-1]
		print X
		print Y
		self.clf.fit(X,Y)
		foutput = open(self.fnoutput, 'w')
		foutput.write('OK\n');
		foutput.close()

	def predict(self):
#		print 'Predicting...'
		pri = np.loadtxt(self.finput, dtype=np.double)
		self.finput.close()
		pro = self.clf.predict(pri)
#		print pro
		sys.stdout.write('.')
		foutput = open(self.fnoutput, 'w')
		np.savetxt(foutput, pro, fmt='%le')
		foutput.close()

	def serve(self):
		while True:
			self.finput = open(self.fninput, 'r')
			command = self.finput.readline()
			if command == 'train\n':
				pred.train()
			elif command == 'predict\n':
				pred.predict()
			elif command == 'exit\n':
				self.finput.close()
				break

if __name__=="__main__":
	if len(sys.argv) > 1:
		suffix = sys.argv[1]
	else:
		suffix = ''
	path_to_mlserver = re.sub('mlserver.py', '', os.path.abspath(sys.argv[0]))
	pred = Predictor(fninput=path_to_mlserver+'input'+suffix, fnoutput=path_to_mlserver+'output'+suffix)
	pred.serve()
