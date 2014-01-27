#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "arrays.h"

#define UND_SCR (-1.0)

class RANK_REC {

// this class is a representation of a subject - a certain something

public:
	int id;
	double opinions[2];

	RANK_REC(RANK_REC* other);
	RANK_REC(int id, double opinion1, double opinion2);
	int src();
	void print();
	void printWithoutNewline();
	int* compare(RANK_REC* other); // Don't forget to delete the returned array!
};

class RANKING {

public:
	RANK_REC** list;
	int space;
	int size;

	RANKING();
	RANKING(int space_req);
	~RANKING();
	void insert(RANK_REC* newSubj, int pos);
	int* findConstraints(RANK_REC* newSubj);
	void autoInsert(RANK_REC* newSubj);
	void autoInsert(int id, double scr, int src);
	void merge(RANKING* other); // heuristic merge - does not guarantee conflict number minimization
	int conflicts();
	int ambiguities();
	int* ambiguousIDs(); // warning - returns a pointer to an array on heap, may cause memory leaks if the array is not deleted properly
	int* extendedAmbiguousIDs(); // warning - returns a pointer to an array on heap, may cause memory leaks if the array is not deleted properly
	void rescore();

	void printWithComparison();
	void print();
};
