#ifndef _TAUS_H
#define _TAUS_H

#include <stdio.h>
#include "tau.h"
//#include "neuralNetwork.h"

class TAUS {

protected:
	TAU* tau[3]; // 0 - user1, 1 - user2, 2 - user1&user2
	int clientList[2]; // 0 - user1, 1 - user2
	int noOfClients;

public:
	TAUS(void);
//	TAUS(ifstream* inFile);
	~TAUS(void);
//	void save(ofstream* outFile);
	double score(NEURAL_NETWORK* genome);
	bool readyToPredict(void);
	void storePref(int pid, int firstID, int secondID, int pref);
//	void userModelsReset(void);
	void controllersSavePair(int pid, OPTIMIZER* optimizer, ofstream* outFile);

	int typeOfLastScore;
	void writeScoreType(void);

private:
	int indexByPID(int pid);
};

#endif
