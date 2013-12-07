#include <stdlib.h>

#ifndef _TAUS_CPP
#define _TAUS_CPP

#include "taus.h"

extern double TAU_NO_SCORE;
extern double TAU_OPTIMIZER_ERROR_UNDEFINED;

TAUS::TAUS(void) {

	for(int i=0; i<3; i++)
		tau[i] = new TAU;
	noOfClients = 0;
	clientList[0] = 0;
	clientList[1] = 0;

	typeOfLastScore = 0;
}

/*
TAUS::TAUS(ifstream* inFile) {

	for(int i=0; i<3; i++)
		tau[i] = new TAU(inFile);
}*/

TAUS::~TAUS(void) {

	for(int i=0; i<3; i++)
		delete tau[i];
}

/*
void TAUS::save(ofstream* outFile) {

	for(int i=0; i<3; i++)
		tau[i]->Save(outFile);
}*/

double TAUS::score(NEURAL_NETWORK* genome) {

	double score[3];
	double error[3];
	bool ready[3];

	for(int i=0; i<3; i++) {
		ready[i] = tau[i]->Ready_To_Predict();
		if( ready[i] ) {
			score[i] = tau[i]->Score_Predict(genome);
			error[i] = tau[i]->Model_Error();
		}
		else {
			score[i] = TAU_NO_SCORE;
			error[i] = TAU_OPTIMIZER_ERROR_UNDEFINED;
		}
	}

	if( ready[0] && ready[1] ) {
		if( ready[2] &&
				error[0] > error[2] &&
				error[1] > error[2] ) {
			typeOfLastScore = 4;
			return score[2];
		}
		else {
			typeOfLastScore = 3;
			return fmax( score[0], score[1] );
		}
	}
	else {
		if(ready[0]) {
			typeOfLastScore = 1;
			return score[0];
		}
		if(ready[1]) {
			typeOfLastScore = 2;
			return score[1];
		}
		if(ready[2]) {
			typeOfLastScore = 5;
			return score[2];
		}
		typeOfLastScore = -1;
		return TAU_NO_SCORE;
	}
}

void TAUS::writeScoreType(int generation) {

	FILE* outFile = fopen("SavedFiles/score_type.log", "a");
	fprintf(outFile, "%d\t%le\t%le\t%le\t%le\t%le\t%le\t%d\n", generation,
		tau[0]->Model_Error(), tau[1]->Model_Error(), tau[2]->Model_Error(),
		1.0/((double) tau[0]->numControllers), 1.0/((double) tau[1]->numControllers), 1.0/((double) tau[2]->numControllers),
		typeOfLastScore);
	fclose(outFile);
}

bool TAUS::readyToPredict(void) {

	if( tau[0]->Ready_To_Predict() || tau[1]->Ready_To_Predict() || tau[2]->Ready_To_Predict() )
		return true;
	return false;
}

void TAUS::storePref(int pid, int firstID, int secondID, int pref) {

	int idx = indexByPID(pid);
	printf("TAUS: feeding %d %d %d to tau[%d]\n", firstID, secondID, pref, idx);
	tau[idx]->Store_Pref(firstID, secondID, pref);
//	tau[idx]->Scores_Check();

	int otherIdx = idx==0 ? 1 : 0;
	if(tau[otherIdx]->Ready_To_Predict())
	{
		printf("TAUS: merging individual TAUs to produce a common one\n");
//		tau[otherIdx]->Scores_Check();
		tau[2] = new TAU(tau[0], tau[1]);
		// normally Optimize() function (which performs backpropagation) is called form Store_Pref, but it's never used on common TAU
		// this is the reason why it is called here
		tau[2]->Optimize();
	}

//	printf("TAUS: feeding %d %d %d to the common TAU\n", firstID, secondID, pref);
//	tau[2]->Store_Pref(firstID, secondID, pref);
}

void TAUS::controllersSavePair(int pid, OPTIMIZER* optimizer, ofstream* outFile) {

	NEURAL_NETWORK** savedControllers;
	if( pid > 0 ) {
		int idx = indexByPID(pid);
		savedControllers = tau[idx]->Controllers_Save_Pair(optimizer, outFile);
	}
	else {
		savedControllers = tau[0]->Controllers_Save_Pair(optimizer, outFile);
		tau[1]->Controller_Store(savedControllers[0]);
		tau[1]->Controller_Store(savedControllers[1]);
	}

//	tau[2]->Controller_Store(savedControllers[0]);
//	tau[2]->Controller_Store(savedControllers[1]);
	delete [] savedControllers;
}

/*** private methods ***/

int TAUS::indexByPID(int pid) {

	if(clientList[0] == pid)
		return 0;
	if(clientList[1] == pid)
		return 1;

	if(noOfClients<2) {
		clientList[noOfClients] = pid;
		return noOfClients++;
	}
	printf("No more than two clients allowed.\n");
	exit(1);
	return -9999;
}

#endif
