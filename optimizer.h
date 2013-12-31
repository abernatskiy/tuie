
#ifndef _OPTIMIZER_H
#define _OPTIMIZER_H

#include "matrix.h"
#include "neuralNetwork.h"

class OPTIMIZER {

public:
	int 		timer;
	int 		evaluationPeriod;

	int 		numSensors;
	int 		numMotors;

	double		mutationProbability;

	long 		nextGenomeID;

	NEURAL_NETWORK **genomes;

	NEURAL_NETWORK *genomeUnderEvaluation; // seems to be obsolete - no memory allocation for this pointer can be found in code
																				// remove where possible

	int		generation;

public:
	OPTIMIZER(int numberOfSensors, int numberOfMotors);
	OPTIMIZER(ifstream *inFile);
	~OPTIMIZER(void);
	void    	EvaluationPeriod_Decrease(void);
	void    	EvaluationPeriod_Increase(void);

//	void		Data_Receive(double fitness, double score, MATRIX *timeSeries, NEURAL_NETWORK *userFavorite, bool startAFPO);

	void		Genome_Discard_Being_Evaluated(void);
	NEURAL_NETWORK *Genome_Get(int i);
	NEURAL_NETWORK *Genome_Get_Best(void);
	NEURAL_NETWORK *Genome_Get_Best_But_Not(NEURAL_NETWORK *other);
	NEURAL_NETWORK *Genome_Get_Best_But_Not(int numControllers, NEURAL_NETWORK **controllers);
	NEURAL_NETWORK *Genome_Get_Curr_To_Evaluate(void);
	NEURAL_NETWORK *Genome_Get_First(void);
	NEURAL_NETWORK *Genome_Get_Most_Different(int numControllers, NEURAL_NETWORK **controllers);
	NEURAL_NETWORK *Genome_Get_Most_Different_But_Not(NEURAL_NETWORK *thisOne, int numControllers, NEURAL_NETWORK **controllers);
	NEURAL_NETWORK *Genome_Get_Most_Different_But_Not(int numExiles, NEURAL_NETWORK **exiles, int numControllers, NEURAL_NETWORK **controllers);
	NEURAL_NETWORK *Genome_Get_Random(void);
	NEURAL_NETWORK *Genome_Get_Random_But_Not(NEURAL_NETWORK *other);
	NEURAL_NETWORK *Genome_Get_Random_But_Not(int numControllers, NEURAL_NETWORK **controllers);
	NEURAL_NETWORK *Genome_Get_Second(void);
	void		Genome_Put_At_End(NEURAL_NETWORK *other);
	int			Genomes_Num_Of_Evaluated(void);
	void		Load(ifstream *inFile);
	void    MutationProbability_Decrease(void);
	void    MutationProbability_Increase(void);
	void    Print(void);
	void		Reset(void);
	void		Reset_Genomes(void);
	void		Save(ofstream *outFile);
	void		Score_Receive(double score);
	void		Scores_Reset(void);
	void		Sensor_Data_Receive(MATRIX *timeSeries);
	int		Time_Elapsed(void);
	void		Timer_Reset(void);
	void		Timer_Update(void);

	NEURAL_NETWORK *Genome_Get_Next_To_Evaluate(void); // DOES NOT call Generation_Create_Next() if Genomes_All_Evaluated() anymore
																										// returns NULL if Genomes_All_Evaluated()
																										// returns pointer to next not evaluated genome otherwise (by linear search implemented in Genome_Find_Next_Not_Evaluated())
	int			Genomes_All_Evaluated(void);
	int			Genomes_All_Scored(void);
	void		Generation_Create_Next(void); // generational function of AFPO - replaces current population with next gen population


private:
	void    Destroy(void);
	int			FlipCoin(void);
	void		Genome_Copy(int genomeIndex, int parentID);
	void    Genome_Create_Random(int genomeIndex);
	void    Genome_Destroy(int genomeIndex);
	int  		Genome_Evaluated(int genomeIndex); // returns true if NEURAL_NETWORK::Fitness_Set() was called on genome
	int  		Genome_Scored(int genomeIndex); // returns true if NEURAL_NETWORK::Score_Set() was called on genome
	NEURAL_NETWORK *Genome_Find_Next_Not_Evaluated(void);
	void 		Genome_Load(int genomeIndex, ifstream *inFile);
	void		Genome_Print(int genomeIndex);
	void		Genomes_Create(void);
	void		Genomes_Delete_Dominated(void);
	void		Genomes_Destroy(void);
	void		Genomes_Fill_Empty_Slots(void);
	void    Genomes_Find_Pareto_Front(void);
	void		Genomes_Increase_Age(void);
	void		Genomes_Inject_Random_Genome(void);
	void		Genomes_Inject_User_Favorite(NEURAL_NETWORK *userFavorite);
	void		Genomes_Load(ifstream *inFile);
	void    	Genomes_Print(void);
	void 		Genomes_Save(ofstream *outFile);
	void		Genomes_Sort(void);
	void		Genomes_Swap(int firstIndex, int secondIndex);
	void		Initialize(void);
	double		Rand(double min, double max);
	int     	RandInt(int min, int max);
};

#endif
