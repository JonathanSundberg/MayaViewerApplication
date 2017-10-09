#include "maya_includes .h"
#include <iostream>

using namespace std;

EXPORT MStatus initializePlugin(MObject obj)
{

	MStatus res = MS::kSuccess;

	MFnPlugin MayaApplication(obj, "Maya plugin", "1.0", "Any", &res);
	if (MFAIL(res))
	{
		CHECK_MSTATUS(res);
	}

	MGlobal::displayInfo("Maya Application loaded!");

	return res;
}

EXPORT MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin App(obj);

	MGlobal::displayInfo("Maya application unloaded!");

	return MS::kSuccess;
	
}