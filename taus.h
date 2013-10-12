#ifndef _TAUS_H
#define _TAUS_H

#include <stdio.h>
#include "tau.h"
//#include "neuralNetwork.h"

class TAUS {

protected:
	TAU* tau;
	int clientList[3];
public:
	TAUS(void);
	TAUS(ifstream* inFile);
	~TAUS(void);
	void save(ofstream* outFile);
	double score(NEURAL_NETWORK* genome);
	bool readyToPredict(void);
	void storePref(int pid, int firstID, int secondID, int pref);
//	void userModelsReset(void);
	void controllersSavePair(int pid, OPTIMIZER* optimizer, ofstream* outFile);
};

#endif
