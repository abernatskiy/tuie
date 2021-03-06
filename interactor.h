#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#ifndef _INTERACTOR_H
#define _INTERACTOR_H

class INTERACTOR { // interactor.cpp

protected:
	char tmpFileName[100];
	bool fileExists(char*);
	void deleteFile(char*);

public:
	bool firstIteration;

	INTERACTOR(void);
	void checkIfFirstIterationAndMakeRecord(void);
};

class CLIENT : public INTERACTOR { // client.cpp

//protected:

public:

	int getPID(void); // used to be protected, published for debug

	bool pairFileExists(void);
	bool prefFileExists(void);
	short pairFileName(char* strspace); // returns 0 if pair file is found, nonzero otherwise
	short prefFileName(char* strspace);
	void deletePairFile(void);
	void tempFileName(char* strspace);
	void deployPrefFile(void);
};

class SERVER : public INTERACTOR { // server.cpp

protected:
	int* prefTableByPID(int pid);
	int* newPrefTableRecord(int pid);
	bool isClient(int pid);
	void addClient(int pid);
	void resetPrefTable(void);

public:
	int curPrefTable[50][4];
	// up to 50 records of user preferences
	// fields: 0 - client's PID, 1 - ANN1's ID, 2 - ANN2's ID, 3 - preference
	int curNoOfClients;

	int clientList[50];
	int noOfClients;

	SERVER(void);
	~SERVER(void);
//	bool prefFileExistsByPID(int pid);
	short pairFileNameByPID(char* strspace, int pid);
	int updatePreferences(void);

	void tempFileName(char* strspace);
	void deployPairFile(char* filename);
};

#endif
