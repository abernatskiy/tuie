#ifndef _TAU_CPP
#define _TAU_CPP

#include "stdio.h"
#include "stdlib.h"
#include "tau.h"
#include "tauOptimizer.h"
#include "rankings/rankings.h"

extern double TAU_NO_SCORE;
extern double TAU_OPTIMIZER_ERROR_UNDEFINED;
extern int    AFPO_POP_SIZE;
extern int		STARTING_EVALUATION_TIME;

double distance(TAU* tau0, TAU* tau1, int id0, int id1) {

	int idx0 = tau0->Find_Index(id0);
	int idx1 = tau1->Find_Index(id1);

	if( idx0 < 0 || idx1 < 0) {
		printf("IDs not found when determining distance\n");
		exit(1);
	}

	int timeStep = int(double(STARTING_EVALUATION_TIME)/2.0);
	MATRIX* sts0 = tau0->controllers[idx0]->sensorTimeSeries;
	MATRIX* sts1 = tau1->controllers[idx1]->sensorTimeSeries;

	if( sts0 == NULL || sts1 == NULL ) {
		printf("Sensor time series not found when determining distance\n");
		exit(1);
	}

	double dist = 0.0;
//	printf("\n!!!!!Calculating the distance\n");
	for(int k=0; k<=10; k=k+2) {
//		printf("+ ( %le - %le )^2\n", sts0->Get(timeStep, k), sts1->Get(timeStep, k));
		dist += pow(sts0->Get(timeStep, k) - sts1->Get(timeStep, k), 2);
	}
	dist = sqrt(dist);
//	printf("!!!!!Done calculating distance, %le is answer\n\n", dist);
	return dist;
}

TAU::TAU(void) {

	id = -1;

	numControllers = 0;

	firstControllerIndex  = -1;
	secondControllerIndex = -1;

	controllers = NULL;

	preferences = NULL;

	tauOptimizer = NULL;

	nextControllerForTraining = 0;

	scoreMin = +1000000.0;
	scoreMax = -1000000.0;

	commonTAU = false;
	ambiguities = -1;
	conflicts = -1;

	timer = 0;

	requestFromCommonTAU = false;
	currentJobFromCommonTAU = false;
	remoteController = NULL;
}

TAU::TAU(int id_no) {

	id = id_no;

	numControllers = 0;

	firstControllerIndex  = -1;
	secondControllerIndex = -1;

	controllers = NULL;

	preferences = NULL;

	tauOptimizer = NULL;

	nextControllerForTraining = 0;

	scoreMin = +1000000.0;
	scoreMax = -1000000.0;

	commonTAU = false;
	ambiguities = -1;
	conflicts = -1;

	timer = 0;

	requestFromCommonTAU = false;
	currentJobFromCommonTAU = false;
	remoteController = NULL;
}


TAU::TAU(ifstream *inFile) {

	commonTAU = false;

	(*inFile) >> numControllers;

	int controllersSaved;
	(*inFile) >> controllersSaved;

	if ( controllersSaved ) {
		controllers = new NEURAL_NETWORK * [numControllers];
		for (int i=0; i<numControllers; i++)
			controllers[i] = new NEURAL_NETWORK(inFile);
	}
	else
		controllers = NULL;

	int preferencesSaved;
	(*inFile) >> preferencesSaved;

	if ( preferencesSaved )
		preferences = new MATRIX(inFile);
	else
		preferences = NULL;
  (*inFile) >> firstControllerIndex;
  (*inFile) >> secondControllerIndex;

	int tauOptimizerSaved;

	(*inFile) >> tauOptimizerSaved;

	if ( tauOptimizerSaved )
		tauOptimizer = new TAU_OPTIMIZER(inFile);
	else
		tauOptimizer = NULL;

        (*inFile) >> nextControllerForTraining;
        (*inFile) >> scoreMin;
        (*inFile) >> scoreMax;

	commonTAU = false;
	ambiguities = -1;
	conflicts = -1;

	requestFromCommonTAU = false;
	currentJobFromCommonTAU = false;
	remoteController = NULL;
}

