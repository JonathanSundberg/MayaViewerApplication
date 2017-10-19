#pragma once
#include <vector>
enum class MsgType
{
	VERTEX_TRANSLATION,
	VERTEX_ROTATION,
	VERTEX_SCALE,
	TRANSFORM_NODE_TRANSFORM,
	TRANSFORM_NODE_ROTATE,
	TRANSFORM_NODE_SCALE
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
	double position[3];
};
struct Normal
{
	double normal[3];
};
struct Mesh {
	int sizeOfVtxIndex;
	int sizeOfVertices;
	int sizeOfNormals;
	int sizeOfNormalIndex;
	string name;
	int meshId;
	vector<int> normalIndices;
	vector<Normal> normals;
	vector<int> vtxIndices;
	vector<Vertex> vertices;
};