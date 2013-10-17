#include "stdio.h"
#include "sys/stat.h"
#include <time.h>	// for fitness timestamp
#include <unistd.h> // for getpid()

#include <ode/ode.h>
#include <drawstuff/drawstuff.h>

//#include <GL/glut.h>

#ifndef _ENVS_CPP
#define _ENVS_CPP

//#include "constants.h"
#include "envs.h"
#include "optimizer.h"

extern int	MODE_VIEW_SIDE;
extern int	MODE_VIEW_TOP;
extern int	MODE_VIEW_BACK;

extern int	MODE_SIMULATE_DESIGN;
extern int	MODE_SIMULATE_EVOLVE;
extern int	MODE_SIMULATE_CHAMP;
extern int	MODE_SIMULATE_TAU;

extern int	TIME_STEPS_PER_FRAME;

extern int	SELECTION_LEVEL_OBJECT;
extern int	SELECTION_LEVEL_ROBOT;
extern int	SELECTION_LEVEL_ENVIRONMENT;
extern int	SELECTION_LEVEL_ENVS;

extern int	MAX_ENVIRONMENTS;

extern double	MOVE_INCREMENT;

extern int	MAX_EVALS_BEFORE_SAVING;

extern double	WORST_FITNESS;

extern int	TIME_TO_CHECK_FOR_NEUTRAL_MUTATION;

extern int	AFPO_POP_SIZE;

extern int	ALG_VARIANT_FITS_ONLY;
extern int	ALG_VARIANT_PREFS_ONLY;

extern int	STARTING_EVALUATION_TIME;

extern double TAU_NO_SCORE;

extern double TAU_OPTIMIZER_ERROR_UNDEFINED;
extern double TAU_OPTIMIZER_ERROR_ACCEPTABLE;

ENVS::ENVS(int rs) {

	speed = 1;

	randSeed = rs;

	simulateMode = MODE_SIMULATE_DESIGN;

	numberOfEnvs = 1;
	currNumberOfEnvs = 1;

	taskEnvironments = new ENVIRONMENT * [MAX_ENVIRONMENTS];

	for (int i=0;	i<MAX_ENVIRONMENTS;	i++)
		taskEnvironments[i] = NULL;

	taskEnvironments[0] = new ENVIRONMENT();
	taskEnvironments[0]->Add_Light_Source();
	taskEnvironments[0]->Add_Robot_Sandbox();
	taskEnvironments[0]->Set_Color(1,0,0);

	optimizer = NULL;
	targetSensorValuesRecorded = false;
	recordingVideo = false;

//	tau = new TAU();
	taus = NULL;

	savedFileIndex = -1;

	selectionLevel = SELECTION_LEVEL_OBJECT;

	activeEnvironment = 0;

	evaluationsSinceLastSave = 0;
	evaluationsSinceLastPreference = 0;

	startTime = clock();

	timeSinceLastWriteout = startTime;

	client = new CLIENT();
	server = new SERVER();

	controllerUnderEvaluation = NULL;

	if( !(In_Design_Mode() && client->pairFileExists()) ) {
		char command[200];
		sprintf(command,"rm SavedFiles/writeout%d.txt",randSeed);
		system(command);
		sprintf(command,"rm SavedFiles/pair*.dat");
		system(command);
		sprintf(command,"rm SavedFiles/pref*.dat");
		system(command);
		sprintf(command,"rm SavedFiles/score_type.log");
		system(command);
	}
}

ENVS::~ENVS(void) {

	if ( optimizer ) {
		delete optimizer;
		optimizer = NULL;
	}

	for (int i=0;	i<MAX_ENVIRONMENTS;	i++) {

		if ( taskEnvironments[i] ) {
			delete taskEnvironments[i];
			taskEnvironments[i] = NULL;
		}
	}
	delete[] taskEnvironments;
	taskEnvironments = NULL;

	if ( taus ) {
		delete taus;
		taus = NULL;
	}

	delete client;
	delete server;
}

void ENVS::Active_Element_Copy(void) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )
		if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )
			Environment_Copy();
		else if ( selectionLevel == SELECTION_LEVEL_OBJECT )
			taskEnvironments[activeEnvironment]->Active_Element_Copy();
		else if ( selectionLevel == SELECTION_LEVEL_ROBOT )
			taskEnvironments[activeEnvironment]->Active_Component_Copy();
}

void ENVS::Active_Element_Delete(void) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )
		if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )
			Environment_Delete();
		else if ( selectionLevel == SELECTION_LEVEL_OBJECT )
			taskEnvironments[activeEnvironment]->Active_Element_Delete();
		else if ( selectionLevel == SELECTION_LEVEL_ROBOT )
			taskEnvironments[activeEnvironment]->Active_Component_Delete();
}

void ENVS::Active_Element_Load(void) {

	if ( simulateMode == MODE_SIMULATE_DESIGN ) {
		if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )
			taskEnvironments[activeEnvironment]->Load();
		else if ( selectionLevel == SELECTION_LEVEL_ENVS )
			Load(true);
		// Robots during design are always red.
		for (int i=0;	i<numberOfEnvs;	i++) {
//			taskEnvironments[i]->Robots_Set_Color(1,0,0);
			taskEnvironments[i]->Robots_Recolor('r');
		}
	}
}

void ENVS::Active_Element_Mark(void) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )
		if ( selectionLevel == SELECTION_LEVEL_OBJECT )
			taskEnvironments[activeEnvironment]->Mark_Object();
		else if ( selectionLevel == SELECTION_LEVEL_ROBOT )
			taskEnvironments[activeEnvironment]->Mark_Component();
}

void ENVS::Active_Element_Unmark(void) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )
		if ( selectionLevel == SELECTION_LEVEL_OBJECT )
			taskEnvironments[activeEnvironment]->Unmark_Object();
		else if ( selectionLevel == SELECTION_LEVEL_ROBOT )
			taskEnvironments[activeEnvironment]->Unmark_Component();
}

void ENVS::Active_Element_Move(double x, double y, double z) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )
		if ( selectionLevel == SELECTION_LEVEL_ENVS )
			Move(x,y,z);
		else if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )
			Environment_Move(x,y,z);
		else if ( selectionLevel == SELECTION_LEVEL_OBJECT )
			taskEnvironments[activeEnvironment]->Active_Element_Move(x,y,z);
		else if ( selectionLevel == SELECTION_LEVEL_ROBOT )
			taskEnvironments[activeEnvironment]->Active_Component_Move(x,y,z);
}