TAU::TAU(TAU* tau0, TAU* tau1, int id_no) {

	id = id_no;

	timer = 0; // irrelevant

	firstControllerIndex  = -1; // irrelevant
	secondControllerIndex = -1; // irrelevant
	nextControllerForTraining = 0; // irrelevant

	commonTAU = true;

	conflicts = 0;
	ambiguities = 0;
	tauOptimizer = NULL;

	printf("TAU: constructing common TAU\n");
	commonNumCont[0] = tau0->Controllers_Available_For_Optimization();
	commonNumCont[1] = tau1->Controllers_Available_For_Optimization();
	numControllers = commonNumCont[0] + commonNumCont[1];
	printf("TAU: %d controllers from tau0, %d controllers from tau1\n", commonNumCont[0], commonNumCont[1]);

	commonCont[0] = new NEURAL_NETWORK*[commonNumCont[0]];
	for(int i=0; i<commonNumCont[0]; i++) {
		commonCont[0][i] = new NEURAL_NETWORK(tau0->controllers[i]);
		commonCont[0][i]->sensorTimeSeries = new MATRIX(tau0->controllers[i]->sensorTimeSeries);
	}

	commonCont[1] = new NEURAL_NETWORK*[commonNumCont[1]];
	for(int i=0; i<commonNumCont[1]; i++) {
		commonCont[1][i] = new NEURAL_NETWORK(tau1->controllers[i]);
		commonCont[1][i]->sensorTimeSeries = new MATRIX(tau1->controllers[i]->sensorTimeSeries);
	}

	commonPref[0] = new MATRIX(tau0->preferences);
	commonPref[1] = new MATRIX(tau1->preferences);

	requestFromCommonTAU = false;
	currentJobFromCommonTAU = false;
	remoteController = NULL;
}

TAU::~TAU(void) {

	if(commonTAU) {
		for(int i=0; i<commonNumCont[0]; i++) {
			delete commonCont[0][i];
			commonCont[0][i] = NULL;
		}
		delete [] commonCont[0];
		commonCont[0] = NULL;

		for(int i=0; i<commonNumCont[1]; i++) {
			delete commonCont[1][i];
			commonCont[1][i] = NULL;
		}
		delete [] commonCont[1];
		commonCont[1] = NULL;

		delete commonPref[0];
		commonPref[0] = NULL;
		delete commonPref[1];
		commonPref[1] = NULL;
	}
	else {
		if ( controllers ) {
			for (int i=0;i<numControllers;i++) {
				if ( controllers[i] ) {
					delete controllers[i];
					controllers[i] = NULL;
				}
			}
			delete [] controllers;
			controllers = NULL;
		}

		if ( preferences ) {
			delete preferences;
			preferences = NULL;
		}

		if( requestFromCommonTAU )
			delete remoteController;
	}

	if ( tauOptimizer ) {
		delete tauOptimizer;
		tauOptimizer = NULL;
	}

}

int TAU::All_Required_Preferences_Supplied(void) {

	if ( preferences )
		return( !preferences->ValFoundOffTheDiagonal(0.0) );
	else
		return( false );
}

void TAU::Controller_First_Store_Sensor_Data(MATRIX *sensorData) {

	Controller_Store_Sensor_Data(firstControllerIndex,sensorData);
}

void TAU::Controller_Second_Store_Sensor_Data(MATRIX *sensorData) {

  Controller_Store_Sensor_Data(secondControllerIndex,sensorData);
}

NEURAL_NETWORK *TAU::Controller_Get_Best(void) {

	double bestScore = -1000.0;
	NEURAL_NETWORK *best = NULL;

	for (int i=0; i<numControllers; i++)
		if ( controllers[i]->Score_Get() > bestScore ) {
			bestScore = controllers[i]->Score_Get();
			best = controllers[i];
		}

	return( best );
}

NEURAL_NETWORK *TAU::Controller_Pair_Get_First(void) {

	// Get the first of the two controllers about to be evaluated.
	return( controllers[firstControllerIndex] );
}

NEURAL_NETWORK *TAU::Controller_Pair_Get_Second(void) {

	// Get the second of the two controllers about to be evaluated.
	return( controllers[secondControllerIndex] );
}

