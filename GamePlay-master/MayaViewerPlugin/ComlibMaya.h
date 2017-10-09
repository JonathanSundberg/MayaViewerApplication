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


	enum class MsgType
	{

	};


	size_t BufferSize;
	size_t* Head;
	size_t* Tail;

	void*viewP;
	char*BuffPtr;

	char* BufferStart;

	
	string Verision;

	HANDLE hFileMapping;


	bool send();

	bool createFileMap();
	bool createMVOF();

private:







};

