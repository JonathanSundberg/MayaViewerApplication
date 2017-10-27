#pragma once
#define BUFFERSIZE (200 * 1<<20)
#define MEGABYTE 1 * 1<<20
#include <vector>
enum class MsgType
{
	CREATE_MESH,

	VERTEX_TRANSLATION,
	TRANSFORM_NODE_TRANSFORM,
	CAMERA_UPDATE

};

struct Translation
{
	MsgType TypeHeader;
	float Tx;
	float Ty;
	float Tz;
	char name[75];
};

struct Rotation
{
	float RotX;
	float RotY;
	float RotZ;
	float RotW;
	char name[75];
};

struct Scaling
{
	float ScaleX;
	float ScaleY;
	float ScaleZ;
	char name[75];
};
struct Vertex {
	float position[3];
};
struct Normal
{
	float normal[3];
};
struct MayaMesh {
	MsgType headerType;
	int sizeOfVtxIndex;
	int sizeOfVertices;
	int sizeOfNormalIndex;
	int sizeOfNormals;
	char name[75];
};

struct Camera {
	MsgType headerType;
	float fViewMatrix[4][4];
	char name[75];
};