void TAU::Controllers_Load_Pair(ifstream *inFile) {

	int numControllers = 2;
	controllers = new NEURAL_NETWORK * [numControllers];

	for (int i=0; i<numControllers; i++) {
		controllers[i] = new NEURAL_NETWORK(inFile);
//		printf("CLIENT: TAU: 0th controller loaded successfully, ID %d\n", controllers[i]->ID);
	}

	firstControllerIndex = 0;
	secondControllerIndex = 1;
}

NEURAL_NETWORK** TAU::Controllers_Save_Pair(OPTIMIZER *optimizer, ofstream *outFile) {

	Controllers_Select_From_Optimizer(optimizer);
	controllers[firstControllerIndex]->Save_ButNotSensorData(outFile);
	controllers[secondControllerIndex]->Save_ButNotSensorData(outFile);

	NEURAL_NETWORK** savedControllers = new NEURAL_NETWORK*[2];
	savedControllers[0] = controllers[firstControllerIndex];
	savedControllers[1] = controllers[secondControllerIndex];

//	printf("TAU: saving controllers %d and %d\n", controllers[firstControllerIndex]->ID, controllers[secondControllerIndex]->ID);
	return savedControllers;
}

NEURAL_NETWORK** TAU::Controllers_Save_Pair(OPTIMIZER *optimizer, TAU* otherTAU, ofstream *outFile) {

	Controllers_Select_From_Optimizer(optimizer, otherTAU);
	controllers[firstControllerIndex]->Save_ButNotSensorData(outFile);
	controllers[secondControllerIndex]->Save_ButNotSensorData(outFile);

	NEURAL_NETWORK** savedControllers = new NEURAL_NETWORK*[2];
	savedControllers[0] = controllers[firstControllerIndex];
	savedControllers[1] = controllers[secondControllerIndex];
	return savedControllers;
}

void TAU::Controllers_Select_From_Optimizer(OPTIMIZER *optimizer) {

	int controllersNeededFromOptimizer = Controllers_Num_Needed_From_Optimizer();

	switch ( controllersNeededFromOptimizer ) {

	case 0:
		Controllers_Select_Two_From_TAU();
		break;

	case 1:
		Controllers_Select_One_From_TAU_One_From_Optimizer(optimizer);
		break;

	case 2:
		Controllers_Select_Two_From_Optimizer(optimizer);
		break;
	}
}

void TAU::Controllers_Select_From_Optimizer(OPTIMIZER *optimizer, TAU* otherTAU) {

	int controllersNeededFromOptimizer = Controllers_Num_Needed_From_Optimizer();

	switch ( controllersNeededFromOptimizer ) {

	case 0:
		Controllers_Select_Two_From_TAU();
		break;

	case 1:
		Controllers_Select_One_From_TAU_One_From_Optimizer(optimizer, otherTAU);
		break;

	case 2:
		Controllers_Select_Two_From_Optimizer(optimizer, otherTAU);
		break;
	}
}

void TAU::Optimize(void) {

	// If the user has not yet supplied any preferences,
	// there is nothing to optimize.

	if ( !Ready_For_Optimization() )
		return;

	if ( !tauOptimizer )
		tauOptimizer = new TAU_OPTIMIZER();

//	printf("TAU::Optimize: optimizing for %d controllers, numControllers %d\n", Controllers_Available_For_Optimization(), numControllers);
	if(commonTAU)
		tauOptimizer->Optimize_Common( commonNumCont[0], commonPref[0], commonCont[0], commonNumCont[1], commonPref[1], commonCont[1] );
	else
		tauOptimizer->Optimize( Controllers_Available_For_Optimization(), preferences, controllers );
}

void TAU::Print(void) {

	printf("Number of controllers stored in TAU: %d.\n",numControllers);
	if ( numControllers > 0 ) {
		printf("Number of preferences supplied: %d.\n",Num_Prefs());
		printf("Average preferences per controller: %3.3f.\n", double(Num_Prefs())/double(numControllers) );
	}
}

int TAU::Ready_To_Predict(void) {

	if ( !tauOptimizer )
		return( false );
	return( tauOptimizer->Ready_To_Predict() );
}

double TAU::Model_Error(void) {

	if ( !tauOptimizer )
		return TAU_OPTIMIZER_ERROR_UNDEFINED;
	return tauOptimizer->Model_Error();
}

