#include "rankings.h"

RANK_REC::RANK_REC(int nid, double opinion0, double opinion1)
{
	if( (opinion0 != UND_SCR && (opinion0 < 0.0 || opinion0 > 1.0)) ||
			(opinion1 != UND_SCR && (opinion1 < 0.0 || opinion1 > 1.0)) )
	{
		printf("RANKINGS: id=%d, opinion0=%2.2f, opinion1=%2.2f\n", nid, opinion0, opinion1);
		printf("RANKINGS: these kinds of opinions will not be tolerated\n");
		exit(1);
	}
	id = nid;
	opinions[0] = opinion0;
	opinions[1] = opinion1;
}

RANK_REC::RANK_REC(RANK_REC* other)
{
	id = other->id;
	opinions[0] = other->opinions[0];
	opinions[1] = other->opinions[1];
}

int RANK_REC::src()
{
	// find out from which user the score came
	int src;
	if(opinions[0] == UND_SCR && opinions[1] != UND_SCR)
		src = 1;
	else if(opinions[1] == UND_SCR && opinions[0] != UND_SCR)
		src = 0;
	else
	{ // do not accept double or empty scores
		printf("RANKINGS: wrong subject record structure\n");
		exit(1);
	}
	return src;
}

void RANK_REC::print()
{
	if(opinions[0] == UND_SCR)
		printf("%d\tu\t%2.2f\n", id, opinions[1]);
	else if(opinions[1] == UND_SCR)
		printf("%d\t%2.2f\tu\n", id, opinions[0]);
	else
		printf("%d\t%2.2f\t%2.2f\n", id, opinions[0], opinions[1]);
}

void RANK_REC::printWithoutNewline()
{
	if(opinions[0] == UND_SCR)
		printf("%d\tu\t%2.2f", id, opinions[1]);
	else if(opinions[1] == UND_SCR)
		printf("%d\t%2.2f\tu", id, opinions[0]);
	else
		printf("%d\t%2.2f\t%2.2f", id, opinions[0], opinions[1]);
}

int* RANK_REC::compare(RANK_REC* other)
{
	// return value is a pointer to an array of two integers
	// first integer is nonzero when a meaningful relationship between subjects exists
	// second integer is encoded differently depending on the value of the first integer:
	// 0 - 0 if relationship is unknown, 1 if relationship is meaningless due to contradiction
	// 1 - -1 if less than other, 0 if equal to other, 1 if greater than other

	int* rel = new int[2];
	rel[0] = 0; // by default we know nothing
	rel[1] = 0;

	if( opinions[0] != UND_SCR && other->opinions[0] != UND_SCR ) {
		rel[0] = 1;
		if( opinions[0] < other->opinions[0] )
			rel[1] = -1;
		else if( opinions[0] == other->opinions[0] )
			rel[1] = 0;
		else
			rel[1] = 1;
	}

	if( opinions[1] != UND_SCR && other->opinions[1] != UND_SCR ) {
		if( rel[0] != 0 ) {
			if( (opinions[1] < other->opinions[1] && rel[1] != -1) ||
					(opinions[1] == other->opinions[1] && rel[1] != 0) ||
					(opinions[1] > other->opinions[1] && rel[1] != 1) ) {
				rel[0] = 0;
				rel[1] = 1;
				return rel;
			}
		}
		else {
			rel[0] = 1;
		if( opinions[1] < other->opinions[1] )
			rel[1] = -1;
		else if( opinions[1] == other->opinions[1] )
			rel[1] = 0;
		else
			rel[1] = 1;
		}
	}
	return rel;
}

//////////////////////////////////////////////////////////////////////////////////////

RANKING::RANKING()
{
	//! default constructor - cannot be used
	printf("RANKINGS: default constructor used\n");
	exit(1);
}

RANKING::RANKING(int space_req)
{
	//! main constructor - allocates a given amount of memory for future list structure
	list = new RANK_REC*[space_req];
	size = 0;
	space = space_req;
}

RANKING::~RANKING()
{
	//! default destructor - frees the momory
	for(int i=0; i<size; i++)
		delete list[i];
	delete [] list;
}

