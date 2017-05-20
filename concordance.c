/* Concordance, C version

(See also, the C++ version of this in the same project.) 
This program takes a text document as input (e.g. from Project Gutenberg), and produces, via stdout, 
an alphabetically-ordered listing of each unique word (case insensitive) in the document along with the 
lines from the input document that the word appears on. So, for example, taking the following text as 
input

This is
some kind OF text it 
Is an example of text

Supplied as an argument to an executable named "concordance" in the current directory: 

$ ./concordance ./input.txt

The following would be the output:
an 3 
example 3 
is 1 3
it 2
kind 2
of 2 3 
some 2 
text 2 3 
this 1

Given that the list of words will need to be alphabetized, I considered using some sort 
of tree structure. But since the table only needs to be sorted once, I stuck with
a hashmap implementation.

The set of words contained in the document will not be known in advance, so collisions are 
unavoidable. For collision resolution, I used open addressing rather than chaining.
With open addressing, I can simply use qsort() to sort the hashmap when I'm done.

Open addressing considerations: 
- I use linear probing and search for open slots in a circular fashion, that is, 
  I'll need to be sure to wrap around to the front of hashmap when I reach the end.
- For now, I'm using a HashMap that I hope is large enough to contain the English language
  but a "real" application would need to figure out how to grow this table when it's full
  https://en.oxforddictionaries.com/explore/how-many-words-are-there-in-the-english-language
- Open addressing requires a good hash function that avoids clustering.
  The "djb2" hash function, from the link below, sounds like a good one
  http://www.cse.yorku.ca/~oz/hash.html

An array of line numbers will need to be stored for each word. The linenumbers for each  Word Entry, WE,
will be stored as dynamic arrays of integers (LineNumberArray = LNA). When the list of Line Numbers for 
a given word is larger than current capacity, the array will need to be realloc()'ed.
Declarations for these LNAs can be found in LineNumberArray.h and the functions to create, destroy and 
add to an LNA can be found in LineNumberArray.c

Assumptions:
- Words are delimited by spaces
- For now, I'm ignoring hyphens between words and also at EOL
- I'm also ignoring non-alpha characters in general
- Words repeated on a line will have a line number entry for every time they appear

*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <LineNumberArray.h>

#define MAXCHARS 45		 // maximum word length
#define HASHSIZE 0x3FFFF // 262,144 decimal (big enough?)

typedef struct wordEntry {
		unsigned char word[MAXCHARS];
        int *lna;
        int lnaIdx;          //Line Number Array index
        int lnaCapacity;     //Line Number Array capacity
} WE;

/*
* Hash function, djb2, by Dan J Bernstein
* https://en.wikipedia.org/wiki/Daniel_J._Bernstein
*/
unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return (hash % HASHSIZE);
}

bool getWord(FILE *file, unsigned char *buf, int *nextLine)
{
    unsigned int ch, i = 0;
    //TODO: check for MAXCHARS    
    while( ((ch = fgetc(file)) != EOF && isalpha(ch)) || ch == '\'')                
    {
        if (i == MAXCHARS -1)   //check for words longer than MAXCHARS
        {
            buf[i] = '\0';
            printf("word: %s exceeds maximum word length\n", buf);
            //or could just continue and process as two words
            return false;
        }
        if (ch == '\'')     //apostrophe
            buf[i++] = ch;
        else
            buf[i++] = tolower(ch);
    }
    if (ch == EOF)
        return false;
    if (ch == '\n')
        (*nextLine)++;
    buf[i] = '\0';
    return true;
}

/* qsort struct comparision function */ 
int WE_cmp_by_word(const void *a, const void *b) 
{ 
    WE *ia = (WE *)a;
    WE *ib = (WE *)b;
    return strcmp((char *)ia->word, (char *)ib->word);
}

int main(int argc, char **argv) //find unique words in file and keep track of linenumbers 
{
	FILE *fp, *fopen();            //fp == where we are in file
	unsigned char chBuf[MAXCHARS]; //current word read from file
	unsigned long e;               //index/entry in hash map, HashM.
	int entries;	               //number of entries used in hash map
	bool found;			           //true when an availble spot or matching entry is found
    int curLine;                   //line number for word returned from getWord()
    int nextLine;                  //line to look for in next call to getWord()

    WE *hashM = calloc( HASHSIZE, sizeof(WE) );
	if (!hashM)
	{
		printf("Error: failure to allocate hashMap.\n");
		return 1;
	}
    fp = fopen(argv[1], "r"); // "r" for read
	if (!fp)
    {
        printf("Error: File open failed.\n");
        return 1;
    }

    entries = 0;
    curLine = nextLine = 1;
    while ( getWord(fp, chBuf, &nextLine) )  //while not EOF
    {
        //printf("curWord = %s curLine = %d \n", curWord, curLine);
        if (chBuf[0] == '\0')   //blank line
        {
            curLine = nextLine;
            continue;           //check next word
        }
        e = hash(chBuf);
        found = false;
        while (!found)
        {
        	if ( hashM[e].lnaCapacity == 0 )   //if hashMap entry is available
        	{
                strcpy((char *)hashM[e].word, (char *)chBuf);  //add new word to hash map
        		entries++;                     //record # of unique words
        		found = true;
        		if ( !(createLNA( (DLNA *) &(hashM[e].lna))) )
        		{       			
					printf("Error: failure to createLNA\n");
					return 1;
        		}
                if ( !(addLineNumber( (DLNA*) &(hashM[e].lna), curLine)) ) //add current LN to array
                {
                    printf("Error: failure to addLineNumber\n");
                    return 1;
                }
        	}
        	else if ( !(strcmp((char *)chBuf, (char *)hashM[e].word)) ) //if chBuf word is in table 
        	{
        		found = true;
        		addLineNumber( (DLNA*) &(hashM[e].lna), curLine);
        	}
        	else //hash collision or unsuccessful probe, search ahead for available entry
       		{
        		if (e == HASHSIZE - 1)	//if we've reached the end
        			e = 0;			//circle to top of hash map
        		else 				//otherwise
        			e++;			//look at next entry for a match or open entry
        	}

        	if (entries == HASHSIZE) //don't keep looking, the table is full
        		break; 
        	
        } //end while (!found)

        if ( entries == HASHSIZE)   //don't get another word
        {
        	//TODO: find a way to rehash current entries and re-grow table
        	//or start over with an even bigger hash map
        	fprintf(stderr, "Error: Hash Map Capacity exceeded\n");
        	fprintf(stderr, "Printing results so far\n");
        	break; //skip to printing results
        }
        curLine = nextLine;

    }	//end while ( getWord() ), that is not EOF

	fclose(fp);

    qsort( hashM, HASHSIZE, sizeof(WE), WE_cmp_by_word ); //sort hashMap to prepare for printing

    //print results
    for(int i=0; i<HASHSIZE; i++) 
    	if (hashM[i].lnaCapacity != 0)        //don't print empty entries
    	{
        	printf("%s ", hashM[i].word);            //print word
    		for (int k=0; k < hashM[i].lnaIdx; k++)  //only print used LNA entries
    		{
    			printf("%d ", hashM[i].lna[k]);      //print line number
    		}
    		destroyLNA( (DLNA *) &(hashM[i].lna) );	 //free line number array for this entry
    		printf("\n");
    	}
	free (hashM);
	return 0;
}