void TAU::Save(ofstream *outFile) {

	(*outFile) << numControllers << "\n";

	if ( !controllers )
		(*outFile) << "0\n";
	else {
		(*outFile) << "1\n";
		for (int i=0;	i<numControllers; i++)

			controllers[i]->Save(outFile);
	}

	if ( !preferences )
		(*outFile) << "0\n";
	else {
		(*outFile) << "1\n";
		preferences->Write(outFile);
	}

	(*outFile) << firstControllerIndex << "\n";
        (*outFile) << secondControllerIndex << "\n";

	if ( !tauOptimizer )
		(*outFile) << "0\n";
	else {
		(*outFile) << "1\n";
		tauOptimizer->Save(outFile);
	}

	(*outFile) << nextControllerForTraining << "\n";
	(*outFile) << scoreMin << "\n";
	(*outFile) << scoreMax << "\n";
}

double TAU::Score_Predict(NEURAL_NETWORK *controller1, NEURAL_NETWORK *controller2) {

	if ( !tauOptimizer )
		return( TAU_NO_SCORE );
	return( tauOptimizer->Score_Predict(controller1, controller2) );
}

void TAU::Store_Pref(int firstID, int secondID, int pref) {

//	printf("TAU::Store_Pref: receiving preference %d %d %d\n", firstID, secondID, pref);

	firstControllerIndex = Find_Index(firstID);
	secondControllerIndex = Find_Index(secondID);

	// print out the entire situation for debug
/*	printf("TAU::Store_Pref: storing preference %d(%d) %d(%d) %d into the following preference matrix:\n", firstControllerIndex, firstID,
		secondControllerIndex, secondID, pref);
	preferences->Print(2);
	printf("TAU::Store_Pref: listing of all familiar controllers:\n");
	for( int j=0; j<numControllers; j++ )
		printf("%d\t%d\t%le\n", j, controllers[j]->ID, controllers[j]->Score_Get());
*/

	// compressing 3 arguments into two to simplify the code
	int winnerIndex = (pref == 0) ? firstControllerIndex : secondControllerIndex;
	int loserIndex = (pref != 0) ? firstControllerIndex : secondControllerIndex;

	// obtaining some scores in advance to optimize things a bit
	double winnerScore = controllers[winnerIndex]->Score_Get();
	double loserScore = controllers[loserIndex]->Score_Get();
	double currentScore;

	// table filling itself
	preferences->SetAntiSymm(winnerIndex, loserIndex, 1.0);
	for( int j=0; j<numControllers; j++ ) {
		currentScore = controllers[j]->Score_Get();
		if( currentScore == TAU_NO_SCORE )
			continue;

		if( loserScore != TAU_NO_SCORE &&
				currentScore <= loserScore ) {
			if( preferences->Get(winnerIndex, j) == 0.0 )
				preferences->SetAntiSymm(winnerIndex, j, 1.0);
			else if( preferences->Get(winnerIndex, j) == -1.0 )
				printf("TAU::Store_Pref: WARNING! Preferences may be insane\n");
		}

		if( winnerScore != TAU_NO_SCORE &&
				currentScore >= winnerScore ) {
			if( preferences->Get(loserIndex, j) == 0.0 )
				preferences->SetAntiSymm(loserIndex, j, -1.0);
			else if( preferences->Get(loserIndex, j) == 1.0 )
				printf("TAU::Store_Pref: WARNING! Preferences may be insane\n");
		}
	}

/*	printf("TAU::Store_Pref: Resulting matrix:\n");
	preferences->Print(2);
	printf("\n");*/

	if( !preferences->ValFoundOffTheDiagonal(0.0) ) { // if we are done bisecting
		printf("TAU::Store_Pref: done bisecting, updating scores\n");
		Scores_Update(); // update the scores

		if(currentJobFromCommonTAU) {// if the job was done for common TAU
			printf("TAU::Store_Pref: disabling requestFromCommonTAU\n");
			currentJobFromCommonTAU = false; // disable the corresponding mutex
			requestFromCommonTAU = false;
		}
	}

//	printf("TAU::Store_Pref: result is a TAU with %d controllers, IDs: ", numControllers);
//	for(int i=0; i<numControllers; i++)
//		printf("%d(%2.2f) ", controllers[i]->ID, controllers[i]->Score_Get());
//	printf("\n");
}

