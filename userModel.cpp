#ifndef _USER_MODEL_CPP
#define _USER_MODEL_CPP

#include "stdlib.h"
#include "stdio.h"
#include "math.h"

#include "userModel.h"

extern double TAU_NO_SCORE;
extern int    TAU_NUM_NEURONS;
extern int    STARTING_EVALUATION_TIME;
extern int    ALG_VARIANT_PREFS_ONLY;
extern int    TAU_NUM_SENSOR_ROWS;
extern int    TAU_BACK_PROP_ITER_MAX_MULTIPLIER;

int TAU_INPUTS = 12;

double MASK_THRESHOLD = 0.5;

USER_MODEL::USER_MODEL(int numS) {

	numSensors = numS;
	Allocate_ANN();
}

USER_MODEL::USER_MODEL(ifstream *inFile) {

  (*inFile) >> numSensors;

	int isANN;

	(*inFile) >> isANN;

	if ( isANN )
		ANN = new CBackProp(inFile);
	else
		ANN = NULL;
}

USER_MODEL::~USER_MODEL(void) {

	if ( ANN ) {

		delete ANN;
		ANN = NULL;
	}
}

void USER_MODEL::Allocate_ANN(void) {

	int *layerSizes = new int[3];

	layerSizes[0] = TAU_INPUTS;
	layerSizes[1] = 12;
	layerSizes[2] = 1;

	ANN = new CBackProp(3,layerSizes,0.1,0.1);

	delete [] layerSizes;
	layerSizes = NULL;
}

double USER_MODEL::Evaluate(int numControllers, MATRIX* preferences, NEURAL_NETWORK **controllers) {

	int errors;

	delete ANN;
	Allocate_ANN();

	int maxIterations = TAU_BACK_PROP_ITER_MAX_MULTIPLIER*numControllers*(numControllers-1)/2;

	MATRIX* mask = new MATRIX(numControllers, numControllers, 0.0);
	mask->Randomize(0.0,1.0);

	printf("Training user model on the following matrix:\n");
	preferences->PrintWithSums(2);
	printf("Mask:\n");
	mask->Print(2);

	int reqiter;
	for (int iter=0; iter<maxIterations; iter++) {

		Learn_On_Matrix_With_Mask(numControllers, preferences, controllers, mask);

		errors = 0;
		errors += Errors_On_Matrix_With_Mask(numControllers, preferences, controllers, mask);

		if (errors == 0) {
			reqiter = iter;
			break;
		}
	}

	MATRIX* demo = new MATRIX(numControllers, numControllers, 0.0);
	for( int i=0; i<numControllers; i++ )
		for( int j=0; j<numControllers; j++ )
			demo->Set(i, j, Predict(controllers[i], controllers[j]));

	printf("We learned:\n");
	demo->PrintWithSums(2);
	delete demo;

	return ((double) Errors_On_Matrix_With_Inverted_Mask(numControllers, preferences, controllers, mask))/((double) (numControllers*numControllers));
}

double USER_MODEL::Evaluate_Common(int numControllers0, MATRIX *preferences0, NEURAL_NETWORK **controllers0, int numControllers1, MATRIX *preferences1, NEURAL_NETWORK **controllers1) {

	int errors;

	delete ANN;
	Allocate_ANN();

	int maxIterations = TAU_BACK_PROP_ITER_MAX_MULTIPLIER*(numControllers0*(numControllers0-1) + numControllers1*(numControllers1-1))/2;

	printf("Training user model on the following:\n");
	printf("Matrix0:\n");
	preferences0->PrintWithSums(2);
	printf("Matrix1:\n");
	preferences1->PrintWithSums(2);

	MATRIX* mask0 = new MATRIX(numControllers0, numControllers0, 0.0);
	mask0->Randomize(0.0,1.0);

	MATRIX* mask1 = new MATRIX(numControllers1, numControllers1, 0.0);
	mask1->Randomize(0.0,1.0);

	int reqiter;
	for (int iter=0; iter<maxIterations; iter++) {

		Learn_On_Matrix_With_Mask(numControllers0, preferences0, controllers0, mask0);
		Learn_On_Matrix_With_Mask(numControllers1, preferences1, controllers1, mask1);

		errors = 0;
		errors += Errors_On_Matrix_With_Mask(numControllers0, preferences0, controllers0, mask0);
		errors += Errors_On_Matrix_With_Mask(numControllers1, preferences1, controllers1, mask1);

		if (errors == 0) {
			reqiter = iter;
			break;
		}
	}

	MATRIX* demo0 = new MATRIX(numControllers0, numControllers0, 0.0);
	for( int i=0; i<numControllers0; i++ )
		for( int j=0; j<numControllers0; j++ )
			demo0->Set(i, j, Predict(controllers0[i], controllers0[j]));

	MATRIX* demo1 = new MATRIX(numControllers1, numControllers1, 0.0);
	for( int i=0; i<numControllers1; i++ )
		for( int j=0; j<numControllers1; j++ )
			demo1->Set(i, j, Predict(controllers1[i], controllers1[j]));

	printf("Zeroth learned matrix:\n");
	demo0->PrintWithSums(2);
	printf("First learned matrix:\n");
	demo1->PrintWithSums(2);

	delete demo0;
	delete demo1;

	return ((double) (Errors_On_Matrix_With_Inverted_Mask(numControllers0, preferences0, controllers0, mask0) +
										Errors_On_Matrix_With_Inverted_Mask(numControllers1, preferences1, controllers1, mask1)))/
										((double) (numControllers0*numControllers0 + numControllers1*numControllers1));
}

