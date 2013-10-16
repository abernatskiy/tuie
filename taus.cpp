#include <stdlib.h>

#ifndef _TAUS_CPP
#define _TAUS_CPP

#include "taus.h"

extern double TAU_NO_SCORE;

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

	if( tau[0]->Ready_To_Predict() && tau[1]->Ready_To_Predict() ) {
		if( tau[0]->Model_Error() > tau[2]->Model_Error() &&
				tau[1]->Model_Error() > tau[2]->Model_Error() &&
				tau[2]->Ready_To_Predict() ) {
			typeOfLastScore = 4;
			return tau[2]->Score_Predict(genome);
		}
		else {
			typeOfLastScore = 3;
			return fmax( tau[0]->Score_Predict(genome),
									tau[1]->Score_Predict(genome) );
		}
	}
	else {
		if(tau[0]->Ready_To_Predict()) {
			typeOfLastScore = 1;
			return tau[0]->Score_Predict(genome);
		}
		if(tau[1]->Ready_To_Predict()) {
			typeOfLastScore = 2;
			return tau[1]->Score_Predict(genome);
		}
		if(tau[2]->Ready_To_Predict()) {
			typeOfLastScore = 5;
			return tau[2]->Score_Predict(genome);
		}
		typeOfLastScore = -1;
		return TAU_NO_SCORE;
	}
}

void TAUS::writeScoreType(void) {

	FILE* outFile = fopen("SavedFiles/score_type.log", "a");
	fprintf(outFile, "%d\n", typeOfLastScore);
	fclose(outFile);
}

bool TAUS::readyToPredict(void) {

	if( tau[0]->Ready_To_Predict() || tau[1]->Ready_To_Predict() || tau[2]->Ready_To_Predict() )
		return true;
	return false;
}

void TAUS::storePref(int pid, int firstID, int secondID, int pref) {

	int idx = indexByPID(pid);
	tau[idx]->Store_Pref(firstID, secondID, pref);
	tau[2]->Store_Pref(firstID, secondID, pref);
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

	tau[2]->Controller_Store(savedControllers[0]);
	tau[2]->Controller_Store(savedControllers[1]);
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
