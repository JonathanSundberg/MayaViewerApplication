#include "maya_includes .h"
#include "ComlibMaya.h"
#include <iostream>
#include "MayaShared.h"
#include <sstream>
using namespace std;

MCallbackIdArray myCallbackArray;

ComlibMaya* Comlib;
char *Message;

size_t length;
void timerCallback(float elapsedTime, float lastTime, void* clientData)
{
	MString msg("Elapsed time: ");
	msg += elapsedTime;
	//MGlobal::displayInfo(msg);
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
void childAdded(MDagPath &child, MDagPath &parent, void* clientData)
{
	MStatus status;

	if (child.node().apiType() == MFn::kMesh)
	{	
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
				MGlobal::displayInfo("MeshChanged callback added successfully!");
			}
		}	
	}
}

void recursiveTransform(MFnDagNode& Parent, bool cameraTransform)
{
	MStatus status;
	MGlobal::displayInfo("Entered rekursive transform");

	MDagPath path;

	Parent.getPath(path);

	MFnTransform transNode(path, &status);


	for (size_t i = 0; i < Parent.childCount(); i++)
	{
		if (Parent.child(i).apiType() == MFn::Type::kTransform)
		{
			MFnDagNode childDag = Parent.child(i);
			recursiveTransform(childDag,cameraTransform);
		}
	}

	if (status == MS::kSuccess)
	{

		//MString name = plug.partialName();






		double scale[3] = { 0,0,0 };
		double RotationX, RotationY, RotationZ, RotationW;
		double TranslationX, TranslationY, TranslationZ;

		//if (name == "t" || name == "tx" || name == "ty" || name == "tz")
		//{

			MString changed;


			TranslationX = transNode.getTranslation(MSpace::kWorld).x;
			TranslationY = transNode.getTranslation(MSpace::kWorld).y;
			TranslationZ = transNode.getTranslation(MSpace::kWorld).z;

			changed += "Attribute changed (World) T: ";
			changed += TranslationX;
			changed += " ";
			changed += TranslationY;
			changed += " ";
			changed += TranslationZ;
			


			

			

		//}
		//if (name == "r" || name == "rx" || name == "ry" || name == "rz")
		//{

			transNode.getRotationQuaternion(RotationX, RotationY, RotationZ, RotationW, MSpace::kWorld);
			

			changed += " R: ";
			changed += RotationX;
			changed += " ";
			changed += RotationY;
			changed += " ";
			changed += RotationZ;
			
		//}
		//if (name == "s" || name == "sx" || name == "sy" || name == "sz")
		//{

			transNode.getScale(scale);
			

			changed += " S: ";
			changed += scale[0];
			changed += " ";
			changed += scale[1];
			changed += " ";
			changed += scale[2];
			MGlobal::displayInfo(changed);
		//}


			// send TRS data

			Translation nodeTransform;

			nodeTransform.TypeHeader = MsgType::TRANSFORM_NODE_TRANSFORM;
			nodeTransform.Tx = TranslationX;
			nodeTransform.Ty = TranslationY;
			nodeTransform.Tz = TranslationZ;

			memcpy(Message, &nodeTransform, sizeof(Translation));

			Comlib->send(Message, sizeof(Translation));

	}
	else
	{
		MGlobal::displayInfo("failed to make a transform");
		MGlobal::displayError(status.errorString());
	}
}



void AttrChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void* clientData)
{
	if (msg & MNodeMessage::AttributeMessage::kAttributeSet)
	{


		/////////////////	MATERIAL	///////////////////
		if (plug.node().hasFn(MFn::kLambert))
		{
			MStatus status;
			MGlobal::displayInfo(plug.name());
			MGlobal::displayInfo("Material attribute changed");
			MGlobal::displayInfo(plug.node().apiTypeStr());
			MFnLambertShader MyLambert(plug.node());

			
			MGlobal::displayInfo(MyLambert.absoluteName());

			MColor transp = MyLambert.transparency();
			MColor incan = MyLambert.incandescence();
			MColor diffuse = MyLambert.diffuseCoeff(); // ??
			MColor myColor = MyLambert.color();
			MString print;
			MPlugArray myPlugs;
			MPlug myPlug = MyLambert.findPlug("color");
			MPlug outColorPlug;
			myPlug.connectedTo(myPlugs, true, false);

			if (myPlugs.length()>0)
			{

				for (size_t i = 0; i < myPlugs.length(); i++)
				{
					MGlobal::displayInfo("myPlug connected plugs: ");
					MGlobal::displayInfo(myPlugs[i].name());
					outColorPlug = myPlugs[i];
					MGlobal::displayInfo(outColorPlug.node().apiTypeStr());
				}
			
				MFnDependencyNode textureNode = outColorPlug.node();
				MPlug PathPlug = textureNode.findPlug("fileTextureName");

				MString texturename;
				PathPlug.getValue(texturename);
				MGlobal::displayInfo("Texture name: ");
				MGlobal::displayInfo(texturename);
			}
			
			

			/*print += myColor.r;
			print += " ";
			print += myColor.g;
			print += " ";
			print += myColor.b;
			print += " ";
			print += myColor.a;*/

			/*print += transp.r;
			print += " ";
			print += transp.g;
			print += " ";
			print += transp.b;
			print += " ";
			print += transp.a;*/

			/*print += incan.r;
			print += " ";
			print += incan.g;
			print += " ";
			print += incan.b;
			print += " ";
			print += incan.a;*/

			/*print += diffuse.r;
			print += " ";
			print += diffuse.g;
			print += " ";
			print += diffuse.b;
			print += " ";
			print += diffuse.a;*/

			//MGlobal::displayInfo(print);
		}
		MStatus status;
		
		if (plug.node().apiType() != MFn::Type::kCamera)
		{
			MGlobal::displayInfo(plug.parent().partialName(false,false,false,false,true,false,NULL));
		
			MGlobal::displayInfo(plug.name());

			MGlobal::displayInfo(plug.partialName());

			MGlobal::displayInfo(plug.node().apiTypeStr());
		}

		/////////////////	CAMERA	///////////////////
		if (plug.node().apiType() == MFn::Type::kCamera)
		{
			MGlobal::displayInfo("Camera attribute changed");
			MGlobal::displayInfo(plug.name());
			MGlobal::displayInfo(plug.node().apiTypeStr());
		}

		/////////////////	TRANSFORM	///////////////////
		if (plug.node().apiType() == MFn::Type::kTransform)
		{
			
			MFnTransform myTrans(plug.node());
			MFnDagNode myNode(plug.node());

			if (myTrans.name() == "persp")
			{
				//recursive with camera
				recursiveTransform(myNode, true);
			}
			else
			{

				recursiveTransform(myNode,false);
			}
			
		}


		/////////////////	MESH	///////////////////
		else if (plug.node().apiType() == MFn::Type::kMesh)
		{

			

				MFnAttribute myAttr = plug.attribute();
				MString attributeName = myAttr.name();

				if (attributeName == "pnts")
				{
					MFnMesh myMesh(plug.node(), &status);

					//MObjectArray Shaders;
					//MIntArray myInts;
					//myMesh.getConnectedShaders(0, Shaders, myInts);
					//for (size_t i = 0; i < Shaders.length(); i++)
					//{
					//	MPlug plug;
					//	MPlugArray connections;
					//	MFnDependencyNode shaderGroup(Shaders[i]);
					//	MString shaderName = shaderGroup.absoluteName();
					//	MColor myColor;
					//	MPlug shaderPlug = shaderGroup.findPlug("surfaceShader");
					//	
					//	shaderPlug.connectedTo(connections, true, false);
					//	
					//	for (size_t l = 0; l < connections.length(); l++)
					//	{
					//		MGlobal::displayInfo(connections[l].name());
					//		if (connections[l].node().hasFn(MFn::kLambert))
					//		{
					//			MGlobal::displayInfo("Lambert");
					//			MFnLambertShader lambShader(connections[l].node());
					//			
					//			MColor transp = lambShader.transparency();
					//			MColor incan = lambShader.incandescence();
					//			MColor diffuse = lambShader.diffuseCoeff(); // ??
					//			myColor = lambShader.color();
					//			MString print;
					//			print += myColor.r;
					//			print += " ";
					//			print += myColor.g;
					//			print += " ";
					//			print += myColor.b;
					//			print += " ";
					//			print += myColor.a;
					//			
					//			MGlobal::displayInfo(print);
					//			MPlugArray colorPlug;
					//			lambShader.findPlug("color").connectedTo(colorPlug, true, false);
					//			
					//			for (size_t p = 0; p < colorPlug.length(); p++)
					//			{
					//				MGlobal::displayInfo("Texture: ");
					//				MGlobal::displayInfo(colorPlug[p].name());
					//				
					//			}
					//
					//		}
					//		
					//		
					//	}
					//
					//	MObject material;
					//	MStatus check;
					//
					//	MString color("color");
					//
					//	MFnDependencyNode fnDependNode(material);
					//
					//	MPlug myPlug = fnDependNode.findPlug(color,check);
					//	if (check == MS::kSuccess)
					//	{
					//		MString print("myPlug: ");
					//		print += myPlug.name();
					//		MGlobal::displayInfo(print);
					//
					//		MPlugArray cc;
					//		plug.connectedTo(cc, true, false);
					//
					//		if (cc.length() > 0)
					//		{
					//			MGlobal::displayInfo(cc[0].name());
					//			MObject src = cc[0];
					//			if (src.hasFn(MFn::kFileTexture))
					//			{
					//				MFnDependencyNode fnFile(src);
					//
					//			}
					//		}
					//
					//	}
					//
					//
					//	
					//
					//	for (size_t j = 0; j < connections.length(); j++)
					//	{
					//		if (connections[j].node().hasFn(MFn::kLambert))
					//		{
					//			MPlugArray plugs;
					//			MFnLambertShader lamberShader(connections[j].node());
					//			lamberShader.findPlug("color").connectedTo(plugs, true, false);
					//
					//			for (size_t u = 0; u < plugs.length(); u++)
					//			{
					//				MGlobal::displayInfo(plugs[u].name());
					//			}
					//		}
					//	}
					//}

					
					
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
				else if (plug.partialName() == "pv")
				{

					MFnMesh myMesh(plug.node(), &status);
					// UV stuff;
					MFloatArray UArr;
					MFloatArray VArr;

					myMesh.getUVs(UArr, VArr);

					for (uint i = 0; i < UArr.length(); i++)
					{
						MString msg;
						msg += "U array at: ";
						msg += "[";
						msg += i;
						msg += "]";
						msg += " ";
						msg += UArr[i];
						MGlobal::displayInfo(msg);
					}
					for (uint j = 0; j < VArr.length(); j++)
					{
						MString msg;
						msg += "V array at: ";
						msg += "[";
						msg += j;
						msg += "]";
						msg += " ";
						msg += VArr[j];
						MGlobal::displayInfo(msg);
					}


				}
			
		}

		/////////////////	POINTLIGHTS		///////////////////
		if (plug.node().apiType() == MFn::kPointLight)
		{
			MGlobal::displayInfo("Light Attribute Changed!");
		}



	}

}

void nodeAdded(MObject &node, void* clientData)
{

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