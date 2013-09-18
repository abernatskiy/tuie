#include <unistd.h>
#include <stdio.h>

#ifndef _CLIENT_H
#define _CLIENT_H

class CLIENT {

private:
	char tmpFileName[100];
	bool firstIteration;
	int getPID(void);
	bool fileExists(char*);

public:
	CLIENT(void);
	~CLIENT(void);
	void checkIfFirstIterationAndMakeRecord(void);
	bool pairFileExists(void);
	bool prefFileExists(void);
	short pairFileName(char* strspace); // returns 0 if pair file is found, nonzero otherwise
	short prefFileName(char* strspace);
};

#endif
