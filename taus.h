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

	int recentAmbiguities, recentConflicts;

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

	int typeOfLastScore;	// -1	TAU_NO_SCORE
												// 0	no scores returned yet
												// 1	score from tau[0]
												// 2	score from tau[1]
												// 3	max(score(tau[0]), score(tau[1]))
												// 4	score from common tau (tau[2])
												// 5	score from common tau, when other taus are not functional
	void writeScoreType(int generation, double time);

private:
	int indexByPID(int pid);
};

#endif