void RANKING::insert(RANK_REC* newSubj, int pos)
{
	//! inserts a new element to a given position in the list
	// if the position is beyond the list boundary, append the element to the end

	if( pos < 0 )
	{
		printf("RANKINGS: insertion error - requested index less than 0\n");
		exit(1);
	}

	if( pos < size )
	{
		for(int i=size-1; i>=pos; i--)
			list[i+1] = list[i];
		list[pos] = newSubj;
		size++;
	}
	else
	{
		list[size] = newSubj;
		size++;
	}
}

int* RANKING::findConstraints(RANK_REC* newSubj)
{
	//! finds an interval of indices at which new element placement is possible
	// makes reasonable assumptions about the new element structure, i.e., that it came from a single source

	int src = newSubj->src();

//	printf("src=%d\n",src);

	// generate constraints
	int* constraints = new int[2];
	constraints[0] = INT_MIN;
	constraints[1] = INT_MAX;
	for(int i=0; i<size; i++)
	{
		if( list[i]->opinions[src] != UND_SCR )
		{
			if( list[i]->opinions[src] < newSubj->opinions[src] )
				constraints[0] = i;
			else if( constraints[0] != INT_MIN )
				break;
		}
	}
	for(int i=size-1; i>=0; i--)
	{
		if( list[i]->opinions[src] != UND_SCR )
		{
			if( list[i]->opinions[src] > newSubj->opinions[src] )
				constraints[1] = i;
			else if( constraints[1] != INT_MAX)
				break;
		}
	}

	return constraints;
}

void RANKING::autoInsert(RANK_REC* newSubj)
{
	int* constraints = findConstraints(newSubj);

//	printf("Inserting ");
//	newSubj->print();

//	printf("Constraints: (%d, %d)\n", constraints[0], constraints[1]);
	int possible_indices[2];
	possible_indices[0] = constraints[0] < 0 ? 0 : constraints[0]+1;
	possible_indices[1] = constraints[1] > size-1 ? size-1 : constraints[1]-1;
	int possible_elements = possible_indices[1] - possible_indices[0] + 1;
//	printf("Indices: (%d, %d) %d\n", possible_indices[0], possible_indices[1], possible_elements);

	if( possible_elements > 0 )
	{
		for(int i=possible_indices[0]; i<=possible_indices[1]; i++)
		{
			if( list[i]->id == newSubj->id )
			{
				if( list[i]->src() != newSubj->src() )
					list[i]->opinions[newSubj->src()] = newSubj->opinions[newSubj->src()];
				else
				{
					printf("RANKINGS: encountered a subject more than once in a single source\n");
					exit(1);
				}
//				printf("Record made to %d, src %d\n\n", i, newSubj->src());
				return;
			}
		}
	}

	// heuristics for minimizing number of conflicts
	int ins_idx;
	if(constraints[1] == INT_MAX)
		ins_idx = possible_indices[0];
	else if(constraints[0] == INT_MIN)
		ins_idx = possible_indices[1]+1;
	else
		ins_idx = 1 + ((possible_indices[0] + possible_indices[1])/2);

//	printf("Inserting to %d\n", ins_idx);

	insert(newSubj, ins_idx);

//	print();
//	printf("\n");

	delete [] constraints;
}

void RANKING::autoInsert(int id, double scr, int src)
{
	RANK_REC* rrec;

	switch(src)
	{
	case 0:
		rrec = new RANK_REC(id, scr, UND_SCR);
		break;
	case 1:
		rrec = new RANK_REC(id, UND_SCR, scr);
		break;
	default:
		printf("RANKINGS: src must be either 0 or 1\n");
		exit(1);
	}

	autoInsert(rrec);
}

void RANKING::merge(RANKING* other)
{
	if(size + other->size > space)
	{
		printf("RANKINGS: merge of too big subject list was requested\n");
		exit(1);
	}

	RANK_REC* rrec;

	for(int i=0; i<(other->size); i++)
	{
		rrec = new RANK_REC(other->list[i]);
		autoInsert(rrec);
	}
}

