#pragma once
#define BUFFERSIZE (200 * 1024)
#define MEGABYTE 1024
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

