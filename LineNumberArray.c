/* LineNumberArray.c */

#include <LineNumberArray.h>
#include <stdlib.h>
#include <stdio.h>

bool createLNA(DLNA *dlna)
{
	dlna->lna = malloc(sizeof(int) * LNAINIT);
	if ( dlna->lna == NULL )
		return false;
    dlna->capacity = LNAINIT;
    dlna->idx = 0;
    return true;
}

bool addLineNumber(DLNA *dlna, int lineNum)
{
	// TODO check that realloc works, use temp buf
	if ( dlna->idx == (dlna->capacity - 1) )  
	{
		//grow the LNA
    	dlna->lna = realloc(dlna->lna,sizeof(int) * (dlna->capacity) * 2);
    	if ( dlna->lna == NULL )
    		return false;
    	dlna->capacity = (dlna->capacity * 2);
    }
    int i = dlna->idx++;
    dlna->lna[i] = lineNum;
    return true;
}

void destroyLNA(DLNA *dlna)
{
	free(dlna->lna);
	dlna->capacity = 0;
	dlna->idx = 0;
} 