void ENVS::Active_Element_Next(void) {

	if ( simulateMode == MODE_SIMULATE_TAU )
		Environment_Next();
	else if ( simulateMode == MODE_SIMULATE_DESIGN ) {
		if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )
			Environment_Next();
		else if ( selectionLevel == SELECTION_LEVEL_OBJECT )
			taskEnvironments[activeEnvironment]->Active_Element_Next();
		else if ( selectionLevel == SELECTION_LEVEL_ROBOT )
			taskEnvironments[activeEnvironment]->Active_Component_Next();
	}
}

void ENVS::Active_Element_Previous(void) {

	if ( simulateMode == MODE_SIMULATE_TAU )
		Environment_Previous();
	else if ( simulateMode == MODE_SIMULATE_DESIGN ) {
		if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )
			Environment_Previous();
		else if ( selectionLevel == SELECTION_LEVEL_OBJECT )
			taskEnvironments[activeEnvironment]->Active_Element_Previous();
		else if ( selectionLevel == SELECTION_LEVEL_ROBOT )
			taskEnvironments[activeEnvironment]->Active_Component_Previous();
	}
}

void ENVS::Active_Element_Resize(double changeX, double changeY, double changeZ) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )
		if ( selectionLevel == SELECTION_LEVEL_OBJECT )
			taskEnvironments[activeEnvironment]->Active_Element_Resize(changeX,changeY,changeZ);
		else if ( selectionLevel == SELECTION_LEVEL_ROBOT )
			taskEnvironments[activeEnvironment]->Active_Component_Resize(changeX,changeY,changeZ);
}

void ENVS::Active_Element_Rotate(double rotX, double rotY, double rotZ) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )
		if ( selectionLevel == SELECTION_LEVEL_OBJECT )
			taskEnvironments[activeEnvironment]->Active_Element_Rotate(rotX,rotY,rotZ);
		else if ( selectionLevel == SELECTION_LEVEL_ROBOT )
			taskEnvironments[activeEnvironment]->Active_Component_Rotate(rotX,rotY,rotZ);
}

void ENVS::Active_Element_Save(void) {

	if ( selectionLevel == SELECTION_LEVEL_ENVIRONMENT )
		taskEnvironments[activeEnvironment]->Save();
	else if ( selectionLevel == SELECTION_LEVEL_ENVS )
		Save(true);
}

void ENVS::Joint_Connect(void) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )
//		if ( selectionLevel == SELECTION_LEVEL_OBJECT )
//			taskEnvironments[activeEnvironment]->Connect_Object_Joint();
//		else if ( selectionLevel == SELECTION_LEVEL_ROBOT )
		if ( selectionLevel == SELECTION_LEVEL_ROBOT )
			taskEnvironments[activeEnvironment]->Connect_Robot_Joint();
}

void ENVS::Joint_Range_Decrease(void) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )
//		if ( selectionLevel == SELECTION_LEVEL_OBJECT )
//			taskEnvironments[activeEnvironment]->Active_Object_Joint_Range_Decrease();
//		else if ( selectionLevel == SELECTION_LEVEL_ROBOT )
		if ( selectionLevel == SELECTION_LEVEL_ROBOT )
			taskEnvironments[activeEnvironment]->Active_Joint_Range_Decrease();
}

void ENVS::Joint_Range_Increase(void) {

	if ( simulateMode == MODE_SIMULATE_DESIGN )
//		if ( selectionLevel == SELECTION_LEVEL_OBJECT )
//			taskEnvironments[activeEnvironment]->Active_Object_Joint_Range_Increase();
//		else if ( selectionLevel == SELECTION_LEVEL_ROBOT )
		if ( selectionLevel == SELECTION_LEVEL_ROBOT )
			taskEnvironments[activeEnvironment]->Active_Joint_Range_Increase();
}


// preferences and pairs deleting logic now resides in interactor.h
/* void ENVS::Delete_Pair(void) {

	char fileName[100];
	if( client->pairFileName(fileName) != 0 ) return;

	char command[100];
	sprintf(command, "rm %s", fileName);
	system(command);
}

void ENVS::Delete_Pref(void) {

	char fileName[100];
	if( client->prefFileName(fileName) != 0 ) return;

	char command[100];
	sprintf(command, "rm %s", fileName);
	system(command);
}*/

void ENVS::Draw(void) {

	for (int i=0;	i<currNumberOfEnvs;	i++) {
		if ( taskEnvironments[i] )
			taskEnvironments[i]->Draw();
	}
}

void ENVS::EvaluationPeriod_Decrease(dWorldID world, dSpaceID space, dJointGroupID contactgroup) {

/*if ( In_Design_Mode() )
		return;
	Destroy_Simulated_Objects();
	optimizer->Reset_But_Keep_Best();
	if ( optimizer )
		optimizer->EvaluationPeriod_Decrease();
*/
}

void ENVS::EvaluationPeriod_Increase(dWorldID world, dSpaceID space, dJointGroupID contactgroup) {

/*if ( In_Design_Mode() )
		return;
	Destroy_Simulated_Objects();
	optimizer->Reset_But_Keep_Best();
	if ( optimizer )
		optimizer->EvaluationPeriod_Increase();
*/
}

void ENVS::Continue_Fitness_Calculation( dWorldID world, dSpaceID space ) {

	// called from Evolve()

	// If there is a robot being evaluated, and its time
	// is not yet up, allow it to move.
	if( !Eval_Finished() ) {
		for( int i=0; i<currNumberOfEnvs; i++ )
			taskEnvironments[i]->Allow_Robot_To_Move(optimizer->timer);
		optimizer->Timer_Update();
	}
	// If the time limit for the robot being evaluated has
	// expired, record its fitness, remove it from the simulator,
	// and prepare for the evaluation of the next robot.
	else {
		if( controllerUnderEvaluation->ID != taskEnvironments[0]->robots[0]->neuralNetwork->ID || // purely debugging code
				controllerUnderEvaluation->ID != optimizer->Genome_Get_Next_To_Evaluate()->ID) {

			printf("Off by 1:\ncontrollerUnderEvaluation\t%x\t%d\ntaskEnvNN\t\t\t%x\t%d\noptimizer->GNTE\t\t\t%x\t%d\nExiting.\n",
					controllerUnderEvaluation, controllerUnderEvaluation->ID,
					taskEnvironments[0]->robots[0]->neuralNetwork, taskEnvironments[0]->robots[0]->neuralNetwork->ID,
					optimizer->Genome_Get_Next_To_Evaluate(), optimizer->Genome_Get_Next_To_Evaluate()->ID);
			exit(1);
		}
		double fitness = taskEnvironments[0]->robots[0]->Fitness_Get(taskEnvironments[0]->robots[1]);
		MATRIX *timeSeries = taskEnvironments[0]->Get_Sensor_Data();

		// feed the data obtained in the current simulation to the optimizer
		controllerUnderEvaluation -> Fitness_Set(fitness);
		controllerUnderEvaluation -> Store_Sensor_Data(timeSeries);

		timeSeries = NULL;
		Destroy_Simulated_Objects();

		optimizer->Timer_Reset();

		// Set up next robot...
		if( !optimizer->Genomes_All_Evaluated() )
			Create_Robot_To_Evaluate(world, space);
	}
}

