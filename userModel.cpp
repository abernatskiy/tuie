#ifndef _USER_MODEL_CPP
#define _USER_MODEL_CPP

#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "string.h"

#include "userModel.h"

extern double TAU_NO_SCORE;
extern int    TAU_NUM_NEURONS;
extern int    STARTING_EVALUATION_TIME;
extern int    ALG_VARIANT_PREFS_ONLY;
extern int    TAU_NUM_SENSOR_ROWS;
extern int    TAU_BACK_PROP_ITER_MAX_MULTIPLIER;

int TAU_INPUTS = 6;

USER_MODEL::USER_MODEL(int id_no, int numS) {

	ready = false;
	id = id_no;
	numSensors = numS;

	ANN = NULL;
}

USER_MODEL::~USER_MODEL(void) {

}

double USER_MODEL::Evaluate(int numControllers, MATRIX* preferences, NEURAL_NETWORK **controllers) {

	int errors = Errors_On_Matrix(numControllers, preferences, controllers);

	FILE* server_input = Server_Input();
	fprintf(server_input, "train\n");
	Learn_On_Matrix(server_input, numControllers, preferences, controllers);
	fclose(server_input);

	FILE* server_output = Server_Output();
	char reply[100];
	fscanf(server_output, "%s", reply);
	fclose(server_output);

	printf("Server's reply was %s\n", reply);
	if(strcmp(reply, "OK") != 0)
		printf("That's not OK\n");

	ready = true;

	printf("Trained user model %d on the following:\n", id);
	preferences->PrintWithSums(2);

	MATRIX* demo0 = new MATRIX(numControllers, numControllers, 0.0);
	for( int i=0; i<numControllers; i++ )
		for( int j=0; j<numControllers; j++ )
			demo0->Set(i, j, Predict(controllers[i], controllers[j]));

	printf("Learned matrix:\n");
	demo0->PrintWithSums(2);

	delete demo0;

	return ((double) errors)/((double) (numControllers*numControllers));
}

double USER_MODEL::Evaluate_Common(int numControllers0, MATRIX *preferences0, NEURAL_NETWORK **controllers0, int numControllers1, MATRIX *preferences1, NEURAL_NETWORK **controllers1) {

	int errors = Errors_On_Matrix(numControllers0, preferences0, controllers0) + Errors_On_Matrix(numControllers1, preferences1, controllers1);

	FILE* server_input = Server_Input();
	fprintf(server_input, "train\n");
	Learn_On_Matrix(server_input, numControllers0, preferences0, controllers0);
	Learn_On_Matrix(server_input, numControllers1, preferences1, controllers1);
	fclose(server_input);

	FILE* server_output = Server_Output();
	char reply[100];
	fscanf(server_output, "%s", reply);
	fclose(server_output);

	printf("Server's reply was %s\n", reply);
	if(strcmp(reply, "OK") != 0)
		printf("That's not OK\n");

	ready = true;

	printf("Trained user model %d on the following:\n", id);
	printf("Matrix0 (using only upper-left %d x %d submatrix):\n", numControllers0, numControllers0);
	preferences0->PrintWithSums(2);
	printf("Matrix1 (using only upper-left %d x %d submatrix):\n", numControllers1, numControllers1);
	preferences1->PrintWithSums(2);

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

	return ((double) errors)/((double) (numControllers0*numControllers0 + numControllers1*numControllers1));
}

double USER_MODEL::Predict(NEURAL_NETWORK *controller1, NEURAL_NETWORK *controller2) {

	if( !ready )
		return TAU_NO_SCORE;

	double in[TAU_INPUTS];

	FILE* server_input = Server_Input();
	fprintf(server_input, "predict\n");

	Extract_Feature_Vector(in, controller1, controller2);
	for(int k=0; k<TAU_INPUTS-1; k++)
		fprintf(server_input, "%le ", in[k]);
	fprintf(server_input, "%le\n", in[TAU_INPUTS-1]);

	fclose(server_input);

	double curpred;
	FILE* server_output = Server_Output();
	fscanf(server_output, "%lf\n", &curpred);
	fclose(server_output);

	return curpred; // or the inverse of Target()
}

/***** PRIVATE METHODS *****/

void USER_MODEL::Extract_Feature_Vector(double* in, NEURAL_NETWORK* controller1, NEURAL_NETWORK* controller2) {

	int m=0;

	for( int k=0; k<=10; k=k+2 ) {
		in[m] =	controller1->sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0),k);
		m++;
  }

	m=0;

	for( int k=0; k<=10; k=k+2 ) {
		in[m] = in[m] - controller2->sensorTimeSeries->Get(int(double(STARTING_EVALUATION_TIME)/2.0),k);
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

void USER_MODEL::Learn_On_Matrix(FILE* server_input, int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers) {

	double in[TAU_INPUTS];

	for (int i=0;	i<numControllers;	i++) {
		for (int j=0; j<numControllers; j++) {

			Extract_Feature_Vector(in, controllers[i], controllers[j]);

			for(int k=0; k<TAU_INPUTS; k++)
				fprintf(server_input, "%le ", in[k]);
			fprintf(server_input, "%le\n", Target(i, j, preferences));
		}
	}
}

int USER_MODEL::Errors_On_Matrix(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers) {

	if( !ready )
		return -1;

	int errors = 0;
	double in[TAU_INPUTS];

	FILE* server_input = Server_Input();
	fprintf(server_input, "predict\n");

	for (int i=0;	i<numControllers;	i++) {
		for (int j=0; j<numControllers; j++) {

			Extract_Feature_Vector(in, controllers[i], controllers[j]);

			for(int k=0; k<TAU_INPUTS-1; k++)
				fprintf(server_input, "%le ", in[k]);
			fprintf(server_input, "%le\n", in[TAU_INPUTS-1]);
		}
	}

	fclose(server_input);

	double curpred;
	double threshold = 0.0;
	FILE* server_output = Server_Output();
	for (int i=0;	i<numControllers;	i++) {
		for (int j=0; j<numControllers; j++) {

			fscanf(server_output, "%lf\n", &curpred);
			if( ( curpred<=threshold && Target(i, j, preferences)>0 ) ||
					( curpred>threshold && Target(i, j, preferences)<=0 ) )
				errors++;
		}
	}

	fclose(server_output);

	return errors;
}

double USER_MODEL::Target(int i, int j, MATRIX* pref) {

	return pref->Get(i,j); // see also Errors_on_matrix() and Predict()
}

FILE* USER_MODEL::Server_Input(void) {

	char fn[100];
	sprintf(fn, "mlserver/input%d", id);
	return fopen(fn, "w");
}

FILE* USER_MODEL::Server_Output(void) {

	char fn[100];
	sprintf(fn, "mlserver/output%d", id);
	return fopen(fn, "r");
}

/* state saving-related funcs */

USER_MODEL::USER_MODEL(ifstream *inFile) {

  (*inFile) >> numSensors;

	int isANN;

	(*inFile) >> isANN;

//	if ( isANN )
//		ANN = new CBackProp(inFile);
//	else
//		ANN = NULL;
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
