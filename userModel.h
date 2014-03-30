#ifndef _USER_MODEL_H
#define _USER_MODEL_H

#include "BackProp.h"
#include "neuralNetwork.h"

class USER_MODEL {

private:
	int id;

	int numSensors;
	CBackProp *ANN;
	void Extract_Feature_Vector(double* vector, NEURAL_NETWORK *controller1, NEURAL_NETWORK *controller2);

	void Learn_On_Matrix(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers);
	int Errors_On_Matrix(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers);
	void Learn_On_Matrix_With_Mask(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers, MATRIX* mask);
	int Errors_On_Matrix_With_Mask(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers, MATRIX* mask);
	int Errors_On_Matrix_With_Inverted_Mask(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers, MATRIX* mask);

	double Target(int i, int j, MATRIX *preferences);

public:
	USER_MODEL(int id_no, int numS);
	USER_MODEL(ifstream *inFile);
	~USER_MODEL(void);
	void Allocate_ANN(void);
	double Evaluate(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers); // backpropagates
	double Evaluate_Common(int numControllers0, MATRIX *preferences0, NEURAL_NETWORK **controllers0, int numControllers1, MATRIX *preferences1, NEURAL_NETWORK **controllers1); // backpropagates
	double Predict(NEURAL_NETWORK *controller1, NEURAL_NETWORK *controller2);
	void   Save(ofstream *outFile);
};

#endif
