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
				tau[2]->Ready_To_Predict() )
			return tau[2]->Score_Predict(genome);
		return fmax( tau[0]->Score_Predict(genome),
								tau[1]->Score_Predict(genome) );
	}
	else {
		if(tau[0]->Ready_To_Predict())
			return tau[0]->Score_Predict(genome);
		if(tau[1]->Ready_To_Predict())
			return tau[1]->Score_Predict(genome);
		if(tau[2]->Ready_To_Predict())
			return tau[2]->Score_Predict(genome);
		return TAU_NO_SCORE;
	}
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

	int idx = indexByPID(pid);
	tau[idx]->Controllers_Save_Pair(optimizer, outFile);
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