void ENVS::Evolve( dWorldID world, dSpaceID space ) {

	// called from M3 in SERVER mode

	if( optimizer->Genomes_All_Evaluated() ) {
		if( !taus )
			taus = new TAUS;
		if( taus->readyToPredict() ) {
			if( Check_For_Pref() == 0 ) {
				Rescore_Population();
				taus->writeScoreType();
				optimizer->Generation_Create_Next();
				Create_Robot_To_Evaluate(world, space);
				Save_All_Pairs_For_Pref();
			}
		}
		else {
		// Generation scoring check here, and Genomes_All_Scored() is not waht you're looking for
			Save_All_Pairs_For_Pref();
			if( Check_For_Pref() == 0 )
				usleep(100000);
		}
	}
	else
		Continue_Fitness_Calculation(world, space);

}

int ENVS::In_Champ_Mode(void) {

	return( simulateMode == MODE_SIMULATE_CHAMP );
}

int ENVS::In_Design_Mode(void) {

	return( simulateMode == MODE_SIMULATE_DESIGN );
}

int ENVS::In_Evolution_Mode(void) {

	return( simulateMode == MODE_SIMULATE_EVOLVE );
}

int ENVS::In_TAU_Mode(void) {

	return( simulateMode == MODE_SIMULATE_TAU );
}

void ENVS::Mode_Simulate_Set_Champ(dWorldID world, dSpaceID space) {

	// Switch to visualizing the best robot so far.
	if ( !Ready_For_Champ_Mode() )
		return;

	End_Current_Mode();

	simulateMode = MODE_SIMULATE_CHAMP;

	// The best robot is always green.
	for (int i=0;	i<numberOfEnvs;	i++) {
		taskEnvironments[i]->Set_Color(0,1,0);
		taskEnvironments[activeEnvironment]->Robots_Recolor('g');
	}

	optimizer->Timer_Reset();

	Create_Robot_Current_Best(world,space);
}

void ENVS::Mode_Simulate_Set_Design(void) {

	// Switch to the designing of task environments.
	if ( !Ready_For_Design_Mode() )
		return;

	End_Current_Mode();

	simulateMode=MODE_SIMULATE_DESIGN;

	currNumberOfEnvs = numberOfEnvs;

	// Robots during design are always red.
	for (int i=0;	i<numberOfEnvs;	i++) {
		taskEnvironments[i]->Set_Color(1,0,0);
		taskEnvironments[activeEnvironment]->Robots_Recolor('r');
	}
}

void ENVS::Mode_Simulate_Set_Evolve(dWorldID world, dSpaceID space) {

	// Switch to evolution mode.
	if ( !Ready_For_Evolution_Mode() )
		return;

	End_Current_Mode();

	// Selection level is too low.
	if ( selectionLevel == SELECTION_LEVEL_ROBOT )	// lowest level
		Selection_Level_Raise();

	if ( selectionLevel == SELECTION_LEVEL_OBJECT )
		Selection_Level_Raise();

	// Just finished defining the task environment.
	if ( In_Design_Mode() ) {

		// if the number of sensors or motors has changed,
		// delete the optimizer
		if ( Optimizer_Robot_Mismatch() ) {
			delete optimizer;
			optimizer = NULL;
		}

		Optimizer_Initialize();
	}

	simulateMode=MODE_SIMULATE_EVOLVE;

	// The evolving robots are always blue.
	for (int i=0;i<numberOfEnvs;i++) {
		taskEnvironments[i]->Set_Color(0,0,1);
		taskEnvironments[activeEnvironment]->Robots_Recolor('b');
	}

	// Set up next robot...
	Create_Robot_To_Evaluate(world,space);
	evaluationsSinceLastSave++;
        if ( evaluationsSinceLastSave == AFPO_POP_SIZE ) {
        	selectionLevel = SELECTION_LEVEL_ENVS;
        	Save(false);
        	evaluationsSinceLastSave = 0;
        }
}

void ENVS::Mode_Simulate_Set_TAU(dWorldID world, dSpaceID space) {

	// called from M3 in client mode

	// Switch to The Approximate User (TAU) mode.

	//	if ( !Ready_For_TAU_Mode() )
	//		return;

	End_Current_Mode();

	simulateMode = MODE_SIMULATE_TAU;

	// In TAU mode, there are always two environments;
	// the user must indicate which of the two robots
	// she likes better.

	Environment_Copy();

	// The evolving robots are always yellow in TAU mode.

	for (int i=0;i<numberOfEnvs;i++)
		taskEnvironments[i]->Set_Color(1,1,0);

	// To start, highlight the first of the two controllers.
	selectionLevel = SELECTION_LEVEL_ENVIRONMENT;
	taskEnvironments[0]->Activate_All();
	taskEnvironments[1]->Deactivate_All();
	activeEnvironment = 0;

	// ...and create the two robots to compare.
	TAU_Send_Controllers_For_Evaluation(world,space);
}

void ENVS::Mode_View_Set_Back(void) {

	viewMode = MODE_VIEW_BACK;
	Viewpoint_Set(0,-2.792,0.79,90,-10.5,0);
}

void ENVS::Mode_View_Set_Side(void) {

	viewMode = MODE_VIEW_SIDE;
	Viewpoint_Set(-7.274,1.658,2.16,0,-6,0);
}

void ENVS::Mode_View_Set_Top(void) {

	viewMode = MODE_VIEW_TOP;
	Viewpoint_Set(0,2,7,90,-90,0);
}

void ENVS::MutationProbability_Decrease(void) {

	if ( optimizer )
		optimizer->MutationProbability_Decrease();
}

void ENVS::MutationProbability_Increase(void) {

	if ( optimizer )
		optimizer->MutationProbability_Increase();
}

bool ENVS::Pair_Available(void) {

	// called from M3 in CLIENT mode
	return client->pairFileExists();
}

void ENVS::Prepare_To_Run_Without_Graphics(dWorldID world, dSpaceID space) {

	selectionLevel=SELECTION_LEVEL_ENVS;
	Load(false);
	Mode_Simulate_Set_Evolve(world,space);
	Reset();
	Mode_Simulate_Set_Evolve(world,space);
}

