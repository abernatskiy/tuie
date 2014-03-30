#ifndef _TAU_OPTIMIZER_CPP
#define _TAU_OPTIMIZER_CPP

#include "math.h"
#include "stdlib.h"
#include "tauOptimizer.h"

extern double TAU_NO_SCORE;
extern double TAU_OPTIMIZER_ERROR_UNDEFINED;
extern double TAU_OPTIMIZER_ERROR_ACCEPTABLE;

TAU_OPTIMIZER::TAU_OPTIMIZER(int id_no) {

	id = id_no;
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
		model = new USER_MODEL(id, controllers[0]->numSensors);

	modelError = model->Evaluate(numControllers, preferences, controllers);
}

void TAU_OPTIMIZER::Optimize_Common(int numControllers0, MATRIX *preferences0, NEURAL_NETWORK **controllers0, int numControllers1, MATRIX *preferences1, NEURAL_NETWORK **controllers1) {

	if ( !model )
		model = new USER_MODEL(id, controllers0[0]->numSensors);

	modelError = model->Evaluate_Common(numControllers0, preferences0, controllers0, numControllers1, preferences1, controllers1);
}

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

#endif