/*
void TAU::User_Models_Reset(vowid) {

	if ( tauOptimizer )
		tauOptimizer->User_Models_Reset(Controllers_Available_For_Optimization() ,controllers);
}*/

void TAU::Controller_Store(NEURAL_NETWORK *newController) {

	for( int i=0; i<numControllers; i++ )
		if( newController->ID == controllers[i]->ID ) {
			printf("TAU: warning - attempt to store the controller already in TAU detected (ID %d)\n", newController->ID);
			return;
		}

	Controller_Store_Without_ID_Check(newController);
}

void TAU::Controller_Store_Without_ID_Check(NEURAL_NETWORK *newController) {

	if ( !controllers )
		Storage_Initialize();
  else
		Storage_Expand();

	controllers[numControllers] = new NEURAL_NETWORK(newController);
	controllers[numControllers]->fitness = newController->fitness;
	controllers[numControllers]->sensorTimeSeries = new MATRIX(newController->sensorTimeSeries);
	controllers[numControllers]->score = TAU_NO_SCORE; // !!!!
  numControllers++;
}

// --------------------------- Private methods -----------------------------

void TAU::Controller_Store_Sensor_Data(int controllerIndex, MATRIX *sensorData) {

	if ( controllers[controllerIndex] )
		controllers[controllerIndex]->Store_Sensor_Data(sensorData);
}

int TAU::Controllers_Available_For_Optimization(void) {
/*
	int numAvailable = 0;

	for (int i=0;	i<numControllers;	i++) {
		if (	 controllers[i] &&
			 controllers[i]->Score_Available() &&
			(controllers[i]->Get_Sensor_Data() != NULL) &&
			(controllers[i]->Score_Get() != TAU_NO_SCORE) &&
			preferences->SumOfRow(i) != 0)

			numAvailable++;
	}

	return( numAvailable );
*/

	if(preferences->ValFoundOffTheDiagonal(0.0))
		return numControllers-1;
	else
		return numControllers;
}

void TAU::Controllers_Expand(void) {

	NEURAL_NETWORK **temp = new NEURAL_NETWORK * [numControllers+1];

	for (int i=0;i<numControllers;i++) {
		temp[i] = controllers[i];
		controllers[i] = NULL;
	}

	temp[numControllers] = NULL;

	delete [] controllers;
	controllers = temp;

	temp = NULL;
}

void TAU::Controllers_Initialize(void) {

	controllers = new NEURAL_NETWORK * [1];
	controllers[0] = NULL;
}

int TAU::Controllers_Num_Needed_From_Optimizer(void) {

	// If no controllers have yet been stored in TAU,
	// request two controllers from the optimizer for comparison.

	if ( numControllers == 0 )
		return( 2 );

	// If there is a zero entry at i,j in the preferences matrix,
	// that indicates that controllers i and j have not yet been
	// compared, so we don't need any new controllers from the optimizer.

	if ( preferences->ValFoundOffTheDiagonal(0.0) )
		return(0);

	// If there are no zero entries off the diagonal, that means
	// that every pair of controllers in TAU has been compared.
	// So, we need a new one from the optimizer to compare against
	// those stored in TAU.

	else
		return(1);
}

void TAU::Controllers_Print(void) {

	for (int i=0;i<numControllers;i++)
		controllers[i]->Print();
}