void ENVS::Rescore_Population(void) {

	for (int i=0; i<AFPO_POP_SIZE; i++) {
		if (	optimizer	&&
			optimizer->genomes[i] &&
			optimizer->genomes[i]->sensorTimeSeries )
			optimizer->genomes[i]->Score_Set( taus->score(optimizer->genomes[i]) );
	}
}

void ENVS::Reset(void) {

	if ( In_Evolution_Mode() ) {
		if ( optimizer )
			optimizer->Reset();
		if ( taus ) {
			delete taus;
			taus = NULL;
		}
	}

	else if ( In_TAU_Mode() ) {
		if ( taus ) {
			delete taus;
			taus = NULL;
			optimizer->Reset_Genomes();
		}
	}

	Mode_Simulate_Set_Design();
}

void ENVS::Selection_Level_Lower(void) {

	// Already at lowest level...
	if ( selectionLevel==SELECTION_LEVEL_ROBOT )
		return;

	if ( selectionLevel==SELECTION_LEVEL_OBJECT ) {
		taskEnvironments[activeEnvironment]->Deactivate_All();
		taskEnvironments[activeEnvironment]->Unmark_All();
		taskEnvironments[activeEnvironment]->Unhide_All();
		taskEnvironments[activeEnvironment]->Activate_Component(0);
		selectionLevel = SELECTION_LEVEL_ROBOT;
	}
	else if ( selectionLevel==SELECTION_LEVEL_ENVIRONMENT ) {
		taskEnvironments[activeEnvironment]->Deactivate_All();
		taskEnvironments[activeEnvironment]->Unmark_All();
		taskEnvironments[activeEnvironment]->Activate_Light_Source();
		selectionLevel = SELECTION_LEVEL_OBJECT;
	}
	else {  // selection level is envs
		Deactivate_All();
		if ( activeEnvironment<0 )
			activeEnvironment=0;
		taskEnvironments[activeEnvironment]->Activate_All();
		selectionLevel = SELECTION_LEVEL_ENVIRONMENT;
	}
}

void ENVS::Selection_Level_Raise(void) {

	// Only allow selection level change in design mode.
	if ( !In_Design_Mode() )
		return;

	// Already at highest level...
	if ( selectionLevel==SELECTION_LEVEL_ENVS )
		return;
	else if ( selectionLevel==SELECTION_LEVEL_ENVIRONMENT ) {
//	TBD: "environment" is highest level for users for now
//		return;
		taskEnvironments[activeEnvironment]->Deactivate_All();
		taskEnvironments[activeEnvironment]->Unmark_All();
		Activate_All();
		selectionLevel=SELECTION_LEVEL_ENVS;
	}
	else if ( selectionLevel==SELECTION_LEVEL_OBJECT ) {
		taskEnvironments[activeEnvironment]->Unmark_All();
		taskEnvironments[activeEnvironment]->Activate_All();
		selectionLevel=SELECTION_LEVEL_ENVIRONMENT;
		taskEnvironments[activeEnvironment]->Robots_Recolor('r');
	}
	else { // selectionLevel==SELECTION_LEVEL_ROBOT
		taskEnvironments[activeEnvironment]->Unmark_All();
		taskEnvironments[activeEnvironment]->Hide_Robot_Joints();
		taskEnvironments[activeEnvironment]->Activate_Robot(0);
		selectionLevel=SELECTION_LEVEL_OBJECT;
		taskEnvironments[activeEnvironment]->Robots_Recolor('r');
	}
}

void ENVS::Show_Champ( dWorldID world, dSpaceID space) {

	// If there is a champ being evaluated, and its time
	// is not yet up, allow it to move.
	if ( !Eval_Finished() ) {
		for (int i=0;	i<currNumberOfEnvs;	i++)
			taskEnvironments[i]->Allow_Robot_To_Move(optimizer->timer);
		optimizer->Timer_Update();
	}
	// If the time limit for the champ has
	// expired, replay it.
	else {
		optimizer->Print();
		Destroy_Simulated_Objects();
		optimizer->Timer_Reset();
		Create_Robot_Current_Best(world,space);
	}
}

void ENVS::Speed_Decrease(void) {

	if ( speed > 1 )
		speed--;
}

void ENVS::Speed_Increase(void) {

	if ( speed < 1000 )
		speed++;
}

/*
NEURAL_NETWORK *ENVS::TAU_Get_User_Favorite(void) {

	if ( !taus )
		return( NULL );
	return( tau->Controller_Get_Best() );
}*/

int ENVS::TAU_Ready_To_Predict(void) {

	if ( !taus )
		return( false );
	return( taus->readyToPredict() );
}

double ENVS::TAU_Score_Get(void) {

	// called from Evolve()

	// Get the current controller that was just evaluated,
//	NEURAL_NETWORK *currentController = taskEnvironments[0]->robots[0]->neuralNetwork;

	// and return the predicted score of that controller.
//	return( taus->score(currentController) );
	return taus->score(controllerUnderEvaluation);
}

void ENVS::TAU_Show_Robot_Pair( dWorldID world, dSpaceID space ) {

/*	if ( tau->timer<1000 ) {

		taskEnvironments[0]->Allow_Robot_To_Move(tau->timer);
		taskEnvironments[1]->Allow_Robot_To_Move(tau->timer);
	 	tau->timer++;
	//	tau->Optimize();
	}

	// If the time limit for the robot pair has expired, replay it.
	else {

		// mmm Modification to allow for automated preferences.
		// TAU_User_Has_Indicated_A_Preference(world,space);

		// mmm Modification to allow for automated preferences.
		//TAU_Store_Sensor_Data();

		Destroy_Simulated_Objects();

		tau->timer=0;

		TAU_Send_Controllers_For_Evaluation(world, space);
	}*/
}

void ENVS::TAU_User_Has_Indicated_A_Preference( dWorldID world, dSpaceID space ) {

	// called from M3 upon pressing the spacebar in CLIENT mode

/*	// Only accept a user's preference if in TAU mode.
	if ( In_TAU_Mode() ) {

		TAU_Store_User_Preference(); // into a file
		delete tau;
		tau = NULL;

		Destroy_Simulated_Objects();

		Mode_Simulate_Set_Design();

		client->deletePairFile();
		client->checkIfFirstIterationAndMakeRecord();
	}*/
}

