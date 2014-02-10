#ifndef _TAU_OPTIMIZER_CPP
#define _TAU_OPTIMIZER_CPP

#include "math.h"
#include "stdlib.h"
#include "tauOptimizer.h"

extern double TAU_NO_SCORE;
extern double TAU_OPTIMIZER_ERROR_UNDEFINED;
extern double TAU_OPTIMIZER_ERROR_ACCEPTABLE;

TAU_OPTIMIZER::TAU_OPTIMIZER(void) {

	model = NULL;
	modelError = TAU_OPTIMIZER_ERROR_UNDEFINED;
}

TAU_OPTIMIZER::TAU_OPTIMIZER(ifstream *inFile) {

	int modelSaved;
	(*inFile) >> modelSaved;

	if ( modelSaved )
		model = new USER_MODEL(inFile);
	else
		model = NULL;

   (*inFile) >> modelError;
}

TAU_OPTIMIZER::~TAU_OPTIMIZER(void) {

	if ( model ) {
		delete model;
		model = NULL;
	}
}

void TAU_OPTIMIZER::Optimize(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers) {

	// Train the current user model on the supplied controllers.
	if ( !model )
		model = new USER_MODEL(controllers[0]->numSensors);

	modelError = model->Evaluate(numControllers, preferences, controllers);
}

/*void TAU_OPTIMIZER::Print_Predictions(	USER_MODEL *model,
					int numControllers,
					NEURAL_NETWORK **controllers) {

	for (	int i=0;	i<numControllers;	i++) {
		double actualScore = controllers[i]->Score_Get();
		double predictedScore = model->Predict(controllers[i]->sensorTimeSeries);

		printf("[%3.3f %3.3f %3.3f]\n",
			actualScore,
			predictedScore,
			fabs(actualScore-predictedScore));
	}
}*/

int TAU_OPTIMIZER::Ready_To_Predict(void) {

//	if( model != NULL && modelError < TAU_OPTIMIZER_ERROR_ACCEPTABLE );
	if( model != NULL )
		return true;
	return false;
}

double TAU_OPTIMIZER::Model_Error(void) {

	if( model != NULL)
		return modelError;
	return TAU_OPTIMIZER_ERROR_UNDEFINED;
}

void TAU_OPTIMIZER::Save(ofstream *outFile) {

	if ( model ) {
		(*outFile) << "1\n";
		model->Save(outFile);
	}
	else
		(*outFile) << "0\n";

	(*outFile) << modelError << "\n";
}

double TAU_OPTIMIZER::Score_Predict(NEURAL_NETWORK *controller1, NEURAL_NETWORK *controller2) {

	if ( !model )
		return( TAU_NO_SCORE );
	return( model->Predict(controller1, controller2) );
}

/*void TAU_OPTIMIZER::User_Models_Reset(int numControllers, NEURAL_NETWORK **controllers) {

	Print_Predictions(model,numControllers,controllers);

	printf("%5.5f\n",modelError);

	modelError = model->Evaluate(numControllers,controllers);
}*/

#endif
