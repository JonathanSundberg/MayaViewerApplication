#pragma once
#define BUFFERSIZE (200 * 1<<20)
#define MEGABYTE 1 * 1<<20
#include <vector>
enum class MsgType
{
	CREATE_MESH,

	VERTEX_TRANSLATION,
	TRANSFORM_NODE_TRANSFORM,
	TRANSFORM_NODE_ROTATE,
	TRANSFORM_NODE_SCALE,
};

struct Translation
{
	MsgType TypeHeader;
	double Tx;
	double Ty;
	double Tz;
};

struct Rotation
{
	double RotX;
	double RotY;
	double RotZ;
	double RotW;
};

struct Scaling
{
	double ScaleX;
	double ScaleY;
	double ScaleZ;
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