int RANKING::conflicts()
{
	int ids[size];
	for(int i=0; i<size; i++)
		ids[i] = list[i]->id;

	quicksortInt(ids, 0, size-1);
	int conflicts = 0;
	int* rel;
	for(int i=0; i<size-1; i++)
	{
		if( ids[i] == ids[i+1] )
			conflicts++;

		rel = list[i]->compare(list[i+1]);
		if( rel[0] == 0 && rel[1] == 1 )
			conflicts++;
		delete [] rel;
	}
	return conflicts;
}

int RANKING::ambiguities()
{
	int ambiguities = 0;
	int* rel;

	for(int i=0; i<size-1; i++)
	{
		rel = list[i]->compare(list[i+1]);
		if( rel[0] == 0 && rel[1] == 0 )
			ambiguities++;
		delete [] rel;
	}
	return ambiguities;
}

int* RANKING::ambiguousIDs()
{
	int* ambiguousIDs = new int[2*ambiguities()];
	int* rel;
	int aidx = 0;

	for(int i=0; i<size-1; i++)
	{
		rel = list[i]->compare(list[i+1]);
		if( rel[0] == 0 && rel[1] == 0 ) {
			if( list[i]->opinions[0] != UND_SCR ) {
				ambiguousIDs[aidx] = list[i]->id;
				ambiguousIDs[aidx+1] = list[i+1]->id;
			}
			else {
				ambiguousIDs[aidx+1] = list[i]->id;
				ambiguousIDs[aidx] = list[i+1]->id;
			}
			aidx += 2;
		}
		delete [] rel;
	}
	return ambiguousIDs;
}

int* RANKING::extendedAmbiguousIDs()
{
	int* ambiguousIDs = new int[4*ambiguities()];
	int* rel;
	int aidx = 0;

	int exp = 0;

	for(int i=0; i<size-1; i++)
	{
		rel = list[i]->compare(list[i+1]);
		if( rel[0] == 0 && rel[1] == 0 ) {
			if( list[i]->opinions[0] != UND_SCR ) {
				ambiguousIDs[aidx] = list[i]->id; // first two ends of free intervals are easily found
				ambiguousIDs[aidx+1] = list[i+1]->id;

				exp = i; // search for another ends of free intervals
				while(  exp >= 0 &&
								list[exp]->opinions[0] != UND_SCR &&
								list[exp]->opinions[1] == UND_SCR )
					exp--;
				ambiguousIDs[aidx+2] = list[exp+1]->id;
				exp = i+1;
				while(  exp <= size-1 &&
								list[exp]->opinions[0] == UND_SCR &&
								list[exp]->opinions[1] != UND_SCR )
					exp++;
				ambiguousIDs[aidx+3] = list[exp-1]->id;
			}
			else {
				ambiguousIDs[aidx+1] = list[i]->id;
				ambiguousIDs[aidx] = list[i+1]->id;

				exp = i;
				while(  exp >= 0 &&
								list[exp]->opinions[0] == UND_SCR &&
								list[exp]->opinions[1] != UND_SCR )
					exp--;
				ambiguousIDs[aidx+3] = list[exp+1]->id;
				exp = i+1;
				while(  exp <= size-1 &&
								list[exp]->opinions[0] != UND_SCR &&
								list[exp]->opinions[1] == UND_SCR )
					exp++;
				ambiguousIDs[aidx+2] = list[exp-1]->id;
			}
			aidx += 4;
		}
		delete [] rel;
	}
	return ambiguousIDs;
}

void RANKING::rescore()
{
	for(int i=0; i<size-1; i++)
	{
		list[i]->opinions[0] = ((double) i)/((double) (size-1));
		list[i]->opinions[1] = UND_SCR;
	}
	list[size-1]->opinions[0] = 1.0;
	list[size-1]->opinions[1] = UND_SCR;
}

void RANKING::print()
{
	printf("ID\tO1\tO2\n");
	for(int i=0; i<size; i++)
		list[i]->print();
}

void RANKING::printWithComparison()
{
	printf("ID\tO1\tO2\trelationship to the next element\n");
	int* comp;
	for(int i=0; i<size-1; i++) {
		list[i]->printWithoutNewline();
		comp = list[i]->compare(list[i+1]);
		printf("\t%d\t%d\n", comp[0], comp[1]);
		delete [] comp;
	}
	list[size-1]->print();
}
