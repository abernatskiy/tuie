#ifndef _INTERACTOR_CPP
#define _INTERACTOR_CPP

#include "interactor.h"

/******* INTERACTOR implementaion *******/

/* protected */

bool INTERACTOR::fileExists(char* fname) {

	if( access( fname, F_OK ) != -1 )
		return true;
	return false;
}

void INTERACTOR::deleteFile(char* fname) {

	char command[200];
	sprintf(command, "rm %s", fname);
	system(command);
}

/* public */

INTERACTOR::INTERACTOR(void) {

	firstIteration = true;
}

void INTERACTOR::checkIfFirstIterationAndMakeRecord(void) {

	if( firstIteration )
		firstIteration = false;
}

#endif
