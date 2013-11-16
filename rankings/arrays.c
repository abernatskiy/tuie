#include "arrays.h"

void quicksortInt(int* A, int start, int end)
{
	int size = end-start+1;
	if( size < 2 )
		return;

	int pivot = A[0];
	int* left = new int[size-1];
	int* right = new int[size-1];

	int l = 0; int r = 0; int i;
	for(i=1; i<size; i++)
	{
		if( A[i] <= pivot )
		{
			left[l] = A[i];
			l++;
		}
		else
		{
			right[r] = A[i];
			r++;
		}
	}
	int leftSize = l;
	int rightSize = r;

	quicksortInt(left, 0, leftSize-1);
	quicksortInt(right, 0, rightSize-1);

	i = 0;
	for(l=0; l<leftSize; l++)
	{
		A[i] = left[l];
		i++;
	}
	A[i] = pivot;
	i++;
	for(r=0; r<rightSize; r++)
	{
		A[i] = right[r];
		i++;
	}

	delete [] left;
	delete [] right;
	return;
}

