#include "rankings.h"

int main()
{
	int n1 = 9;
	int n2 = 8;

	RANKING* l1 = new RANKING(n1+n2);
	RANKING* l2 = new RANKING(n2);

	RANK_REC* newS1[n1];
	RANK_REC* newS2[n2];

	newS1[0] = new RANK_REC(1, 1.0, -1.0);
	newS1[1] = new RANK_REC(6, 0.6, -1.0);
	newS1[2] = new RANK_REC(9, 0.3, -1.0);
	newS1[3] = new RANK_REC(5, 0.35, -1.0);
	newS1[4] = new RANK_REC(10, 0.8, -1.0);
	newS1[5] = new RANK_REC(34, 0.55, -1.0);
	newS1[6] = new RANK_REC(3, 0.5, -1.0);
	newS1[7] = new RANK_REC(2, 0.25, -1.0);
	newS1[8] = new RANK_REC(4, 0.0, -1.0);

	newS2[0] = new RANK_REC(1, -1.0, 1.0);
	newS2[1] = new RANK_REC(3, -1.0, 0.5);
	newS2[2] = new RANK_REC(5, -1.0, 0.75);
	newS2[3] = new RANK_REC(2, -1.0, 0.2);
	newS2[4] = new RANK_REC(8, -1.0, 0.85);
	newS2[5] = new RANK_REC(65, -1.0, 0.35);
	newS2[6] = new RANK_REC(6, -1.0, 0.95);
	newS2[7] = new RANK_REC(4, -1.0, 0.0);

	for(int i=0; i<n1; i++)
		l1->autoInsert(newS1[i]);
	for(int i=0; i<n2; i++)
		l2->autoInsert(newS2[i]);

	l1->merge(l2);
//	l1->list[2]->opinions[0] = 0.1;
	l1->print();
	printf("Conflicts: %d, ambiguities: %d\n\n", l1->conflicts(), l1->ambiguities());

	l1->rescore();
	l1->print();

	return 0;
}
