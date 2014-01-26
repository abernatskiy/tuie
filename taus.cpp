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

	recentAmbiguities = 0;
	recentConflicts = 0;
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

	if( ready[2] ) {
		typeOfLastScore = 4;
		return tau[2]->Score_Predict(genome);
	}
	else {
		typeOfLastScore = -1;
		return 1.0;
	}

/*	for(int i=0; i<3; i++) {
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
	}*/
}

void TAUS::writeScoreType(int generation, double time) {

	FILE* outFile = fopen("SavedFiles/score_type.log", "a");
	fprintf(outFile, "%d\t%le\t%le\t%le\t%le\t%le\t%le\t%d\t%d\t%d\t%le\n", generation,
		tau[0]->Model_Error(), tau[1]->Model_Error(), tau[2]->Model_Error(),
		1.0/((double) tau[0]->Controllers_Available_For_Optimization()), 1.0/((double) tau[1]->Controllers_Available_For_Optimization()), 1.0/((double) tau[2]->numControllers),
		recentAmbiguities, recentConflicts,
		typeOfLastScore, time);
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

	if( tau[otherIdx]->Ready_To_Predict() &&
			tau[idx]->numControllers == tau[idx]->Controllers_Available_For_Optimization() )
	{
		printf("TAUS: merging individual TAUs to produce a common one\n");
//		tau[otherIdx]->Scores_Check();
//		TAU* commonTAU = new TAU(tau[idx], tau[otherIdx]); // order of TAUs may be important here - in case of ambiguity TAU(TAU* tau0, TAU* tau1) will ask tau0 to resolve it
		TAU* commonTAU = new TAU(tau[0], tau[1]); // order of TAUs may be important here - in case of ambiguity TAU(TAU* tau0, TAU* tau1) will ask tau0 to resolve it
		// no need to optimize anything - common TAU comes with batteries included

		recentAmbiguities = commonTAU->ambiguities;
		recentConflicts = commonTAU->conflicts;
		if( commonTAU->ambiguities == 0 ) {
			delete tau[2];
			tau[2] = commonTAU;
		}
		else {
			printf("TAUS: ditching ambiguous common TAU till it gets better\n");
			delete commonTAU;
		}
	}
}

void TAUS::controllersSavePair(int pid, OPTIMIZER* optimizer, ofstream* outFile) {

	NEURAL_NETWORK** savedControllers;
	if( pid > 0 ) { // if we are not at the first iteration, ignore pointers to saved controllers returned by Controllers_Save_Pair()
		int idx = indexByPID(pid);
		savedControllers = tau[idx]->Controllers_Save_Pair(optimizer, outFile);
	}
	else { // if we are at the first iteration, use pointers to store the controllers in another tau
		savedControllers = tau[0]->Controllers_Save_Pair(optimizer, outFile);
		savedControllers = tau[1]->Controllers_Save_Pair(optimizer, tau[0], outFile);
//		tau[1]->Controller_Store(savedControllers[0]);
//		tau[1]->Controller_Store(savedControllers[1]);
	}

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
