#include <stdlib.h>

#ifndef _TAUS_CPP
#define _TAUS_CPP

#include "taus.h"

extern double TAU_NO_SCORE;
extern double TAU_OPTIMIZER_ERROR_UNDEFINED;
extern int AFPO_POP_SIZE;

TAUS::TAUS(void) {

	for(int i=0; i<3; i++)
		tau[i] = new TAU(i);
	noOfClients = 0;
	clientList[0] = 0;
	clientList[1] = 0;

	typeOfLastScore = 0;

	recentAmbiguities = 0;
	recentConflicts = 0;
}

TAUS::~TAUS(void) {

	for(int i=0; i<3; i++)
		delete tau[i];
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
			if( !tau[2]->commonTAU ) {
				delete tau[2];
				tau[2] = new TAU(tau[0], tau[1], 2);
			}
			else
				tau[2]->Update_Common_TAU(tau[0], tau[1]);
//		tau[otherIdx]->Scores_Check();
//		TAU* commonTAU = new TAU(tau[idx], tau[otherIdx], 2);
//		commonTAU->Optimize();
//		delete tau[2];
//		tau[2] = commonTAU;
			tau[2]->Optimize();
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
/*
	// trivial decision maker for preliminary testing
	if( ready[2] ) {
		typeOfLastScore = 4;
		for( int j=0; j<AFPO_POP_SIZE; j++ )
			optimizer->genomes[j]->Score_Set( scores[2][j] );
		return;
	}
	else if( ready[0] ) {
		typeOfLastScore = 1;
		for( int j=0; j<AFPO_POP_SIZE; j++ )
			optimizer->genomes[j]->Score_Set( scores[0][j] );
		return;
	}
	else {
		printf("ERROR - Not enough ready TAUs\n");
		exit(1);
	}
*/

	// deciding which score to use and performing the rescoring
	if( ready[0] && ready[1] ) {
		if( ready[2] &&
				error[0] >= error[2] &&
				error[1] >= error[2] ) {

			typeOfLastScore = 4;
			for( int j=0; j<AFPO_POP_SIZE; j++ )
				optimizer->genomes[j]->Score_Set( scores[2][j] );

			printf("TAU2:");
			for( int i=0; i<AFPO_POP_SIZE; i++ )
				printf(" %2.2lf", optimizer->genomes[i]->Score_Get());
			printf("\n");
			return;
		}
		else {
			typeOfLastScore = 3;
			for( int j=0; j<AFPO_POP_SIZE; j++ )
				optimizer->genomes[j]->Score_Set( fmax( scores[0][j], scores[1][j] ) );

			printf("max(TAU0, TAU1):");
			for( int i=0; i<AFPO_POP_SIZE; i++ )
				printf(" %2.2lf", optimizer->genomes[i]->Score_Get());
			printf("\n");
			return;
		}
	}
	else {
		if(ready[0]) {
			typeOfLastScore = 1;
			for( int j=0; j<AFPO_POP_SIZE; j++ )
				optimizer->genomes[j]->Score_Set( scores[0][j] );

			printf("TAU0:");
			for( int i=0; i<AFPO_POP_SIZE; i++ )
				printf(" %2.2lf", optimizer->genomes[i]->Score_Get());
			printf("\n");
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

/* ///// bubble sort for controllers /////
void TAUS::sortControllers(TAU* tau, NEURAL_NETWORK** A, int start, int end)
{
	int size = end-start+1;
	if( size < 2 )
		return;

	NEURAL_NETWORK* buf;

	int i=0;
	while( i<AFPO_POP_SIZE-1 ) {
		while( i >= 0 && tau->Score_Predict(A[i], A[i+1]) > 0.5 ) {
			buf = A[i];
			A[i] = A[i+1];
			A[i+1] = buf;
			i--;
		}
		i++;
	}

	return;
}*/

/* ///// quicksort for controllers /////
void TAUS::sortControllers(TAU* tau, NEURAL_NETWORK** A, int start, int end)
{
	int size = end-start+1;
	if( size < 2 )
		return;

	NEURAL_NETWORK* pivot = A[0];
	NEURAL_NETWORK** left = new NEURAL_NETWORK*[size-1];
	NEURAL_NETWORK** right = new NEURAL_NETWORK*[size-1];

	int l = 0; int r = 0; int i;
	for(i=1; i<size; i++)
	{
		if( tau->Score_Predict(A[i], pivot) <= 0.5 )
		{
			left[l] = A[i];
			l++;
		}
		else
		{
			right[r] = A[i];
			r++;
		}
	}
	int leftSize = l;
	int rightSize = r;

	sortControllers(tau, left, 0, leftSize-1);
	sortControllers(tau, right, 0, rightSize-1);

	i = 0;
	for(l=0; l<leftSize; l++)
	{
		A[i] = left[l];
		i++;
	}
	A[i] = pivot;
	i++;
	for(r=0; r<rightSize; r++)
	{
		A[i] = right[r];
		i++;
	}

	delete [] left;
	delete [] right;
	return;
}*/

void TAUS::rawScores(double* output, int tauidx, OPTIMIZER* optimizer) {

/*	printf("\nStarting raw scores generation\n");
	printf("TAU preferences matrix\n");
	tau[tauidx]->preferences->PrintWithSums(2);

  MATRIX* demo = new MATRIX(tau[tauidx]->numControllers, tau[tauidx]->numControllers, 0.0);
  for( int i=0; i<tau[tauidx]->numControllers; i++ ) {
    for( int j=0; j<tau[tauidx]->numControllers; j++ ) {

      demo->Set(i, j, tau[tauidx]->Score_Predict(optimizer->genomes[i], optimizer->genomes[j]));
    }
  }
  printf("Learned pref. matrix:\n");
  demo->PrintWithSums(2);*/


	MATRIX* approxPref = new MATRIX(AFPO_POP_SIZE, AFPO_POP_SIZE, 0.0);

//	printf("X11%d:", tauidx);
//	for( int i=0; i<AFPO_POP_SIZE; i++ ) {
//		printf(" %2.2lf", genomes1[i]->sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0), 11));
//	}
//	printf("\n");

	double prediction;
	bool nontrivial = false;
	for(int i=0; i<AFPO_POP_SIZE; i++) {
		for(int j=0; j<AFPO_POP_SIZE; j++) {
			prediction = tau[tauidx]->Score_Predict(optimizer->genomes[i], optimizer->genomes[j]);
			approxPref->Set(i, j, prediction);
			if( prediction != 0.0 )
				nontrivial = true;
		}
	}

/*	printf("Got an approximate preferences matrix for AFPO population:\n");
	approxPref->PrintWithSums(2);*/

//	double min = INFINITY;
//	double max = -1*INFINITY;


	for(int i=0; i<AFPO_POP_SIZE; i++) {
		if(nontrivial)
			output[i] = (approxPref->SumOfRow(i) + ((double) AFPO_POP_SIZE))/((double) 2*AFPO_POP_SIZE);
		else
			output[i] = TAU_NO_SCORE;
//		min = min > output[i] ? output[i] : min;
//		max = max < output[i] ? output[i] : max;
	}

// normalization
//	printf("Normalizing scores: min %le, max %le\n", min, max);
//	for(int i=0; i<AFPO_POP_SIZE; i++)
//		output[i] = (output[i] - min)/(max - min);

//	printf("SCR%d:", tauidx);
//	for( int i=0; i<AFPO_POP_SIZE; i++ )
//		printf(" %2.2lf", output[i]);
//	printf("\n");

//	for(int i=0; i<tau[tauidx]->numControllers-1; i++) {
//		printf("%d\t", tau[tauidx]->preferences->Get(i,i+1));
//	}
//	printf("\n");

//	for(int i=0; i<tau[tauidx]->numControllers; i++) {
//		printf("%d\t", out);
//	}
//	printf("\n");

	delete approxPref;
}

#endif
