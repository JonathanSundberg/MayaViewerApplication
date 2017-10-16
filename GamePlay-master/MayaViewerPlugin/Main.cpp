#include "maya_includes .h"
#include "ComlibMaya.h"
#include <iostream>
#include "MayaShared.h"
#include <sstream>
using namespace std;
#define BUFFERSIZE (200 * 1024)
#define MEGABYTE 1024
MCallbackIdArray myCallbackArray;
//Try adding eventName callback for SelectionChange also add postUserEvent to change the void* clientData to send an MObject node to be able to work with the mesh selected after creation.
// https://nccastaff.bournemouth.ac.uk/jmacey/RobTheBloke/www/maya/MSelectionList.html //Maya selection guide
ComlibMaya* Comlib;
char *Message;

size_t length;
void timerCallback(float elapsedTime, float lastTime, void* clientData)
{
	MString msg("Elapsed time: ");
	msg += elapsedTime;
//	MGlobal::displayInfo(msg);
}

void nodeAddedToModel(MObject &node, void* clientData) {

	MStatus status;
	MGlobal::displayInfo("Node added to model type: " + (MString)node.apiTypeStr());
	
	MFnMesh mesh(node, &status);
	if (status == MS::kSuccess)
	{
		MFloatPointArray vtxArray;
		MGlobal::displayInfo("Node name: " + mesh.name());

		mesh.getPoints(vtxArray, MSpace::kWorld);
		int nrOfVerts = vtxArray.length();
		MString strVerts; strVerts += nrOfVerts;
		MGlobal::displayInfo(strVerts);
		for (size_t i = 0; i < nrOfVerts; i++)
		{
			MString vtxInfo;
			vtxInfo += "X: "; vtxInfo += vtxArray[i].x; vtxInfo += " Y: "; vtxInfo += vtxArray[i].y; vtxInfo += " Z: "; vtxInfo += vtxArray[i].z;
			MGlobal::displayInfo(vtxInfo);
		}
	}
	MFnMesh tempMesh = node;
	MGlobal::displayInfo("Mesh name: " + tempMesh.name());
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
void attrAdded(MNodeMessage::AttributeMessage msg, MPlug &plug, void *clientData)
{
	if (msg & MNodeMessage::AttributeMessage::kAttributeAdded)
	{
		MGlobal::displayInfo("Attribute added!!");
		MStatus status;
		MFnMesh mesh(plug.node(), &status);
		if (status == MS::kSuccess)
		{
			MGlobal::displayInfo("Attribute ADDED!!!");
		}
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
		MCallbackId nodeAddedToModelID = MModelMessage::addNodeAddedToModelCallback(
			child.node(),
			nodeAddedToModel,
			nullptr,
			&status
		);
		if (status == MS::kSuccess)
		{
			if (myCallbackArray.append(nodeAddedToModelID) == MS::kSuccess) {

				//	MGlobal::displayInfo("NodeAddedToModel callback added successfully!");
			}
		}

		MCallbackId attrAddedId = MNodeMessage::addAttributeAddedOrRemovedCallback(
			child.node(),
			attrAdded,
			NULL,
			&status
		);
		if (status == MS::kSuccess)
		{
			if (myCallbackArray.append(attrAddedId) == MS::kSuccess)
			{
				MGlobal::displayInfo("Attribute added/removed callback added successfully!");
			}
		}
	}
}

void AttrChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void* clientData)
{
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


	MStringArray eventNames;
	MEventMessage::getEventNames(eventNames);
	for (size_t i = 0; i < eventNames.length(); i++)
	{
		MGlobal::displayInfo(eventNames[i]);
	}

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