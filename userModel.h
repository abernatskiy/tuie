#ifndef _USER_MODEL_H
#define _USER_MODEL_H

#include "BackProp.h"
#include "neuralNetwork.h"

class USER_MODEL {

private:
	int numSensors;
	CBackProp *ANN;

public:
	USER_MODEL(int numS);
	USER_MODEL(ifstream *inFile);
	~USER_MODEL(void);
	void Allocate_ANN(void);
	double Evaluate(int numControllers, NEURAL_NETWORK **controllers); //backpropagates
	double Predict(MATRIX *sensorTimeSeries);
	void   Save(ofstream *outFile);
};

#endif
