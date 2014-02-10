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

	layerSizes[0] = 7;
	layerSizes[1] = 6;
	layerSizes[2] = 1;

	ANN = new CBackProp(3,layerSizes,0.1,0.1);

	delete [] layerSizes;
	layerSizes = NULL;
}

double USER_MODEL::Evaluate(int numControllers, MATRIX* preferences, NEURAL_NETWORK **controllers) {

//	printf("USER_MODEL: Backpropagating on  %d controllers\n", numControllers);

//	printf("USER_MODEL: Backpropagating on  %d controllers, IDs: ", numControllers);
//	  for(int i=0; i<numControllers; i++)
//	   printf("%d(%2.2f) ", controllers[i]->ID, controllers[i]->Score_Get());
//  printf("\n");

	double scorePrediction;
	int errors;
	double* in = new double[7];
	double* target = new double[1];

	delete ANN;
	Allocate_ANN();

	int maxIterations = TAU_BACK_PROP_ITER_MAX_MULTIPLIER*numControllers*(numControllers-1)/2;

//	printf("Training user model on the following matrix:\n");
//	preferences->Print(2);

	for (int iter=0; iter<maxIterations; iter++) {

		for (int i=0;	i<(numControllers-1);	i++) {
			for (int j=i+1; j<numControllers; j++) {

				Extract_Feature_Vector(in, controllers[i], controllers[j]);

				if (preferences->Get(i,j) > 0 )
						target[0] = 1.0;
				else if (preferences->Get(i,j) < 0)
						target[0] = 0.0;
				else
						target[0] = 0.5;

				ANN->bpgt(in,target);

			}
		}

		errors = 0;
		for (int i=0;	i<(numControllers-1);	i++) {
			for (int j=i+1; j<numControllers; j++) {

				Extract_Feature_Vector(in, controllers[i], controllers[j]);

				ANN->ffwd(in);
				scorePrediction = ANN->Out(0);

				if (preferences->Get(i,j) > 0 )
						target[0] = 1.0;
				else if (preferences->Get(i,j) < 0)
						target[0] = 0.0;
				else
						target[0] = 0.5;

//				printf("bppgtd %d vs %d, target %2.2lf, got %2.2lf\n", controllers[i]->ID, controllers[j]->ID, target[0], scorePrediction);

				if( (scorePrediction < 0.5 && target[0] > 0.5) ||
						(scorePrediction > 0.5 && target[0] < 0.5) )
					errors++;
			}
		}
//		printf("\n");

		if (errors == 0) {
			delete [] in;
			delete [] target;
			return 100.0/((double) iter);
		}
	}

	printf("WARNING - Couldn't train the network, got %d errors (out of %d controllers) after %d iterations\n", errors, numControllers, maxIterations);
	return 10000.0;
}

void USER_MODEL::Extract_Feature_Vector(double* in, NEURAL_NETWORK* controller1, NEURAL_NETWORK* controller2) {

	int m=0;
	for( int k=0; k<=10; k=k+2 ) {
		in[m] = controller2->sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0),k) -
			controller1->sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0),k);
		m++;
  }
	if( controller1->ID > controller2->ID )
		in[m] = 1.0;
	else if( controller1->ID < controller2->ID )
		in[m] = -1.0;
	else
		in[m] = 0.0;

//			in[0] = sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0), 11);
//			in[1] = sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0), 12);
//			in[2] = 0;
//			in[3] = 0;
//			in[4] = 0;
//			in[5] = 0;
//			in[6] = 1.0;
}

double USER_MODEL::Predict(NEURAL_NETWORK *controller1, NEURAL_NETWORK *controller2) {

	double* in = new double[7];
	Extract_Feature_Vector(in, controller1, controller2);
  ANN->ffwd(in);
	delete [] in;
	return( ANN->Out(0) );
//	return( 2*(ANN->Out(0) - 0.5) );
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

#endif
