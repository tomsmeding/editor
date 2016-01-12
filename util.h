#pragma once

#include <vector>
#include <string>

using namespace std;

unsigned int uniqid(void);

string basename(string s);
string basename(string s,string suf);
string dirname(string s);
string trim(string s);
bool startswith(string s,string prefix,size_t minlen=1);
vector<string> split(string s,char sep);
vector<string> splitSmart(string s,char sep); //trims and ignores double seps
string join(vector<string> v,char sep);
/*template <class InputIt>
string join(InputIt begin,InputIt end,char sep);*/

unsigned long long int msclock(void); //milliseconds sinds program start
