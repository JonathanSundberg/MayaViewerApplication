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

	enum class MsgType
	{
		VERTEXPOS_MESH,
		TRANSFORMATION_NODE,
		LIGHT_POS,
		MESH_CREATION,
		BLABLABLA,
	};


	size_t BufferSize;
	size_t* Head;
	size_t* Tail;

	void*viewP;
	char*BuffPtr;

	char* BufferStart;


	string Verision;

	HANDLE hFileMapping;


	bool receive();

	bool createFileMap();
	bool createMVOF();

private:

};

