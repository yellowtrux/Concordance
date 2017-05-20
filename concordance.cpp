/* concordance.cpp

I originally wrote this in C and wondered if it was any faster than doing the same(ish) thing in C++. 
The C version is roughly 30-40% faster (determined by running both C and C++ versions with "time," 
using the same input files). 

**TODO: take out other characters, ",', =, etc. in addition to spaces. 

This program takes a text document as input (e.g. from Project Gutenberg), and produces, via stdout, 
an alphabetically-ordered listing of each unique word (case insensitive) in the document along with the 
lines from the input document that the word appears on. So, for example, taking the following text as 
input:  

This is
some kind OF text it 
Is an example of text

Supplied as an argument to the executable (./a.out, etc): 

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

I don't know a way to sort an unordered_map, so I used a regular, sorted, map. Since the standard library 
map is implemented as a tree rather than a hash table, it should take up less space. Also, since I used
C++ strings to store the line numbers, I didn't have to worry about dynamically sizing arrays of line 
numbers (see concordance.c, also in this project). Not surprisingly, it's much easier to code this in C++, 
but it is a bit slower than the C version that uses a proper hash table.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <algorithm>
//#include <chrono>
using namespace std;

int main(int argc, char **argv) //find unique words in file and keep track of linenumbers 
{
    //auto start = chrono::steady_clock::now();
    map <string, string> WE; //WE for word entry
    map <string, string>::iterator it;
    string line, word;   

    string fname = argv[1]; 
    ifstream file_in(fname);
    if (!file_in) {
        cout << "could not open file" << endl;
    }

    int curLine = 1;
    string lineNumStr = to_string(curLine) + " ";
    while (!file_in.eof()) {
        getline(file_in, line);
        transform(line.begin(), line.end(), line.begin(), ::tolower); //make everything lowercase
        istringstream iss(line);
        while (iss >> word) {
            WE[word] += lineNumStr; //add a linenumber for this word
        }
        lineNumStr = to_string(++curLine) + " ";
    }

    //print results
    for (it = WE.begin(); it != WE.end(); ++it) {
        cout << it->first << " ";   //print the word
        cout << it->second;         //print the linenumbers
        cout << endl;
    }  

    //auto end = chrono::steady_clock::now(); //elapsed time for this program
    //auto diff = end - start;
    //cout << chrono::duration <double, milli> (diff).count() << " ms" << endl;

	return 0;
}



