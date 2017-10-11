#pragma once
#include <Windows.h>
#include <string>
#include <tchar.h>
using namespace std;

class Comlib
{
public:
	Comlib(size_t Buffsize);
	~Comlib();

	bool receive(char* msg);

private:

	size_t BufferSize;
	size_t* Head;
	size_t* Tail;

	enum TYPE
	{
		NORMAL,
		DUMMY
	};

	struct Header
	{
		TYPE msgId;
		size_t length;
	};

	void*viewP;
	char*BuffPtr;

	char* BufferStart;


	string Verision;

	HANDLE hFileMapping;

	HANDLE Mutex;
	

	bool createFileMap();
	bool createMVOF();

private:

};