void ENVS::Video_Record(void) {

/*
	if ( timeStepsSinceLastFrameCapture < TIME_STEPS_PER_FRAME ) {
		timeStepsSinceLastFrameCapture++;
		return;
	}

	printf("Recording frame %d in Movie%d.\n",frameIndex,movieIndex);

	int width = 352*2;
	int height = 288*2;

	char s[100];

	if ( frameIndex<10 )
		sprintf (s,"Movie%d/0000%d.ppm",movieIndex,frameIndex);
	else if ( frameIndex<100 )
		sprintf (s,"Movie%d/000%d.ppm",movieIndex,frameIndex);
	else if ( frameIndex<1000 )
		sprintf (s,"Movie%d/00%d.ppm",movieIndex,frameIndex);
	else if ( frameIndex<10000 )
		sprintf (s,"Movie%d/0%d.ppm",movieIndex,frameIndex);
	else if ( frameIndex<100000 )
		sprintf (s,"Movie%d/%d.ppm",movieIndex,frameIndex);

	FILE *f = fopen (s,"wb");
	fprintf (f,"P6\n%d %d\n255\n",width,height);

	void *buf = malloc( width * height * 3 );
	glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buf );

	for (int y=(height - 1); y>=0; y--) {
		for (int x=0; x<width; x++) {
			unsigned char *pixel = ((unsigned char *)buf)+((y*width+ x)*3);
			unsigned char b[3];
			b[0] = *pixel;
			b[1] = *(pixel+1);
			b[2] = *(pixel+2);
			fwrite(b,3,1,f);
		}
	}
	free(buf);
	fclose(f);

	frameIndex++;
	timeStepsSinceLastFrameCapture = 0;
*/
}

void ENVS::Video_Start_Stop(void) {

	if ( recordingVideo )
		Video_Stop();
	else
		Video_Start();
}

void ENVS::Viewpoint_Get(void) {

	dsGetViewpoint(xyz,hpr);

	printf("%3.3f %3.3f %3.3f %3.3f %3.3f %3.3f\n",
		xyz[0],xyz[1],xyz[2],hpr[0],hpr[1],hpr[2]);
}

// ---------------- Private methods -------------------

void ENVS::Activate_All(void) {

	for (int i=0;	i<numberOfEnvs;	i++)
		if ( taskEnvironments[i] )
			taskEnvironments[i]->Activate_All();
}

void ENVS::Camera_Position_Load(ifstream *inFile, int showGraphics) {

	(*inFile) >> xyz[0] >> xyz[1] >> xyz[2];
	(*inFile) >> hpr[0] >> hpr[1] >> hpr[2];
	if ( showGraphics )
		dsSetViewpoint(xyz,hpr);
}

void ENVS::Camera_Position_Save(ofstream *outFile, int showGraphics) {

	if ( showGraphics )
		dsGetViewpoint(xyz,hpr);
	(*outFile) << xyz[0] << " " << xyz[1] << " " << xyz[2] << "\n";
	(*outFile) << hpr[0] << " " << hpr[1] << " " << hpr[2] << "\n";
}

int ENVS::Check_For_Pref(void) {

	// called from Evolve() in SERVER mode

	int noOfPrefsFound = server->updatePreferences();

	printf("Feeding the obtained preferences to TAU:\n");
	for( int i=0; i < server->curNoOfClients; i++ ) { // for all client processes which updatePreferences() managed to find store prefs in tau
		if( server->clientList[i] != server->curPrefTable[i][0] ) printf("WARNING! There used to be off by 1 error here.\n");
		taus->storePref( server->curPrefTable[i][0],
										server->curPrefTable[i][1], // ID of the first controller as read by server->updatePreferences()
										server->curPrefTable[i][2], // ID of the second controller
										server->curPrefTable[i][3] ); // user preference
		printf("%d\t%d\t%d\t%d\n", server->curPrefTable[i][0], server->curPrefTable[i][1], server->curPrefTable[i][2], server->curPrefTable[i][3] );
//		Rescore_Population();
	}

	return noOfPrefsFound;
}

void ENVS::Check_Whether_To_End(void) {

	clock_t currTime = clock();
	double CPUSecondsUsed = ((double) (currTime - startTime)) / CLOCKS_PER_SEC;
	double CPUMinutesUsed = CPUSecondsUsed/60.0;
	double CPUHoursUsed = CPUMinutesUsed/60.0;

	// End run after 9 hours of CPU time
//	if ( CPUSecondsUsed > 60 )
	if ( CPUHoursUsed >= 9.0 )
		exit(0);
}

void ENVS::Check_Whether_To_Writeout(void) {

	clock_t currTime = clock();
	double CPUSecondsSinceLastWriteout = ((double) (currTime - timeSinceLastWriteout)) / CLOCKS_PER_SEC;
	double CPUMinutesSinceLastWriteout = CPUSecondsSinceLastWriteout/60.0;

	// Write an update every 5 minutes.
	if ( CPUSecondsSinceLastWriteout > 5 ) {
//	if ( CPUMinutesSinceLastWriteout > 1 ) {
			char fileName[200];
			sprintf(fileName,"SavedFiles/writeout%d.txt",randSeed);

		ofstream *outFile = new ofstream(fileName,ios::app);

		// Write out elapsed hours...
		(*outFile) << double((clock()-startTime)/CLOCKS_PER_SEC)/(60.0*60.0) << "\t";
		// Write out elapsed minutes...
		(*outFile) << double((clock()-startTime)/CLOCKS_PER_SEC)/60.0 << "\t";
		// Write out elapsed seconds...
		(*outFile) << double((clock()-startTime)/CLOCKS_PER_SEC) << "\t";

		optimizer->Genome_Get_Best()->Writeout(outFile);

		(*outFile) << "\n";

		outFile->close();
		delete outFile;
		outFile = NULL;

		timeSinceLastWriteout = currTime;
	}
}

void ENVS::Create_Robot_Current_Best( dWorldID world, dSpaceID space ) {

	currNumberOfEnvs = numberOfEnvs;
	NEURAL_NETWORK *bestGenome = optimizer->Genome_Get_Best();
	for (int i=0;	i<currNumberOfEnvs;	i++) {
		taskEnvironments[i]->Prepare_For_Simulation(world,space);
		taskEnvironments[i]->Label(bestGenome,i);
	}
	bestGenome = NULL;
}

void ENVS::Create_Robot_To_Evaluate( dWorldID world, dSpaceID space ) {

	NEURAL_NETWORK *nextGenome = optimizer->Genome_Get_Next_To_Evaluate();
	for (int i=0;	i<currNumberOfEnvs;	i++) {
		taskEnvironments[i]->Prepare_For_Simulation(world,space);
		taskEnvironments[i]->Label(nextGenome,i);
		taskEnvironments[i]->Record_Sensor_Data(optimizer->evaluationPeriod);
	}

	controllerUnderEvaluation = nextGenome;

	nextGenome = NULL;
}

void ENVS::Deactivate_All(void) {

	for (int i=0;	i<numberOfEnvs;	i++)
		if ( taskEnvironments[i] )
			taskEnvironments[i]->Deactivate_All();
}

