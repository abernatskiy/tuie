#ifndef _SERVER_CPP
#define _SERVER_CPP

#include "interactor.h"

int* SERVER::prefTableByPID(int pid) {

	int i=0;
	while( curPrefTable[i][0] != 0 ) {
		if( curPrefTable[i][0] == pid )
			return curPrefTable[i];
		i++;
	}
	return NULL;
}

int* SERVER::newPrefTableRecord(int pid) {

	int i=0;
	while( curPrefTable[i][0] != 0 ) i++;
	curPrefTable[i][0] = pid;
	curNoOfClients++;
	return curPrefTable[i];
}

bool SERVER::isClient(int pid) {

	for(int i=0; i<noOfClients; i++)
		if( clientList[i] == pid )
			return true;
	return false;
}

void SERVER::addClient(int pid) {

	clientList[noOfClients++] = pid;
}

void SERVER::resetPrefTable(void) {

	curNoOfClients = 0;
	for(int i=0; i<50; i++)
		for(int j=0; j<4; j++)
			curPrefTable[i][j] = 0;
}

SERVER::SERVER(void) {

	resetPrefTable();
	noOfClients = 0;
	for(int i=0; i<50; i++)
		clientList[i] = 0;
}

SERVER::~SERVER(void) {

	if( pairFileNameByPID(tmpFileName, 0) == 0 )
		deleteFile(tmpFileName);
}

short SERVER::pairFileNameByPID(char* strspace, int pid) {

	sprintf(strspace, "SavedFiles/pair%d.dat", pid);
	if( fileExists(strspace) )
		return 0;
	return 1;
}

void SERVER::updatePreferences(void) {

	printf("SERVER: Trying to update preferences\n");

	resetPrefTable();
	DIR* dir;
	struct dirent *ent;
	int prefFiles = 0;
	int curPID;
	int i=0;

	FILE* file;
	int* curRecord;

	if( (dir = opendir("SavedFiles")) != NULL ) {
		while ( (ent = readdir(dir)) != NULL ) {
			if( strstr(ent->d_name, "pref") == NULL )
				continue;

			sscanf(ent->d_name, "pref%d.dat", &curPID); // no dirname in ent->d_name string

			if( !isClient(curPID) )
				addClient(curPID);
			curRecord = prefTableByPID(curPID);
			if( curRecord == NULL ) curRecord = newPrefTableRecord(curPID);

			sprintf(tmpFileName, "SavedFiles/%s", ent->d_name);

			printf("SERVER: Trying to open %s\n", tmpFileName);

			if( file = fopen(tmpFileName, "r") )
				printf("SERVER: file successfully opened\n");
			else {
				printf("Something has gone very, very wrong\n");
				exit(1);
			}
			fscanf(file, "%d %d %d", curRecord+1, curRecord+2, curRecord+3);
			fclose(file);
			deleteFile(tmpFileName);

			printf("SERVER: file %s processed successfully.\n", tmpFileName);

			prefFiles++;
		}
	}
}

#endif
