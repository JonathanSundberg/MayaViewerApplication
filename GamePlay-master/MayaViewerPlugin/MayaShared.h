#pragma once
#define BUFFERSIZE (200 * 1<<20)
#define MEGABYTE 1 * 1<<20
#include <vector>
enum class MsgType
{
	CREATE_MESH,

	VERTEX_TRANSLATION,
	TRANSFORM_NODE_TRANSFORM,
	CAMERA_UPDATE,
	COLOR_UPDATE,
	NODE_REMOVED,
	TEXTURE_UPDATE,
	MATERIAL_CHANGE
};

struct TransformData
{
	MsgType TypeHeader;
	float Tx;
	float Ty;
	float Tz;
	float Rx;
	float Ry;
	float Rz;
	float Rw;
	float Sx;
	float Sy;
	float Sz;
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
struct UV {
	float U, V;
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
	int sizeOfUV;
	int sizeOfUVIndex;
	float color[4];
	char name[75];
	char materialName[75];
};

struct MCamera {
	MsgType headerType;
	float fViewMatrix[4][4];
	float Rot[4];
	float aspectRatio;
	float FOV;
	float farPlane;
	float nearPlane;
	float zoom;
	bool isOrtho;
	char name[75];
};

struct Color
{
	MsgType headerType;
	float colors[3];
	char meshName[75];
	char matName[75];
};
struct TextureName
{
	MsgType headerType;
	char file[255];
	char matName[75];
};
struct NodeName
{
	MsgType headerType;
	char name[75];
};
struct MatChange
{
	MsgType headerType;
	char texture[255];
	char meshName[75];
	char matName[75];
};