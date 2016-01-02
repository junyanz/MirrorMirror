#ifndef UTILITY_H_
#define UTILITY_H_
#include "common.h"

namespace utility {
class CUtility {
public:
	CUtility() {}
	~CUtility() {}
	static int FindFiles(string _fName, vector<string>& _nameVec);
	static int FindImageFiles(string fName, vector<string>& nameVec,
			vector<string> &extVec);
	static int itoa(int val, char* buf);
	static void AddSuffix(vector<string>& _nameVec, string _suffix, vector<string>& _outNameVec);
	static void mkdirs( vector<string> _dirNameVec);
	static void mkdirs(string _dirName);
private:
	static bool ParseImageFileName(string& fileName, string& ext);
};
}

#endif //UTILITY_H_
