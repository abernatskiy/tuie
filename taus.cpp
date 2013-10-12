#ifndef _TAUS_CPP
#define _TAUS_CPP

#include "taus.h"

TAUS::TAUS(void) {

	tau = new TAU[3];
}

TAUS::TAUS(ifstream* inFile) {

	tau = new TAU[3];
	//
}

TAUS::~TAUS(void) {

	delete[] tau;
}

void TAUS::save(ofstream* outFile) {

	//
}

double TAUS::score(NEURAL_NETWORK* genome) {

	//
	return 0;
}

bool TAUS::readyToPredict(void) {

	//
	return false;
}

void TAUS::storePref(int pid, int firstID, int secondID, int pref) {

	//
}

void TAUS::controllersSavePair(int pid, OPTIMIZER* optimizer, ofstream* outFile) {

	//
}

#endif
