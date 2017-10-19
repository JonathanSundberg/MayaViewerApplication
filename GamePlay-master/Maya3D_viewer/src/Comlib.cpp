#include "Comlib.h"


Comlib::Comlib(size_t Buffsize)
{
	BufferSize = Buffsize;
	Verision = "3D_Viewer";

	createFileMap();
	createMVOF();

	Mutex = CreateMutex(nullptr, false, L"Mutex");

}

Comlib::~Comlib()
{
	UnmapViewOfFile(viewP);
	CloseHandle(hFileMapping);
}

bool Comlib::createFileMap()
{
	hFileMapping = CreateFileMapping
	(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		BufferSize,
		L"hFileMapper"
	);

	if (hFileMapping == NULL)
	{
		//cout << "Could not create file mapping object ComlibMaya: " << GetLastError() << endl;
		exit(-1);
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		//cout << "Map already exists" << endl;

	}

	return true;
}

bool Comlib::createMVOF()
{
	viewP = MapViewOfFile
	(
		hFileMapping,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		BufferSize
	);

	if (viewP == NULL)
	{
		//cout << "Could not map view of file ComlibMaya: " << GetLastError() << endl;

		CloseHandle(hFileMapping);

		exit(-1);
	}

	Head = (size_t*)viewP; // giving the head variable the adress of the beginning of viewP.
	Tail = (size_t*)viewP + 1; // + 1 för att incrementera med 4 bytes
	BufferStart = (char*)viewP + (sizeof(size_t) * 2);

	if (Verision == "Maya") // probobly unecessary
	{
		*Head = 0;
		*Tail = 0;
	}

	BuffPtr = (char*)viewP;

	return true;
}

bool Comlib::receive(char* &msg,size_t* &length)
{

	// if we have new messages
	if (*Head != *Tail)
	{
		Header h;
		memcpy(&h, BufferStart + *Tail, sizeof(Header));

		// If it was a dummy message
		if (h.msgId == DUMMY)
		{
			WaitForSingleObject(Mutex, INFINITE);
			{
				*Tail = 0;
			}
			ReleaseMutex(Mutex);
			return false;
		}
		

		length = new size_t(h.length);
		*length = h.length;
		msg = new char[*length];
		size_t blocks = ceil((h.length + sizeof(Header)) / 64.0);
		size_t totalBlocksize = blocks * 64;

		memcpy(msg, BufferStart + *Tail + sizeof(Header), h.length);

		WaitForSingleObject(Mutex, INFINITE);
		{
			*Tail += totalBlocksize;
		}
		ReleaseMutex(Mutex);

		return true;


	}

	return false;
}