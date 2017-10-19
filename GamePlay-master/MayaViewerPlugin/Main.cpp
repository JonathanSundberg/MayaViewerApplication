#include "maya_includes .h"
#include "ComlibMaya.h"
#include <iostream>
#include "MayaShared.h"
#include <sstream>
using namespace std;
#define BUFFERSIZE (200 * 1024)
#define MEGABYTE 1024
MCallbackIdArray myCallbackArray;
MCallbackId meshCreatedId;
ComlibMaya* Comlib;
Mesh createdMesh;
char *Message;

size_t length;
void timerCallback(float elapsedTime, float lastTime, void* clientData)
{
	MString msg("Elapsed time: ");
	msg += elapsedTime;
//	MGlobal::displayInfo(msg);
}


void findCamera()
{
	MItDag dagIterator(MItDag::kBreadthFirst, MFn::kCamera);

	for (; !dagIterator.isDone(); dagIterator.next())
	{
		if (dagIterator.currentItem().apiType() == MFn::Type::kCamera)
		{

			MFnCamera myCam = dagIterator.currentItem();

			if (myCam.name() == "perspShape")
			{
				MString msg = "Perspective camera: ";
				msg += myCam.absoluteName();

				MGlobal::displayInfo(msg);
			}

		}
	}

}

void meshChanged(MObject & node, void* clientData)
{

	MString msg = "A mesh has been changed! ";
	MFnDagNode myDag = node;
	msg += myDag.absoluteName();

	MGlobal::displayInfo(msg);

}
void getNewMeshData(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void* clientData)
{
	MStatus status;
	MFnMesh newMesh(plug.node(), &status);
	if (status == MS::kSuccess)
	{
		//getting vertex index data
		MIntArray vtxCount;
		MIntArray vtxList;
		newMesh.getVertices(vtxCount, vtxList);
		//cerr << "Vtxlist index length: " << vtxList.length() << endl;
		int *indexArray = new int[vtxList.length()];
		vtxList.get(indexArray);
		for (size_t i = 0; i < vtxList.length(); i++)
		{
			//cerr << "VTXIndex: " << indexArray[i] << endl;
			createdMesh.vtxIndices.push_back(indexArray[i]);
			
		}
		createdMesh.sizeOfVtxIndex = createdMesh.vtxIndices.size();

		//Getting vertex data
		MFloatPointArray vtxArray;
		newMesh.getPoints(vtxArray);
		createdMesh.meshId = 1;
		createdMesh.name = "TestMesh";
		//cerr << "Vertex count: " << vtxArray.length() << endl;
		for (size_t i = 0; i < vtxArray.length(); i++) 
		{
			Vertex vtx;
			vtx.position[0] = (double)vtxArray[i].x;
			vtx.position[1] = (double)vtxArray[i].y;
			vtx.position[2] = (double)vtxArray[i].z;
			createdMesh.vertices.push_back(vtx);
		}
		createdMesh.sizeOfVertices = createdMesh.vertices.size(); // getting the size of the vertices vector and storing it in sizeOfVertices to be able to read vector in comlib

		//Getting normal data
		MFloatVectorArray normals;
		newMesh.getNormals(normals);
		//cerr << "Size of normal arr: " << normals.length() << endl; 
		for (size_t i = 0; i < normals.length(); i++)
		{
			Normal meshNormal;
			float arr[3];
			normals[i].get(arr);
		//	cerr << "Normals: X: " << arr[0] << " Y: " << arr[1] << " Z: " << arr[2] << endl;
			meshNormal.normal[0] = (double)arr[0];
			meshNormal.normal[1] = (double)arr[1];
			meshNormal.normal[2] = (double)arr[2];
			createdMesh.normals.push_back(meshNormal);
		}
		createdMesh.sizeOfNormals = createdMesh.normals.size();

		//Getting normal indices
		MIntArray normalCounts;
		MIntArray meshNormalIds;
		newMesh.getNormalIds(normalCounts, meshNormalIds);
		//cerr << "Normal indices amount: " << meshNormalIds.length() << endl;
		for (size_t i = 0; i < meshNormalIds.length(); i++)
		{
			createdMesh.normalIndices.push_back(meshNormalIds[i]);
		}
		createdMesh.sizeOfNormalIndex = createdMesh.normalIndices.size();
		
		createdMesh.vertices.clear();
		createdMesh.vtxIndices.clear();
		createdMesh.normals.clear();
		createdMesh.normalIndices.clear();
		MMessage::removeCallback(meshCreatedId);//Removes the callback
		delete[] indexArray;
	}
}

void childAdded(MDagPath &child, MDagPath &parent, void* clientData)
{
	MStatus status;
	if (child.node().apiType() == MFn::kMesh)
	{
		MGlobal::displayInfo("NODE TYPE: " + (MString)child.node().apiTypeStr());
		MCallbackId meshChangedID = MPolyMessage::addPolyTopologyChangedCallback
		(
			child.node(),
			meshChanged,
			NULL,
			&status
		);
		if (status == MS::kSuccess)
		{
			if (myCallbackArray.append(meshChangedID) == MS::kSuccess)
			{
				//MGlobal::displayInfo("MeshChanged callback added successfully!");
			}
		}
		meshCreatedId = MNodeMessage::addAttributeChangedCallback(
			child.node(),
			getNewMeshData,
			NULL,
			&status
		);
	}
}

void AttrChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void* clientData)
{
//	MGlobal::displayInfo("Plug name: " + plug.name());
	if (msg & MNodeMessage::AttributeMessage::kAttributeSet)
	{
		MStatus status;
		if (plug.node().apiType() == MFn::Type::kTransform)
		{

			MFnTransform transNode(plug.node(), &status);


			if (status == MS::kSuccess)
			{

				MString name = plug.partialName();

				double scale[3] = { 0,0,0 };
				double RotationX, RotationY, RotationZ, RotationW;
				double TranslationX, TranslationY, TranslationZ;
				


				if (name == "t" || name == "tx" || name == "ty" || name == "tz")
				{

					TranslationX = transNode.getTranslation(MSpace::kTransform).x;
					TranslationY = transNode.getTranslation(MSpace::kTransform).y;
					TranslationZ = transNode.getTranslation(MSpace::kTransform).z;

					MString changed = "Attribute changed: " + name + " " + TranslationX + " " + TranslationY + " " + TranslationZ;

					MGlobal::displayInfo(changed);
					
	
					Translation nodeTransform;
					
					nodeTransform.TypeHeader = MsgType::TRANSFORM_NODE_TRANSFORM;
					nodeTransform.Tx = TranslationX;
					nodeTransform.Ty = TranslationY;
					nodeTransform.Tz = TranslationZ;

					memcpy(Message, &nodeTransform, sizeof(Translation));
	
				}
				if (name == "r" || name == "rx" || name == "ry" || name == "rz")
				{
					

					transNode.getRotationQuaternion(RotationX, RotationY, RotationZ, RotationW);
					MString changed = "Attribute changed: " + name + " " + RotationX + " " + RotationY + " " + RotationZ;

					MGlobal::displayInfo(changed);
				}
				if (name == "s" || name == "sx" || name == "sy" || name == "sz")
				{
					
					transNode.getScale(scale);

					MString changed = "Attribute changed: " + name + " " + scale[0] + " " + scale[1] + " " + scale[2];

					MGlobal::displayInfo(changed);
				}


			}
			else
			{
				MGlobal::displayInfo("failed to make a transform");
				MGlobal::displayError(status.errorString());
			}
		}
		else if (plug.node().apiType() == MFn::Type::kMesh)
		{

			if (plug.node().hasFn(MFn::kMesh))
			{

				MFnAttribute myAttr = plug.attribute();
				MString attributeName = myAttr.name();

				if (attributeName == "pnts")
				{

					MFnMesh myMesh(plug.node(), &status);
					if (status)
					{
						if (plug.logicalIndex() < 100000) // for not printing out some weird index
						{
							MPoint myPoint;

							myMesh.getPoint(plug.logicalIndex(), myPoint, MSpace::kTransform);

							MString Vertex = "Vertex Changed: ";
							Vertex += "[";
							Vertex += plug.logicalIndex();
							Vertex += "]";
							Vertex += " ";
							Vertex += myPoint.x;
							Vertex += " ";
							Vertex += myPoint.y;
							Vertex += " ";
							Vertex += myPoint.z;

							MGlobal::displayInfo(Vertex);

						}
					}
					else
					{
						MGlobal::displayInfo("failed to make a Mesh");
						MGlobal::displayError(status.errorString());
					}
				}
			}
		}

		if (plug.node().apiType() == MFn::kPointLight)
		{
			MGlobal::displayInfo("Light Attribute Changed!");
		}

	}

}

void nodeAdded(MObject &node, void* clientData)
{
	if (node.hasFn(MFn::kMesh))
	{
		MStatus status;
		MFnMesh mesh(node, &status);
		if (status == MS::kSuccess)
		{
			MGlobal::displayInfo("Hey");
			MGlobal::displayInfo(mesh.name());
		}
	}
	
}

EXPORT MStatus initializePlugin(MObject obj)
{
	MObject callBackNode;
	
	MStatus res = MS::kSuccess;

	MFnPlugin MayaApplication(obj, "Maya plugin", "1.0", "Any", &res);
	if (MFAIL(res))
	{
		CHECK_MSTATUS(res);
	}

	MGlobal::displayInfo("Maya Application loaded!");


	Comlib = new ComlibMaya(BUFFERSIZE);
	Message = new char[MEGABYTE];
	

	MStatus status = MS::kSuccess;

	MCallbackId id = MTimerMessage::addTimerCallback
	(
		5,
		timerCallback,
		NULL, &status
	);

	if (status == MS::kSuccess) {

		if (myCallbackArray.append(id) == MS::kSuccess)
		{

		}
	}

	MCallbackId addNodeID = MDGMessage::addNodeAddedCallback
	(
		nodeAdded,
		"dependNode",
		NULL,
		&status
	);

	if (status == MS::kSuccess)
	{
		if (myCallbackArray.append(addNodeID) == MS::kSuccess)
		{

		}
	}

	MCallbackId AttrChangedID = MNodeMessage::addAttributeChangedCallback
	(
		callBackNode,
		AttrChanged,
		NULL,
		&status
	);

	if (status == MS::kSuccess)
	{

		if (myCallbackArray.append(AttrChangedID) == MS::kSuccess)
		{
			MGlobal::displayInfo("Attribute changed callback added successfully!");
		}
	}

	MCallbackId DAGChildAddedID = MDagMessage::addChildAddedCallback(
		childAdded,
		NULL,
		&status
	);
	if (status == MS::kSuccess)
	{
		if (myCallbackArray.append(DAGChildAddedID) == MS::kSuccess)
		{

		}
	}


	//MStringArray eventNames;
	//MEventMessage::getEventNames(eventNames);
	//for (size_t i = 0; i < eventNames.length(); i++)
	//{
	//	MGlobal::displayInfo(eventNames[i]);
	//}

	findCamera();
	return res;
}

EXPORT MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin App(obj);

	MMessage::removeCallbacks(myCallbackArray);
	MGlobal::displayInfo("Maya application unloaded!");

	delete Comlib;
	return MS::kSuccess;
	
}