void ENVS::Destroy_Simulated_Objects(void) {

	for (int i=0;i<currNumberOfEnvs;i++)
		taskEnvironments[i]->Destroy_Simulated_Objects();

	controllerUnderEvaluation = NULL;
}

int ENVS::Directory_Found(char *dirName) {

	struct stat stFileInfo;
	bool blnReturn;
	int intStat;

	intStat = stat(dirName,&stFileInfo);
	if(intStat == 0) {
		blnReturn = true;
	} else {
		blnReturn = false;
	}

	return(blnReturn);
}

void ENVS::Directory_Make(char *dirName) {

        char command[500];

        sprintf(command,"mkdir %s",dirName);
        system(command);
}

void ENVS::End_Current_Mode(void) {

        // In some modes, the objects are physically simulated.
        // Switch them back to only being drawn.

        Destroy_Simulated_Objects();

        // If robot behavior was being simulated,
        // reset the timer in the optimizer.

        if ( optimizer )
                optimizer->Timer_Reset();

        // If in TAU mode, two environments were simulated,
        // so delete the second one.

        if ( In_TAU_Mode() )
                Environment_Delete();
}

int  ENVS::End_State_Missing(void) {

	int missing = false;
	int currentEnvironment = 0;

	while ( (currentEnvironment<numberOfEnvs) && (!missing) ) {

		if ( taskEnvironments[currentEnvironment]->numRobots<2 )
			missing = true;
		else
			currentEnvironment++;
	}

	return( missing );
}

void ENVS::Environment_Copy(void) {

	// Maximum number of environments already exist.
	if ( numberOfEnvs == MAX_ENVIRONMENTS )
		return;

	taskEnvironments[numberOfEnvs] =
		new ENVIRONMENT(taskEnvironments[activeEnvironment]);

	taskEnvironments[activeEnvironment]->Deactivate_All();

//	taskEnvironments[numberOfEnvs]->Move(-MOVE_INCREMENT,MOVE_INCREMENT,0);
	taskEnvironments[numberOfEnvs]->Activate_All();

	activeEnvironment = numberOfEnvs;

	numberOfEnvs++;
	currNumberOfEnvs++;
}

void ENVS::Environment_Delete(void) {

	// Cannot remove only remaining environment.
	if ( numberOfEnvs == 1 )
		return;

	delete taskEnvironments[numberOfEnvs-1];
	taskEnvironments[numberOfEnvs-1] = NULL;

	numberOfEnvs--;
	currNumberOfEnvs--;

	activeEnvironment = numberOfEnvs-1;

	taskEnvironments[numberOfEnvs-1]->Activate_All();
}

void ENVS::Environment_Move(double x, double y, double z) {

	taskEnvironments[activeEnvironment]->Move(x,y,z);
}

void ENVS::Environment_Next(void) {

	taskEnvironments[activeEnvironment]->Deactivate_All();

	activeEnvironment++;

	if ( activeEnvironment==numberOfEnvs )
		activeEnvironment = 0;

	taskEnvironments[activeEnvironment]->Activate_All();
}

void ENVS::Environment_Previous(void) {

	taskEnvironments[activeEnvironment]->Deactivate_All();

	activeEnvironment--;

	if ( activeEnvironment==-1 )
		activeEnvironment = numberOfEnvs-1;

	taskEnvironments[activeEnvironment]->Activate_All();

}

int  ENVS::Eval_Finished(void) {

	return( optimizer->Time_Elapsed() );
}

bool ENVS::File_Exists(char *fileName) {

	ifstream ifile(fileName);
	return ifile;
}

int ENVS::File_Index_Next_Available(void) {

	int fileIndex = 0;
	char fileName[100];
	sprintf(fileName,"SavedFiles/envs%d.dat",fileIndex);
	while ( File_Exists(fileName) ) {
		fileIndex++;
		sprintf(fileName,"SavedFiles/envs%d.dat",fileIndex);
	}
	return( fileIndex );
}

double ENVS::Fitness_Get(void) {

	// Take average fitness.
	double fitness = taskEnvironments[0]->Fitness_Get();
	for (int i=1;	i<currNumberOfEnvs;	i++)
		fitness = fitness + taskEnvironments[i]->Fitness_Get();
	fitness = fitness / double(currNumberOfEnvs);

/*
	// Multiply fitnesses.
	double fitness = fabs(taskEnvironments[0]->Fitness_Get());
	for (int i=1;	i<numberOfEnvs;	i++)
		fitness = fitness * fabs(taskEnvironments[i]->Fitness_Get());
	fitness = -fitness;
*/

/*
	// Take worst fitness.
	double fitness = +1000.0;
	for (int i=0;	i<numberOfEnvs;	i++) {
		double temp = taskEnvironments[i]->Fitness_Get();
		if ( temp < fitness )
			fitness = temp;
	}
*/

	return( fitness );
}

void ENVS::Load(int showGraphics) {

	SavedFile_FindNext();

	char fileName[100];
	sprintf(fileName,"SavedFiles/envs%d.dat",savedFileIndex);
	printf("%s\n",fileName);

	ifstream *inFile = new ifstream(fileName);

	if ( !inFile->is_open() ) {

		printf("Can't open envs file %d to load.\n",savedFileIndex);
	}
	else {

		Camera_Position_Load(inFile,showGraphics);

		Load_Environments(inFile);

		Load_Optimizer(inFile);

		Load_TAU(inFile);

		printf("envs %d loaded.\n",savedFileIndex);
	}

	inFile->close();
	delete inFile;
	inFile = NULL;
}

void ENVS::Load_Pair(void) {

	// called from M3 in CLIENT mode
	char fileName[100];
	client->pairFileName(fileName);
	ifstream *inFile = new ifstream(fileName);

	Camera_Position_Load(inFile,true);
	Load_Environments(inFile);
	TAU_Load_Controller_Pair(inFile);

	inFile->close();
	delete inFile;
	inFile = NULL;
}

void ENVS::Load_Environments(ifstream *inFile) {

	for (int i=0;i<numberOfEnvs;i++) {
		delete taskEnvironments[i];
		taskEnvironments[i] = NULL;
	}
	(*inFile) >> numberOfEnvs;
	for (int i=0;	i<numberOfEnvs;	i++)
		taskEnvironments[i] = new ENVIRONMENT(inFile);
}

void ENVS::Load_Optimizer(ifstream *inFile) {

	int isOptimizer;
	(*inFile) >> isOptimizer;

	if ( optimizer )
		delete optimizer;

	if ( isOptimizer )
		optimizer = new OPTIMIZER(inFile);
	else
		optimizer = NULL;
}

