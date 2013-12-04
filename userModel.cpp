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
extern int    TAU_BACK_PROP_TRAINING_ITERATIONS;

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

/*	int *layerSizes = new int[3];

	layerSizes[0] = 7;
	layerSizes[1] = 2;
	layerSizes[2] = 1;

	ANN = new CBackProp(3,layerSizes,0.1,0.1);*/

	int *layerSizes = new int[2];

	layerSizes[0] = 1;
	layerSizes[1] = 1;

	ANN = new CBackProp(2,layerSizes,0.1,0.1);

	delete [] layerSizes;
	layerSizes = NULL;
}

double USER_MODEL::Evaluate(int numControllers, NEURAL_NETWORK **controllers) {

	printf("USER_MODEL: Backpropagating on  %d controllers\n", numControllers);
	double score, scorePrediction;
	double totalError = 0.0;
	double* in = new double[1];
	double* target = new double[1];

	delete ANN;
	Allocate_ANN();

	for (int j=0; j<TAU_BACK_PROP_TRAINING_ITERATIONS; j++) {
		for (int i=0;	i<numControllers;	i++) {

			MATRIX *sensorTimeSeries = controllers[i]->sensorTimeSeries;
//			int sensorRow;
			score = controllers[i]->Score_Get();

//			int m=0;
//			for (int k=0;k<=10;k=k+2) {
//				//in[m] = sensorTimeSeries->Get(STARTING_EVALUATION_TIME-1,k);
//				in[m] = sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0),k);
//				m++;
//			}
//			in[m] = 1.0; // Bias node, in[6]

			in[0] = sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0), 11);
//			in[1] = sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0), 12);
//			in[2] = 0;
//			in[3] = 0;
//			in[4] = 0;
//			in[5] = 0;
//			in[6] = 1.0;

			target[0] = score;

			ANN->bpgt(in,target);

			scorePrediction = ANN->Out(0); // raw, unbiased prediction

			if( j == TAU_BACK_PROP_TRAINING_ITERATIONS-1)
				totalError = totalError + fabs(score-scorePrediction);
//				totalError += ANN->mse(target);

			sensorTimeSeries = NULL;
		}
	}

	delete [] in;
	delete [] target;

	return( totalError / ((double) numControllers) );
}

double USER_MODEL::Predict(MATRIX *sensorTimeSeries) {

//	int i;
	double *in;
	in = new double[1];
	in[0] = sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0), 11);

/*	int m=0;
	for (int k=0;k<=10;k=k+2) {
		//in[m] = sensorTimeSeries->Get(STARTING_EVALUATION_TIME-1,k);
		in[m] = sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0),k);
		m++;
  }
	in[m] = 1.0; // Bias neuron*/

  ANN->ffwd(in);
	delete [] in;
	return( pow(ANN->Out(0),0.3) ); // artificial bias introduced to avoid underestimating individuals with low predicted score
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
