#ifndef _CLIENT_CPP
#define _CLIENT_CPP

#include "interactor.h"

int CLIENT::getPID(void) {

	return getpid();
}

bool CLIENT::pairFileExists(void) {

	if( pairFileName(tmpFileName) == 0 )
		return true;
	return false;
}

bool CLIENT::prefFileExists(void) {

	if( prefFileName(tmpFileName) == 0 )
		return true;
	return false;
}

short CLIENT::pairFileName(char* strspace) {

//	sprintf(strspace, "SavedFiles/pair0.dat");
//	if( fileExists(strspace) )
//		return 0;

	if( firstIteration ) {

		sprintf(strspace, "SavedFiles/pair0.dat");
		if( fileExists(strspace) )
			return 0;
		else
		{
//			printf(".");
//			exit(1);
			return 1;
		}
	}

	sprintf(tmpFileName, "SavedFiles/pair%d.dat", getPID());
	if( fileExists(tmpFileName) ) {

		sprintf(strspace, "SavedFiles/pair%d.dat", getPID());
		return 0;
	}

	return 1;
}

short CLIENT::prefFileName(char* strspace) {

	sprintf(strspace, "SavedFiles/pref%d.dat", getPID());
	if( fileExists(strspace) )
		return 0;
	return 1;
}

void CLIENT::deletePairFile(void) {

	if( firstIteration ) return;
	if( pairFileName(tmpFileName) == 0 && strstr(tmpFileName, "pair0.dat") == NULL )
		deleteFile(tmpFileName);
}

void CLIENT::tempFileName(char* strspace) {

	sprintf(strspace, "SavedFiles/tmp%d", getPID());
}

void CLIENT::deployPrefFile(void) {

	char fn[100], command[200];
	prefFileName(fn);
	tempFileName(tmpFileName);
	sprintf(command, "mv %s %s", tmpFileName, fn);
	system(command);
}

#endif
