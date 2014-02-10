#ifndef _USER_MODEL_H
#define _USER_MODEL_H

#include "BackProp.h"
#include "neuralNetwork.h"

class USER_MODEL {

private:
	int numSensors;
	CBackProp *ANN;
	void Extract_Feature_Vector(double* vector, NEURAL_NETWORK *controller1, NEURAL_NETWORK *controller2);

public:
	USER_MODEL(int numS);
	USER_MODEL(ifstream *inFile);
	~USER_MODEL(void);
	void Allocate_ANN(void);
	double Evaluate(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers); // backpropagates
	double Predict(NEURAL_NETWORK *controller1, NEURAL_NETWORK *controller2);
	void   Save(ofstream *outFile);
};

#endif
