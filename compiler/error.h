#ifndef __ERROR_H_
#define __ERROR_H_

class Error {
protected:
	int ErrorCount;
	char* filename;

public:
	void Do_Error(char *err, int line = 0);
	void Do_Fatal(char *err, int line = 0);
};

#endif
