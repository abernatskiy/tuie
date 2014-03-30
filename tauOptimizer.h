#ifndef _TAU_OPTIMIZER_H
#define _TAU_OPTIMIZER_H

#include "neuralNetwork.h"
#include "userModel.h"

class TAU_OPTIMIZER {

private:
	USER_MODEL *model;
	int id;

public:
	double 	    modelError;

	TAU_OPTIMIZER(int id_no);
	TAU_OPTIMIZER(ifstream *inFile);
	~TAU_OPTIMIZER(void);
	void   Optimize(int numControllers, MATRIX *preferences, NEURAL_NETWORK **controllers);
	void   Optimize_Common(int numControllers0, MATRIX *preferences0, NEURAL_NETWORK **controllers0, int numControllers1, MATRIX *preferences1, NEURAL_NETWORK **controllers1);
//	void   Print_Predictions(USER_MODEL *model, int numControllers, NEURAL_NETWORK **controllers);
	int    Ready_To_Predict(void);
	double Model_Error(void);
	void   Save(ofstream *outFile);
	double Score_Predict(NEURAL_NETWORK *controller1, NEURAL_NETWORK *controller2);
//	void   User_Models_Reset(int numControllers, NEURAL_NETWORK **controllers);
};

#endif
