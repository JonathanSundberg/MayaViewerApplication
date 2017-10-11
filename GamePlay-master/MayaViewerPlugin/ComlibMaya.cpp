#include"ComlibMaya.h"

ComlibMaya::ComlibMaya(size_t Buffsize)
{
	BufferSize = Buffsize;
	Verision = "Maya";
	
	createFileMap();
	
	createMVOF();

}

ComlibMaya::~ComlibMaya()
{
	UnmapViewOfFile(viewP);
	CloseHandle(hFileMapping);
}

bool ComlibMaya::createFileMap()
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


bool ComlibMaya::createMVOF()
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

bool ComlibMaya::send(const void* msg, const size_t length)
{

	size_t headerSize = sizeof(Header);
	size_t msgSize = headerSize + length;

	double nrOfBlocks = ceil(msgSize / 64.0);

	size_t totalBlockSize = nrOfBlocks * 64;

	if (*Head + totalBlockSize >= *Tail && *Head < *Tail)
	{
		return false;
	}
	else if (*Head + totalBlockSize >= BufferSize && *Tail == 0)
	{
		return false;
	}

	
	return true;
}