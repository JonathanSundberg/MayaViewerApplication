#pragma once
#include <Windows.h>
#include <tchar.h>
#include <string>
using namespace std;
class ComlibMaya
{
public:
	ComlibMaya(size_t Buffsize);
	~ComlibMaya();

	bool send(const void* msg, const size_t length);


private:
	size_t BufferSize;
	size_t* Head;
	size_t* Tail;
	enum class TYPE
	{
		NORMAL,
		DUMMY
	};

	struct Header
	{
		TYPE msgId;
		size_t length;



	};

	void* viewP;
	char* BuffPtr;

	char* BufferStart;

	
	string Verision;

	HANDLE hFileMapping;



	HANDLE Mutex;

	bool createFileMap();
	bool createMVOF();









};