void TAU::Controllers_Select_One_From_TAU_One_From_Optimizer(OPTIMIZER *optimizer) {

	// Choose best controller from TAU
	double bestScore = -1000.0;
	for (int i=0;i<numControllers;i++) {
		if ( controllers[i]->Score_Get() > bestScore ) {
			bestScore = controllers[i]->Score_Get();
			firstControllerIndex = i;
		}
	}

	printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: func called, first controller %d, idx %d\n", controllers[firstControllerIndex]->ID, firstControllerIndex);

	if( requestFromCommonTAU ) { // will be turned off when job is done
		// If common TAU has requested ambiguity resolution compare it against the ambiguous controller
		printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: common TAU request found\n");
		Controller_Store( remoteController );
		currentJobFromCommonTAU = true;
	}

	else {
		// Otherwise, get a new controller from the optimizer.
		if ( numControllers > 0 ) {

			NEURAL_NETWORK *secondController = NULL;
			if ( !tauOptimizer ) {
//				printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: no tauOptimizer, getting random genome\n");
//				secondController = optimizer->Genome_Get_Random();

//				printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: no tauOptimizer, getting most different genome\n");
//				secondController = optimizer->Genome_Get_Most_Different(numControllers, controllers);

				printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: no tauOptimizer, getting best genome");
				secondController = optimizer->Genome_Get_Best();
			}
			else {
//				printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: normal structure of self, getting random genome but not\n");
//				secondController = optimizer->Genome_Get_Random_But_Not(numControllers,controllers);

//				printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: normal structure of self, getting most different genome but not\n");
//				secondController = optimizer->Genome_Get_Most_Different_But_Not(numControllers, controllers, numControllers, controllers);

				printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: normal structure of self, getting best genome but not\n");
				secondController = optimizer->Genome_Get_Best_But_Not(numControllers, controllers);
			}

			Controller_Store( secondController );
		}
		else {
//			printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: numControllers<=0, getting random genome\n");
//			Controller_Store( optimizer->Genome_Get_Random() );

			printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: numControllers<=0, getting best genome\n");
			Controller_Store( optimizer->Genome_Get_Best() );
		}
	}
	secondControllerIndex = numControllers-1;

	printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: second controller %d, idx %d\n", controllers[secondControllerIndex]->ID, secondControllerIndex);
}

void TAU::Controllers_Select_One_From_TAU_One_From_Optimizer(OPTIMIZER *optimizer, TAU* otherTAU) {

	// Choose best controller from TAU
	double bestScore = -1000.0;
	for (int i=0;i<numControllers;i++) {
		if ( controllers[i]->Score_Get() > bestScore ) {
			bestScore = controllers[i]->Score_Get();
			firstControllerIndex = i;
		}
	}

	printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: func called with TAU, first controller %d, idx %d\n", controllers[firstControllerIndex]->ID, firstControllerIndex);

	if( requestFromCommonTAU ) { // will be turned off when job is done
		// If common TAU has requested ambiguity resolution compare it against the ambiguous controller
		printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: common TAU request found\n");
		Controller_Store( remoteController );
		currentJobFromCommonTAU = true;
	}
	else {
		// Otherwise, get a new controller from the optimizer.
		if ( numControllers > 0 ) {

			NEURAL_NETWORK *secondController = NULL;
			if ( !tauOptimizer ) {
				printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: no tauOptimizer, getting best genome");
				secondController = optimizer->Genome_Get_Best();
			}
			else {

				int tot = numControllers + otherTAU->numControllers;
				NEURAL_NETWORK* notSecond[tot];
				for(int i=0; i<numControllers; i++)
					notSecond[i] = controllers[i];
				for(int i=0; i<otherTAU->numControllers; i++)
					notSecond[numControllers+i] = otherTAU->controllers[i];

				printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: normal structure of self, getting best genome but not\n");
				secondController = optimizer->Genome_Get_Best_But_Not(tot, notSecond);

//				printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: normal structure of self, getting most different genome but not\n");
//				secondController = optimizer->Genome_Get_Most_Different_But_Not(numControllers, controllers, otherTAU->numControllers, otherTAU->controllers);

//				printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: normal structure of self, getting random genome but not\n");
//				secondController = optimizer->Genome_Get_Random_But_Not(numControllers,controllers);

//				printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: normal structure of self, getting best genome but not\n");
//				secondController = optimizer->Genome_Get_Best_But_Not(numControllers,controllers);
			}

			Controller_Store( secondController );
		}
		else {
//			printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: numControllers<=0, getting random genome\n");
//			Controller_Store( optimizer->Genome_Get_Random() );

			printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: numControllers<=0, getting best genome\n");
			Controller_Store( optimizer->Genome_Get_Best() );
		}
	}
	secondControllerIndex = numControllers-1;

	printf("TAU::Controllers_Select_One_From_TAU_One_From_Optimizer: second controller %d, idx %d\n", controllers[secondControllerIndex]->ID, secondControllerIndex);
}

