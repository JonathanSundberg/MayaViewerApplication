#include "maya_includes .h"
#include <iostream>

using namespace std;
MCallbackIdArray myCallbackArray;

void timerCallback(float elapsedTime, float lastTime, void* clientData)
{
	MString msg("Elapsed time: ");
	msg += elapsedTime;
	MGlobal::displayInfo(msg);
}
void AttrChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void* clientData)
{

}

void nodeAdded(MObject &node, void* clientData)
{
	MStatus status = MS::kSuccess;







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

		}
	}

	return res;
}

EXPORT MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin App(obj);

	MGlobal::displayInfo("Maya application unloaded!");

	return MS::kSuccess;
	
}