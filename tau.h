#ifndef _TAU_H
#define _TAU_H

#include "neuralNetwork.h"
#include "optimizer.h"
#include "tauOptimizer.h"

/*** The Approximate User ***/

class TAU {

public:
	int	timer; // used in CLIENT mode
	int numControllers;
	NEURAL_NETWORK **controllers;
	MATRIX *preferences;

	// used in CLIENT mode for addressing the controllers when starting simulation
	// AND in SERVER mode when generating a new pair of controllers to send to the client
	int		 firstControllerIndex;
	int		 secondControllerIndex;

	TAU_OPTIMIZER   *tauOptimizer;
	int		 nextControllerForTraining;
	double scoreMin;
	double scoreMax;

	bool commonTAU;
	int conflicts;
	int ambiguities;

	bool requestFromCommonTAU;
	bool currentJobFromCommonTAU;
	NEURAL_NETWORK* remoteController;

public:
	TAU(void);
	TAU(ifstream *inFile);
	TAU(TAU* tau0, TAU* tau1);
	~TAU(void);

	void	Store_Pref(int firstID, int secondID, int pref); // Is called from ENVS when reading preferences supplied by clients
																												// After a couple of procedure levels calls Optimize() (see below)

	int		All_Required_Preferences_Supplied(void); // Searches the preference matrix for non-diagonal elements
																								// Might be useful for the implementaion of maturity

	void	Controller_First_Store_Sensor_Data(MATRIX *sensorData);
	void	Controller_Second_Store_Sensor_Data(MATRIX *sensorData);

	NEURAL_NETWORK *Controller_Get_Best(void);
	NEURAL_NETWORK *Controller_Pair_Get_First(void);
	NEURAL_NETWORK *Controller_Pair_Get_Second(void);
	void		Controllers_Load_Pair(ifstream *inFile);
	NEURAL_NETWORK**		Controllers_Save_Pair(OPTIMIZER *optimizer, ofstream *outFile);
	NEURAL_NETWORK**		Controllers_Save_Pair(OPTIMIZER *optimizer, TAU* other, ofstream *outFile);
 	void    Controllers_Select_From_Optimizer(OPTIMIZER *optimizer); // Pair selection implementation core component
																																	// Selects varying combinations of ANNs from TAU and from AFPO population depending on
																																	// value of Controllers_Num_Needed_From_Optimizer()
																																	// Might be useful for sparse preference matrix implementation
 	void    Controllers_Select_From_Optimizer(OPTIMIZER *optimizer, TAU* other);

	void		Optimize(void); // Is called from Scores_Update()
													// Gets a variable "controllers" and the return value of Controllers_Available_For_Optimization()
													// and passes them all the way down to USER_MODEL::Evaluate( .. ).
													// Scores and sensorTimeSeries (the entire matrix for each controller) are stored in controllers as member variables.

													// USER_MODEL::Evaluate() works by backpropagating the ANN user model TAU_BACK_PROP_TRAINING_ITERATIONS times for each controller
													// among the first Controllers_Available_For_Optimization() NEURAL_NETWORK instances in "controllers"

													// The network is created upon the first call of Optimize() and is never reset

													// It returns average (used to be total before my modifications) prediction error of the ANN as it was registered during
													// the backpropagation process on the data listed above (NOT all data in history!).
													// Unbiased backpropagation result is used when evaluating the error.

													// The error value is available as a member variable of tauOptimizer.
													// It is updated every time this function or User_Models_Reset() is called.

	void    Print(void);
	int			Ready_To_Predict(void); // Returns tauOptimizer->Ready_To_Predict()
																	// May be useful in maturity implementation
	double	Model_Error(void);

	void		Save(ofstream *outFile);
	double	Score_Predict(NEURAL_NETWORK *controller); // biased - returns a pow(, 0.3) of a score predicted by the backpropagated network of tau - see USER_MODEL::Predict
//	void		User_Models_Reset(void); // never really called
  void    Controller_Store(NEURAL_NETWORK *newController); // stores a copy of a given controller to controllers[] if there are none present with such ID
  void    Controller_Store_Without_ID_Check(NEURAL_NETWORK *newController); // stores a copy of a given controller to controllers[] unconditionally

	void		Scores_Check(void);

	int			Find_Index(int ID);

  int     Controllers_Available_For_Optimization(void); // returns no of controllers with available scores (good scores) and sensor data

private:
//	void  Controller_First_Preferred(void);  // these two functions are called only from
//	void	Controller_Second_Preferred(void); // Store_Pref(int, int, int)

	void		Controller_Store_Sensor_Data(int controllerIndex, MATRIX *sensorData);
	void    Controllers_Expand(void);
	void    Controllers_Initialize(void);
	int		  Controllers_Num_Needed_From_Optimizer(void); // read this function before rewriting Controllers_Select_From_Optimizer()
	void    Controllers_Print(void);
	void		Controllers_Select_One_From_TAU_One_From_Optimizer(OPTIMIZER *optimizer);
	void		Controllers_Select_One_From_TAU_One_From_Optimizer(OPTIMIZER *optimizer, TAU* other);
	void		Controllers_Select_Two_From_Optimizer(OPTIMIZER *optimizer);
	void		Controllers_Select_Two_From_Optimizer(OPTIMIZER *optimizer, TAU* other);
	void		Controllers_Select_Two_From_TAU(void);
	int			Num_Prefs(void);
	void		Preferences_Expand(void);
	void		Preferences_Initialize(void);
	void		Preferences_Print(void);
	int			Ready_For_Optimization(void);
  double	Scale(	double value, double min1, double max1,
                             	double min2, double max2);
	void		Scores_Print(void);
	void		Scores_Update(void); // is called from Controller_First_Preferred(), Controller_Second_Preferred()
	void		Storage_Expand(void);
	void		Storage_Initialize(void);
};

double distance(TAU* tau0, TAU* tau1, int id0, int id1);

#endif