void ENVS::Load_TAU(ifstream *inFile) {

        int isTAU;
        (*inFile) >> isTAU;

/*        if ( taus )
                delete taus;

        if ( isTAU )
                taus = new TAUS(inFile);
        else
                taus = NULL;*/
}

void ENVS::Move(double x, double y, double z) {

	for (int i=0;i<numberOfEnvs;i++)
		if ( taskEnvironments[i] )
			taskEnvironments[i]->Move(x,y,z);
}

int ENVS::No_Robots_Above_The_Barrier(void) {

        if ( !optimizer )
                return( true );

				int noRobotsAboveTheBarrier = true;
        for (int i=0; i<AFPO_POP_SIZE; i++) {
                if ( optimizer->Genome_Get(i)->sensorTimeSeries ) {
                        if ( optimizer->Genome_Get(i)->sensorTimeSeries->Get(STARTING_EVALUATION_TIME-1,12) > 3.0 )
                                noRobotsAboveTheBarrier = false;
                }
        }
        return( noRobotsAboveTheBarrier );
}

void ENVS::Optimizer_Initialize(void) {

	if ( !optimizer ) {
		int numberOfSensors = taskEnvironments[0]->robots[0]->Sensors_Number_Of();
		int numberOfMotors = taskEnvironments[0]->robots[0]->Motors_Number_Of();

		// The genomes encode a synaptic weight from each
		// sensor to each hidden neuron, and from each
		// hidden neuron to each motor
		// of size = (inputs + outputs ) x hidden neurons
		optimizer = new OPTIMIZER(numberOfSensors, numberOfMotors );
	}
}

int ENVS::Optimizer_Robot_Mismatch(void) {

	if ( !optimizer )
		return (1);

	return ( optimizer->numSensors !=
	     	taskEnvironments[0]->robots[0]->Sensors_Number_Of()
	     || optimizer->numMotors !=
		 taskEnvironments[0]->robots[0]->Motors_Number_Of() );
}

int  ENVS::Ready_For_Champ_Mode(void) {

        // Already in champ mode.
        if ( In_Champ_Mode() )
                return( false );


	// Can't play back the best controller
	// if there aren't any.
	if ( !optimizer )
		return( false );

	if ( optimizer->Genome_Get_Best() == NULL )
		return( false );

  return( true );
}

int  ENVS::Ready_For_Design_Mode(void) {

        // Already in design mode.
        if ( In_Design_Mode() )
                return( false );

	return( true );
}

int  ENVS::Ready_For_Evolution_Mode(void) {

        // Already in evolve mode.
        if ( In_Evolution_Mode() )
                return( false );

        // No task for the robot to perform has yet been set.
        if ( End_State_Missing() ) {
                printf("No target state for the robot has been defined.\n");
                return( false );
        }

        // A robot isn't evolvable due to a construction problem
        for (int i=0; i<numberOfEnvs; i++) {
                if ( taskEnvironments[i]->Robot_Joint_Is_Unattached() ) {
                        printf("At least one robot joint is unconnected.\n");
                        while ( selectionLevel != SELECTION_LEVEL_ROBOT )
                                Selection_Level_Lower();
                        Viewpoint_Set(0,2.5,20,90,-90,0);
                        return( false );
                }
        }

        return( true );
}

int ENVS::Ready_For_TAU_Mode(void) {

	// Already in TAU mode.
	if ( In_TAU_Mode() )
		return( false );

	// Only allow TAU if there is one task environment.
	if ( numberOfEnvs != 1 )
		return( false );

	// Only allow TAU if there are at least two controllers
	// available for the user to choose between.
	if ( !optimizer )
		return( false );

	if ( optimizer->Genomes_Num_Of_Evaluated() < 2 )
		return( false );

	return( true );
}

void ENVS::Reset(dWorldID world, dSpaceID space, dJointGroupID contactgroup) {

	Destroy_Simulated_Objects();

	dJointGroupEmpty(contactgroup);

	optimizer->Genome_Discard_Being_Evaluated();

	Create_Robot_To_Evaluate(world, space);
}

int ENVS::Robot_Being_Evaluated(void) {

	int allAreBeingEvaluated = true;

	int currentEnvironment = 0;

	while (	(allAreBeingEvaluated) &&
		(currentEnvironment<currNumberOfEnvs) )

		if ( taskEnvironments[currentEnvironment]->robots[0]->In_Simulator() )

			currentEnvironment++;
		else
			allAreBeingEvaluated = false;

	return( allAreBeingEvaluated );
}

void ENVS::Save(int showGraphics) {

//	int fileIndex = File_Index_Next_Available();
	int fileIndex = 0;

	char fileName[100];
	sprintf(fileName,"SavedFiles/envs%d.dat",fileIndex);

	ofstream *outFile = new ofstream(fileName);

	Camera_Position_Save(outFile,showGraphics);

	Save_Environments(outFile);

//	char envsindex[128];
//	sprintf(envsindex,"envs.dat index: %d\n",fileIndex);
//	Save_Fitness(envsindex);

	Save_Optimizer(outFile);

	Save_TAU(outFile);

	outFile->close();
	delete outFile;
	outFile = NULL;

//	printf("Envs %d saved.\n",fileIndex);
}

void ENVS::Save_Pair_For_Pref(int pid, char* fileName) {

	ofstream *outFile = new ofstream(fileName);

	Camera_Position_Save(outFile,false);
	Save_Environments(outFile);
	TAU_Save_Controller_Pair(pid, outFile); // calls taus->controllersSavePair(pid, outFile)

	outFile->close();
	delete outFile;
	outFile = NULL;
}

void ENVS::Save_All_Pairs_For_Pref(void) {

	// called from Evolve() in SERVER mode
	char fileName[100];
	int pid;

	if( server->firstIteration ) { // if we are at the beginning of the service
		if( server->pairFileNameByPID(fileName, 0) == 0 ) { // read common pair file name and check if file exists
			printf("Old common pair file found - another server is possibly running.\nExiting.\n"); // if yes, get grumpy and exit
			exit(1);
		}
		Save_Pair_For_Pref(0, fileName); // otherwise, create a common pair file
	}
	else { // if we are not in the beginning of the service
		for(int i=0; i < server->noOfClients; i++) { // go through the client list
			pid = server->clientList[i];
			if( server->pairFileNameByPID(fileName, pid) != 0 ) // if pair files do not exist
				Save_Pair_For_Pref(pid, fileName); // create them
		}
	}

	server->checkIfFirstIterationAndMakeRecord();
}

