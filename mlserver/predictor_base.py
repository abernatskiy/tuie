#!/usr/bin/python2

import numpy as np
import sys

class PredictorBase:
	def __init__(self, fninput = 'input', fnoutput = 'output'):
		print 'Init...'
		self.fninput = fninput
		self.fnoutput = fnoutput

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
				self.train()
			elif command == 'predict\n':
				self.predict()
			elif command == 'exit\n':
				print 'Exiting'
				self.finput.close()
				break
