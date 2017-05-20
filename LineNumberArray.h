/* 
	LineNumberArray.h 
	struct and function declarations for Dynamic Line Number Arrays
*/

#include <stdbool.h>
#define LNAINIT	100	 // initial number of line numbers     

typedef struct dynamicLineNumberArray {
        int *lna;
        int idx;
        int capacity;
} DLNA;

bool createLNA(DLNA *dlna);
bool addLineNumber(DLNA *dlna, int lineNum);
void destroyLNA(DLNA *dlna);