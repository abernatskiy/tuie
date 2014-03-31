#ifndef _USER_MODEL_H
#define _USER_MODEL_H

#include "BackProp.h"
#include "neuralNetwork.h"

class USER_MODEL {

private:
	bool ready;

	int id;

	int numSensors;
	CBackProp *ANN;

	void Extract_Feature_Vector(double* vector, NEURAL_NETWORK *controller1, NEURAL_NETWORK *controller2);

	void Learn_On_Matrix(FILE* server_input, int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers);
	int Errors_On_Matrix(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers);

	double Target(int i, int j, MATRIX *preferences);

	FILE* Server_Input(void);
	FILE* Server_Output(void);

public:
	USER_MODEL(int id_no, int numS);
	USER_MODEL(ifstream *inFile);
	~USER_MODEL(void);
	double Evaluate(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers);
	double Evaluate_Common(int numControllers0, MATRIX *preferences0, NEURAL_NETWORK **controllers0, int numControllers1, MATRIX *preferences1, NEURAL_NETWORK **controllers1);
	double Predict(NEURAL_NETWORK *controller1, NEURAL_NETWORK *controller2);
	void   Save(ofstream *outFile);
};

#endif
