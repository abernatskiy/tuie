#include <stdlib.h>

#ifndef _TAUS_CPP
#define _TAUS_CPP

#include "taus.h"

extern double TAU_NO_SCORE;
extern double TAU_OPTIMIZER_ERROR_UNDEFINED;
extern int AFPO_POP_SIZE;

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
/*	for(int i=0; i<3; i++)
		ready[i] = tau[i]->Ready_To_Predict();

	if( ready[2] ) {
		typeOfLastScore = 4;
		return tau[2]->Score_Predict(genome);
	}
	else if( ready[0] ) {
		typeOfLastScore = 1;
		return tau[0]->Score_Predict(genome);
	}
	else {
		printf("You messed up score composition. See l.62 of taus.cpp\n");
		exit(1);
	}

*/	for(int i=0; i<3; i++) {
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
	printf("TAUS: feeding %d %d %d to tau[%d] (PID %d)\n", firstID, secondID, pref, idx, pid);
	tau[idx]->Store_Pref(firstID, secondID, pref);
//	tau[idx]->Scores_Check();

	int otherIdx = idx==0 ? 1 : 0;

	if( tau[otherIdx]->Ready_To_Predict() &&
			tau[idx]->numControllers == tau[idx]->Controllers_Available_For_Optimization() )
	{
		printf("TAUS: merging individual TAUs to produce a common one\n");
//		tau[otherIdx]->Scores_Check();
//		TAU* commonTAU = new TAU(tau[idx], tau[otherIdx]); // order of TAUs may be important here - in case of ambiguity TAU(TAU* tau0, TAU* tau1) will ask tau0 to resolve it
		TAU* commonTAU;
		if ( tau[0]->numControllers <= tau[1]->numControllers )
			commonTAU = new TAU(tau[0], tau[1]); // order of TAUs is important here - in case of ambiguity TAU(TAU* tau0, TAU* tau1) will ask tau0 to resolve it
		else
			commonTAU = new TAU(tau[1], tau[0]);

		recentAmbiguities = commonTAU->ambiguities;
		recentConflicts = commonTAU->conflicts;
		if( commonTAU->conflicts == 0 ) {
//		if( commonTAU->conflicts == 0 && commonTAU->ambiguities == 0 ) {
			commonTAU->Optimize();
			delete tau[2];
			tau[2] = commonTAU;
		}
		else {
			printf("TAUS: ditching conflicting common TAU forever\n");
			delete commonTAU;
		}
	}
}

void TAUS::controllersSavePair(int pid, OPTIMIZER* optimizer, ofstream* outFile) {

	NEURAL_NETWORK** savedControllers;
	if( pid > 0 ) { // if we are not at the first iteration, ignore pointers to saved controllers returned by Controllers_Save_Pair()
		int idx = indexByPID(pid);
		int other = idx==0 ? 1 : 0;
		savedControllers = tau[idx]->Controllers_Save_Pair(optimizer, tau[other], outFile);
	}
	else if(pid == 0) { // if we are at the first iteration, use pointers to store the controllers in another tau
		savedControllers = tau[0]->Controllers_Save_Pair(optimizer, outFile);
	}
	else if(pid == -1) {
		savedControllers = tau[1]->Controllers_Save_Pair(optimizer, tau[0], outFile);
	}
	else {
		printf("TAUS::controllersSavePair failed\n");
		exit(1);
	}

	delete [] savedControllers;
}

void TAUS::rescorePopulation(OPTIMIZER* optimizer) { // a monster

	double scores[3][AFPO_POP_SIZE];

	double error[3] = {TAU_OPTIMIZER_ERROR_UNDEFINED, TAU_OPTIMIZER_ERROR_UNDEFINED, TAU_OPTIMIZER_ERROR_UNDEFINED};
	bool ready[3];

	// filling our scores array with proper values (see rawScores)
	for( int i=0; i<3; i++ ) {
		ready[i] = tau[i]->Ready_To_Predict();
		if( ready[i] ) {
			error[i] = tau[i]->Model_Error();
			rawScores(scores[i], i, optimizer);
		}
		else
			for(int j=0; j<AFPO_POP_SIZE; j++)
				scores[i][j] = TAU_NO_SCORE;
	}

	// deciding which score to use and performing the rescoring
	if( ready[0] && ready[1] ) {
		if( ready[2] &&
				error[0] > error[2] &&
				error[1] > error[2] ) {

			typeOfLastScore = 4;
			for( int j=0; j<AFPO_POP_SIZE; j++ )
				optimizer->genomes[j]->Score_Set( scores[2][j] );
			return;
		}
		else {
			typeOfLastScore = 3;
			for( int j=0; j<AFPO_POP_SIZE; j++ )
				optimizer->genomes[j]->Score_Set( fmax( scores[0][j], scores[1][j] ) );
			return;
		}
	}
	else {
		if(ready[0]) {
			typeOfLastScore = 1;
			for( int j=0; j<AFPO_POP_SIZE; j++ )
				optimizer->genomes[j]->Score_Set( scores[0][j] );
			return;
		}
		if(ready[1]) {
			typeOfLastScore = 2;
			printf("TAUS: WARNING - tau[1] is ready before tau[0]\n");
			for( int j=0; j<AFPO_POP_SIZE; j++ )
				optimizer->genomes[j]->Score_Set( scores[1][j] );
			return;
		}
		if(ready[2]) {
			typeOfLastScore = 5;
			printf("TAUS: WARNING - common tau is ready before any other\n");
			for( int j=0; j<AFPO_POP_SIZE; j++ )
				optimizer->genomes[j]->Score_Set( scores[2][j] );
			return;
		}
		typeOfLastScore = -1;
		printf("TAUS: WARNING - decision maker default case reached\n");
		return;
	}
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

void TAUS::rawScores(double* output, int tauidx, OPTIMIZER* optimizer) {

	// TEMPORARY SUBSTITUTE

//	double hiscores[3] = {-1*INFINITY, -1*INFINITY, -1*INFINITY};
//	double loscores[3] = {INFINITY, INFINITY, INFINITY};

	// use straightforward TAU predictions where available, TAU_NO_SCORE where not
	// taking notes on min and max scores

	printf("TAU%d: ", tauidx);
	for( int j=0; j<AFPO_POP_SIZE; j++ ) {
		if( optimizer->genomes[j] && optimizer->genomes[j]->sensorTimeSeries ) {
			printf("%2.2lf ", tau[tauidx]->Score_Predict(optimizer->genomes[j]));
			output[j] = 1.0;

//			hiscores[i] = hiscores[i] < scores[i][j] ? scores[i][j] : hiscores[i];
//			loscores[i] = loscores[i] > scores[i][j] ? scores[i][j] : loscores[i];
		}
		else {
			printf("%2.2lf ", TAU_NO_SCORE);
			output[j] = TAU_NO_SCORE;
			printf("TAUS: WARNING - the number of simulated individuals in AFPO population is less than expected\n");
		}
	}
	printf("\n");

	// normalizing scores
//	for( int i=0; i<3; i++ ) {
//		if( ready[i] ) {
//			for( int j=0; j<AFPO_POP_SIZE; j++ ) {
//				if( scores[i][j] != TAU_NO_SCORE )
//					scores[i][j] = (scores[i][j] - loscores[i])/(hiscores[i] - loscores[i]);
//			}
//		}
//	}
}

#endif