void TAU::Controllers_Select_Two_From_Optimizer(OPTIMIZER *optimizer) {

	// Choose two random yet distinct controllers from the optimizer.

	NEURAL_NETWORK *firstController = optimizer->Genome_Get_First();
//	NEURAL_NETWORK *firstController = optimizer->Genome_Get_Random();

	Controller_Store(firstController);

	firstControllerIndex = numControllers-1;

	NEURAL_NETWORK *secondController = optimizer->Genome_Get_Second();
//	NEURAL_NETWORK *secondController = optimizer->Genome_Get_Random_But_Not(firstController);

	Controller_Store( secondController );

	secondControllerIndex = numControllers-1;

	firstController = NULL;
	secondController = NULL;
}

void TAU::Controllers_Select_Two_From_Optimizer(OPTIMIZER *optimizer, TAU* otherTAU) {

/*	NEURAL_NETWORK* firstController = otherTAU->controllers[0];
	Controller_Store(firstController);
	firstControllerIndex = numControllers-1;

	NEURAL_NETWORK* notASecondController[2];
	notASecondController[0] = otherTAU->controllers[0];
	notASecondController[1] = otherTAU->controllers[1];

	NEURAL_NETWORK *secondController = optimizer->Genome_Get_Most_Different_But_Not(2, notASecondController, 2, notASecondController);
	Controller_Store( secondController );
	secondControllerIndex = numControllers-1;*/

	NEURAL_NETWORK* firstController = optimizer->Genome_Get_Best_But_Not(otherTAU->numControllers, otherTAU->controllers);
	Controller_Store(firstController);
	firstControllerIndex = numControllers-1;

	NEURAL_NETWORK* notASecondController[1+otherTAU->numControllers];
	notASecondController[0] = firstController;
	for(int i=0; i<otherTAU->numControllers; i++)
		notASecondController[i+1] = otherTAU->controllers[i];

	NEURAL_NETWORK *secondController = optimizer->Genome_Get_Best_But_Not(1+otherTAU->numControllers, notASecondController);
	Controller_Store( secondController );
	secondControllerIndex = numControllers-1;

	firstController = NULL;
	secondController = NULL;
}

void TAU::Controllers_Select_Two_From_TAU(void) {

	// Choose the most recently-added controller.
	secondControllerIndex = numControllers-1;

/*	printf("\nTwo controllers from TAU requested\n");
	printf("Second one is the recently added %d with %le score\n", controllers[secondControllerIndex]->ID, controllers[secondControllerIndex]->Score_Get());
	printf("Hunting the first. Conditions:\n");
	printf("\nidx\tscr\t\tprf\t\tid\n");*/

	/**** Choose most information-rich comparison in dichotomic manner ****/
	double left_ev		= INFINITY;				// min score among uncompared controllers
	double right_ev		= (-1)*INFINITY;	// max score among uncompared controllers
	double left_unev	= INFINITY;				// min score among compared controllers
	double right_unev	= (-1)*INFINITY;	// max score among compared controllers
	int left_unev_idx, right_unev_idx;
	int num_unev			= 0;							// no of uncompared controllers
	double avg_unev		= 0;							// average score of nonevaluated controllers
	double curscore;

	// make a pass through the corresponding row in preferences matrix and calculate all values above
	for( int i=0; i<numControllers-1; i++ ) {
		curscore = controllers[i]->Score_Get();
//		printf("%d\t%le\t%lf\t%d\n", i, curscore, preferences->Get(secondControllerIndex, i), controllers[i]->ID);
		if( preferences->Get(secondControllerIndex, i) == 0 ) {
			if( curscore < left_unev ) {
				left_unev = curscore;
				left_unev_idx = i;
			}
			if( curscore > right_unev ) {
				right_unev = curscore;
				right_unev_idx = i;
			}
			avg_unev += curscore;
			num_unev++;
		}
		else {
			if( curscore < left_ev ) left_ev = curscore;
			if( curscore > right_ev ) right_ev = curscore;
		}
	}
	avg_unev /= ((double) num_unev);

//	printf("Found bounds: [(%le, %d) .. (%le, %d)] unevaluated (avg %le), [%le .. %le] evaluated.\n", left_unev, left_unev_idx, right_unev, right_unev_idx, avg_unev, left_ev, right_ev);

	// find a sensible comparison
	// if the newbie wasn't compared agains the best or the worst controller of TAU, do that immediately
	if( right_unev > right_ev )
		firstControllerIndex = right_unev_idx;
	else if( left_unev < left_ev )
		firstControllerIndex = left_unev_idx;
	else {	// if the newbie is somewhere in between, hunt it dichotomically
		double min_dist_to_avg = INFINITY;
		for( int i=0; i<numControllers-1; i++ ) {
			if( preferences->Get(secondControllerIndex, i) == 0 &&
					fabs( controllers[i]->Score_Get() - avg_unev ) <= min_dist_to_avg ) {
				min_dist_to_avg = fabs( controllers[i]->Score_Get() - avg_unev );
				firstControllerIndex = i;
			}
		}
	}

//	printf("Decision: %d with id %d, score %le\n\n", firstControllerIndex, controllers[firstControllerIndex]->ID, controllers[firstControllerIndex]->Score_Get());
}