double USER_MODEL::Predict(NEURAL_NETWORK *controller1, NEURAL_NETWORK *controller2) {

	double* in = new double[TAU_INPUTS];
	Extract_Feature_Vector(in, controller1, controller2);
  ANN->ffwd(in);
	delete [] in;

	return( ANN->Out(0) );
//	return( 2.0*(ANN->Out(0) - 1.0) );
}

void USER_MODEL::Save(ofstream *outFile) {

	(*outFile) << numSensors << "\n";

	if ( ANN ) {
		(*outFile) << "1\n";
		ANN->Save(outFile);
	}
	else
		(*outFile) << "0\n";
}

/***** PRIVATE METHODS *****/

void USER_MODEL::Extract_Feature_Vector(double* in, NEURAL_NETWORK* controller1, NEURAL_NETWORK* controller2) {

	int m=0;

	for( int k=0; k<=10; k=k+2 ) {
		in[m] =	controller1->sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0),k);
		m++;
  }

	for( int k=0; k<=10; k=k+2 ) {
		in[m] = controller2->sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0),k);
		m++;
	}

//	if( controller1->ID > controller2->ID )
//		in[m] = 1.0;
//	else if( controller1->ID < controller2->ID )
//		in[m] = -1.0;
//	else
//		in[m] = 0.0;

//			in[0] = sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0), 11);
//			in[1] = sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0), 12);
//			in[2] = 0;
//			in[3] = 0;
//			in[4] = 0;
//			in[5] = 0;
//			in[6] = 1.0;
}

void USER_MODEL::Learn_On_Matrix(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers) {

	double* in = new double[TAU_INPUTS];
	double* target = new double[1];

//		printf("here\n\n\n");
	for (int i=0;	i<numControllers;	i++) {
		for (int j=0; j<numControllers; j++) {

			Extract_Feature_Vector(in, controllers[i], controllers[j]);
			target[0] = Target(i, j, preferences);
			ANN->bpgt(in, target);
		}
	}
	delete [] in;
	delete [] target;
}

int USER_MODEL::Errors_On_Matrix(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers) {

	double* in = new double[TAU_INPUTS];
	double target, scorePrediction;
	int errors = 0;

	for (int i=0;	i<numControllers;	i++) {
		for (int j=0; j<numControllers; j++) {

			scorePrediction = Predict(controllers[i], controllers[j]);
			target = Target(i, j, preferences);

			if( (scorePrediction < 0.5 && target > 0.5) ||
					(scorePrediction > 0.5 && target < 0.5) )
				errors++;
		}
	}

	delete [] in;
	return errors;
}

void USER_MODEL::Learn_On_Matrix_With_Mask(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers, MATRIX *mask) {

	double* in = new double[TAU_INPUTS];
	double* target = new double[1];

	for (int i=0;	i<numControllers;	i++) {
		for (int j=0; j<numControllers; j++) {

			if (mask->Get(i,j)<MASK_THRESHOLD) {
				Extract_Feature_Vector(in, controllers[i], controllers[j]);
				target[0] = Target(i, j, preferences);
				ANN->bpgt(in, target);
			}
		}
	}
	delete [] in;
	delete [] target;
}

int USER_MODEL::Errors_On_Matrix_With_Mask(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers, MATRIX *mask) {

	double* in = new double[TAU_INPUTS];
	double target, scorePrediction;
	int errors = 0;

	for (int i=0;	i<numControllers;	i++) {
		for (int j=0; j<numControllers; j++) {

			if (mask->Get(i,j)<MASK_THRESHOLD) {
				scorePrediction = Predict(controllers[i], controllers[j]);
				target = Target(i, j, preferences);

				if( (scorePrediction < 0.5 && target > 0.5) ||
						(scorePrediction > 0.5 && target < 0.5) )
					errors++;
			}
		}
	}

	delete [] in;
	return errors;
}

int USER_MODEL::Errors_On_Matrix_With_Inverted_Mask(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers, MATRIX *mask) {

	double* in = new double[TAU_INPUTS];
	double target, scorePrediction;
	int errors = 0;

	for (int i=0;	i<numControllers;	i++) {
		for (int j=0; j<numControllers; j++) {

			if (mask->Get(i,j)>=MASK_THRESHOLD) {
				scorePrediction = Predict(controllers[i], controllers[j]);
				target = Target(i, j, preferences);

				if( (scorePrediction < 0.5 && target > 0.5) ||
						(scorePrediction > 0.5 && target < 0.5) )
					errors++;
			}
		}
	}

	delete [] in;
	return errors;
}

double USER_MODEL::Target(int i, int j, MATRIX* pref) {

	double target;
	if (pref->Get(i,j) > 0 )
		target = 1.0;
	else if (pref->Get(i,j) < 0)
		target = 0.0;
	else
		target = 0.5;

	return target;
}

#endif
