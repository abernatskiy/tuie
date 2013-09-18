#ifndef _CLIENT_CPP
#define _CLIENT_CPP

#include "client.h"

CLIENT::CLIENT(void) {

	firstIteration = true;
}

int CLIENT::getPID(void) {

	return getpid();
}

bool CLIENT::fileExists(char* fname) {

	if( access( fname, F_OK ) != -1 )
		return true;
	return false;
}

void CLIENT::checkIfFirstIterationAndMakeRecord(void) {

	if( firstIteration )
		firstIteration = false;
}

bool CLIENT::pairFileExists(void) {

	sprintf(tmpFileName, "SavedFiles/pair0.dat");
	return fileExists(tmpFileName);
}

bool CLIENT::prefFileExists(void) {

	sprintf(tmpFileName, "SavedFiles/pref0.dat");
	return fileExists(tmpFileName);
}

short CLIENT::pairFileName(char* strspace) {

	sprintf(strspace, "SavedFiles/pair0.dat");
	if( fileExists(strspace) )
		return 0;

/*	if( firstIteration )
		if( fileExists("SavedFiles/pair0.dat") ) {
			sprintf(strspace, "SavedFiles/pair0.dat");
			return 0;
		}
		else
			return 1;

	sprintf(tmpFileName, "SavedFiles/pair%d.dat", getPID());
	if( fileExists(tmpFileName) )
	{
		sprintf(strspace, "SavedFiles/pair%d.dat", getPID());
		return 0;
	}
*/
	return 1;
}

short CLIENT::prefFileName(char* strspace) {

//	sprintf(strspace, "SavedFiles/pref%d.dat", getpid());
	sprintf(strspace, "SavedFiles/pref0.dat");
	if( fileExists(strspace) )
		return 0;
	return 1;
}

#endif