void ENVS::Save_Environments(ofstream *outFile) {

	(*outFile) << numberOfEnvs << "\n";

	for (int i=0;	i<numberOfEnvs;	i++)

		if ( taskEnvironments[i] )

			taskEnvironments[i]->Save(outFile);
}

void ENVS::Save_Fitness(string str) {

	char fileName[100];
	sprintf(fileName,"SavedFiles/fitness.dat");

	ofstream fitFile;
	fitFile.open (fileName, ios::app);

	if ( str == "" )  {
		char fit[127];
		sprintf(fit,"%4.5f",Fitness_Get());
		fitFile << fit << "\n";
	} else {
		fitFile << str;
	}

	fitFile.close();

	printf("Fitness saved.\n");
}

void ENVS::Save_Optimizer(ofstream *outFile) {

	if ( optimizer ) {
		(*outFile) << "1\n";
		optimizer->Save(outFile);
	}
	else
		(*outFile) << "0\n";
}

void ENVS::Save_TAU(ofstream *outFile) {

/*        if ( taus ) {
                (*outFile) << "1\n";
                taus->save(outFile);
        }
        else*/
                (*outFile) << "0\n";
}

void ENVS::SavedFile_FindNext(void) {

	savedFileIndex++;

	char fileName[100];
	sprintf(fileName,"SavedFiles/envs%d.dat",savedFileIndex);

	if ( !File_Exists(fileName) )

		savedFileIndex = 0;
}

void ENVS::Sensor_Data_Receive(void) {

	MATRIX *timeSeries = taskEnvironments[0]->Get_Sensor_Data();

	if ( optimizer )

		optimizer->Sensor_Data_Receive(timeSeries);
}

double ENVS::Sensor_Sum(void) {

	double sum = 0.0;
	for (int i=0;	i<numberOfEnvs;	i++)
		sum = sum + taskEnvironments[i]->Sensor_Sum();
	return( sum );
}

int  ENVS::Target_Sensor_Values_Recorded(void) {

	return( targetSensorValuesRecorded );
}

/*
void ENVS::TAU_Get_Controllers_From_Optimizer(void) {

	if ( !tau )
		tau = new TAU;
	tau->Controllers_Select_From_Optimizer(optimizer);
}*/

void ENVS::TAU_Load_Controller_Pair(ifstream *inFile) {

/*	if ( !tau )
		tau = new TAU;
	tau->Controllers_Load_Pair(inFile);*/
}

void ENVS::TAU_Reset_User_Models(void) {

	if ( optimizer )
		optimizer->Scores_Reset();
//	if ( taus )
//		taus->userModelsReset();
}

void ENVS::TAU_Save_Controller_Pair(int pid, ofstream *outFile) {

	if ( !taus )
		taus = new TAUS;
	taus->controllersSavePair(pid, optimizer, outFile);
}

void ENVS::TAU_Send_Controllers_For_Evaluation(dWorldID world, dSpaceID space) {

/*	taskEnvironments[0]->Prepare_For_Simulation(world,space);
	taskEnvironments[0]->Label(tau->Controller_Pair_Get_First(),0);
	taskEnvironments[0]->Record_Sensor_Data(STARTING_EVALUATION_TIME); // into RAM matrix

	taskEnvironments[1]->Prepare_For_Simulation(world,space);
	taskEnvironments[1]->Label(tau->Controller_Pair_Get_Second(),1);
	taskEnvironments[1]->Record_Sensor_Data(STARTING_EVALUATION_TIME);

	// There are contact resolution errors if the non-robot objects in both
	// environments are created. So, remove the non-robot objects from
	// the second environment.
	taskEnvironments[1]->Make_NonRobotObjects_Incorporeal();*/
}

void ENVS::TAU_Store_Sensor_Data(void) {

/*        // Store the sensor time series data generated by the
        // two controllers.

        MATRIX *timeSeries = taskEnvironments[0]->Get_Sensor_Data();

        tau->Controller_First_Store_Sensor_Data(timeSeries);

        timeSeries = NULL;

        timeSeries = taskEnvironments[1]->Get_Sensor_Data();

        tau->Controller_Second_Store_Sensor_Data(timeSeries);

        timeSeries = NULL;*/
}

void ENVS::TAU_Store_User_Preference(void) {

/*	char fileName[100];
	client->prefFileName(fileName);

	ofstream *outFile = new ofstream(fileName);

	(*outFile) << tau->controllers[0]->ID << " ";
	(*outFile) << tau->controllers[1]->ID << " "; // !!!!

  // mmm Get an automated preference, rather than one from the human user.
	//
  //      double prefFirst  = taskEnvironments[0]->robots[0]->Preference_Get(taskEnvironments[0]->robots[1],2);
  //     double prefSecond = taskEnvironments[1]->robots[0]->Preference_Get(taskEnvironments[0]->robots[1],2);
	//
  //      if ( prefFirst > prefSecond )
  //      	(*outFile) << "0";
  //      else
  //      	(*outFile) << "1";
	//
	// mmm
	(*outFile) << activeEnvironment;

	outFile->close();
	delete outFile;
	outFile = NULL;
*/
}

void ENVS::Video_Start(void) {

	movieIndex = 0;

	char dirName[100];

	sprintf(dirName,"Movie%d",movieIndex);

	while ( Directory_Found(dirName) ) {
		movieIndex++;
		sprintf(dirName,"Movie%d",movieIndex);
	}

	Directory_Make(dirName);

	frameIndex=1;
	timeStepsSinceLastFrameCapture = 0;

	recordingVideo = true;
}

void ENVS::Video_Stop(void) {

	char command[500];

	char fileName[500];
	sprintf(fileName,"Movie%d.bat",movieIndex);

	ofstream *outFile = new ofstream(fileName);

	(*outFile) << "cd Movie"<<movieIndex<<" \n";

	(*outFile) << "for f in *ppm ; do nice -n 20 convert -quality 100 $f `basename $f ppm`jpg; done \n";

	(*outFile) << "rm *.ppm \n";

	(*outFile) << "mencoder 'mf://*.jpg' -mf fps=60 -o Movie"<<movieIndex<<".avi -ovc lavc \n";

// TBD: leave the series of jpegs, so different frame rates can be run
//	(*outFile) << "rm *.jpg \n";

	outFile->close();
	delete outFile;
	outFile = NULL;

	sprintf(command,"chmod 777 Movie%d.bat",movieIndex);
	system(command);

	sprintf(command,"./Movie%d.bat &",movieIndex);
	system(command);

	recordingVideo = false;
}

void ENVS::Viewpoint_Set(	double x, double y, double z,
				double h, double p, double r) {

	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;

	hpr[0] = h;
	hpr[1] = p;
	hpr[2] = r;

	dsSetViewpoint(xyz,hpr);
}
#endif