int TAU::Find_Index(int ID) {

	for (int i=0; i<numControllers; i++)
		if ( controllers[i]->ID == ID )
			return( i );
//	printf("TAU: cannot find index.\n");
//	exit(1);
	return -10000;
}

int TAU::Num_Prefs(void) {

	if ( !preferences )
		return( 0 );

	int numZeros = preferences->NumOccurancesOf(0.0);
	int numNonZeroValues = numControllers*numControllers - numZeros;
	int numPrefs = int( double(numNonZeroValues) / 2.0 );

	return( numPrefs );
}

void TAU::Preferences_Expand(void) {

	preferences->AddColumn(0);
	preferences->AddRow(0);
}

void TAU::Preferences_Initialize(void) {

	preferences = new MATRIX(1,1,0);
}

void TAU::Preferences_Print(void) {

	preferences->Print();
}

int  TAU::Ready_For_Optimization(void) {

	// optimization can occur if TAU is a common TAU, obtained by merging
	if ( commonTAU )
		return true;

	// If the user has not indicated any preferences yet,
	// there is nothing to optimize.

	if ( !preferences )
		return( false );

	// Optimization can occur if there are at least
	// two controllers that have a non-zero score
	// assigned to them in the preferences matrix.

	return( preferences->ValuesDifferentFrom(0) >= 2 );
}

double TAU::Scale(double value, double min1, double max1,
                                           double min2, double max2) {

        if ( min1 < 0 )
                value = value - min1;
        else
                value = value + min1;
        return( (value*(max2-min2)/(max1-min1)) + min2 );
}

void TAU::Scores_Print(void) {

	for (int i=0; i<numControllers; i++)
		printf("%2.2f ",controllers[i]->Score_Get() );
//		printf("%0.0f ",controllers[i]->Score_Get() );
	printf("\n");
}

void TAU::Scores_Update(void) {

	// goes through the list of controllers and writes sums of rows of preferences matrix to their "score" field
	// meanwhile, finds the maximum and minimum of these sums
	for (int i=0; i<numControllers; i++) {
		if ( preferences && controllers[i] ) {
			double score = preferences->SumOfRow(i);
			controllers[i]->Score_Set(score);
			if ( score < scoreMin )
				scoreMin = score;
			if ( score > scoreMax )
				scoreMax = score;
		}
	}

	// rewrites "score" fields with scaled sums
	for (int i=0; i<numControllers; i++) {
		double score = controllers[i]->Score_Get();
		score = Scale(score,scoreMin,scoreMax,0,1);
		controllers[i]->Score_Set(score);
	}

	Optimize();
}

void TAU::Scores_Check(void) {

	double known_scores[numControllers];
	for ( int i=0; i<numControllers; i++ ) {
		for ( int j=0; j<i; j++ ) {
			if ( known_scores[j] == controllers[i]->Score_Get() ) {
				preferences->Print();
				Scores_Print();
				exit(1);
			}
		}
		known_scores[i] = controllers[i]->Score_Get();
	}
}

void TAU::Storage_Expand(void) {

	Controllers_Expand();
	Preferences_Expand();
}

void TAU::Storage_Initialize(void) {

	Controllers_Initialize();
	Preferences_Initialize();
}

#endif
