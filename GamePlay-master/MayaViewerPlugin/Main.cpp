#include "maya_includes .h"
#include "ComlibMaya.h"
#include <iostream>
#include "MayaShared.h"
#include <sstream>
#include <assert.h>
using namespace std;

MCallbackIdArray myCallbackArray;
MCallbackId meshCreatedId;
ComlibMaya* Comlib;

char *Message;

size_t length;
void timerCallback(float elapsedTime, float lastTime, void* clientData)
{
	MString msg("Elapsed time: ");
	msg += elapsedTime;
	//	MGlobal::displayInfo(msg);
}

void CameraViewCallback(const MString &str, void* clientData)
{
	M3dView view;
	MGlobal::displayInfo("ViewCallBack");

	MStatus status = MS::kSuccess;

	status = M3dView::getM3dViewFromModelPanel(str, view);
	if (status == MS::kSuccess)
	{
		MMatrix viewMatrix;
		MMatrix cameraPos;
		MDagPath camera;
		MString msg;
		view.modelViewMatrix(viewMatrix);



		view.getCamera(camera);
		cameraPos = camera.inclusiveMatrix();
		

		float fViewMatrix[4][4];



		for (size_t i = 0; i < 4; i++)
		{

			// i on 4 is translation (position) of the camera
			for (size_t j = 0; j < 4; j++)
			{

				fViewMatrix[i][j] = cameraPos[i][j];
				/*msg += cameraPos.matrix[i][j];
				msg += " ";*/
			}
		/*	msg += "\n";*/
		}

		/*MGlobal::displayInfo(msg);*/
		Camera myCamera;

		myCamera.headerType = MsgType::CAMERA_UPDATE;
		for (size_t i = 0; i < 4; i++)
		{
			for (size_t j = 0; j < 4; j++)
			{
				myCamera.fViewMatrix[i][j] = fViewMatrix[i][j];
			}
		}
		MFnCamera MyCam(camera.node(),&status);
		if (status == MS::kSuccess)
		{
			
		}

		string CamName = MyCam.name().asChar();
		strncpy(myCamera.name, CamName.c_str(), sizeof(myCamera.name));
		myCamera.name[sizeof(myCamera.name) - 1] = 0;

		
		size_t cpySize = 0;
		cpySize = sizeof(myCamera);;
		memcpy(Message, &myCamera, cpySize);
		


		while (true)
		{
			if (Comlib->send(Message,sizeof(myCamera)))
			{
				break;
			}
		}
	}


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
MIntArray GetLocalIndex(MIntArray &getVertices, MIntArray &getTriangle)
{
    MIntArray localIndex;
    int gv, gt;

    assert(getTriangle.length() == 3);

    for (gt = 0; gt < getTriangle.length(); gt++)
    {
        for (gv = 0; gv  < getVertices.length(); gv ++)
        {
            if (getTriangle[gt] == getVertices[gv])
            {
                localIndex.append(gv);
                break;
            }
        }
        if (localIndex.length() == gt)
        {
            localIndex.append(-1);
        }
    }
    return localIndex;
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
		vector<int> normalIndices;
		vector<Normal> meshNormals;
		vector<int> vtxIndices;
		vector<Vertex> vertices;

		MIntArray meshTris;
		MIntArray meshTriVerts;
		newMesh.getTriangles(meshTris, meshTriVerts);

		MItMeshPolygon meshIterator(newMesh.object(), &status);
		if (status == MS::kSuccess)
		{
			MGlobal::displayInfo("Succeeded creating MItMeshPolygod object!");
		}
		int triCount = 0;

		for (; !meshIterator.isDone(); meshIterator.next())
		{
            
        
            MIntArray polyVertexIndex;
            meshIterator.getVertices(polyVertexIndex);//Polygon vertex indices

			MPointArray triPoints;
			MIntArray vtxTriIdx;
			meshIterator.numTriangles(triCount);
			meshIterator.getTriangles(triPoints, vtxTriIdx);//Triangle vertices and vertex indices
			int triLength = vtxTriIdx.length();
         //   cerr << "Number of vertices in face: " << triPoints.length() << endl;
			Vertex vertex;
		
			if (vtxTriIdx.length() > 0)
			{
				int faceVertexIndex[1000];// = new int[vtxTriIdx.length()];
				vtxTriIdx.get(faceVertexIndex);
				for (size_t i = 0; i < vtxTriIdx.length(); i++)//loops through each vertex index
				{
			//		cerr << "Triangle face vertex index: " << faceVertexIndex[i] << endl;
					vtxIndices.push_back(faceVertexIndex[i]);
				}
				//delete[] faceVertexIndex;
			}

			if (triCount > 1)
			{	
                MIntArray *localIndex = new MIntArray[triCount];

                MIntArray *faceTriangle = new MIntArray[triCount];
                
				for (size_t i = 0; i < triCount; i++)
				{
					faceTriangle[i].setLength(3);
					
				}

                int indexCount = 0;//Keep track of what index to store inside the faceTriangle array
                for (size_t i = 0; i < triCount; i++)
                {
                    faceTriangle[i][0] = vtxTriIdx[indexCount];
                    indexCount++;
                    faceTriangle[i][1] = vtxTriIdx[indexCount];
                    indexCount++;
                    faceTriangle[i][2] = vtxTriIdx[indexCount];
                }
				for (size_t i = 0; i < triCount; i++)//loops through each triangle in the current face
				{
                    localIndex[i] = GetLocalIndex(polyVertexIndex, faceTriangle[i]);
              //      cerr << "Local index length: " << localIndex[i].length() << endl;

					int localIndexArray[1000];// = new int[localIndex[i].length()];
                    localIndex[i].get(localIndexArray);

                    for (size_t i = 0; i < 3; i++)//loops through every vertex in the triangle
                    { 
                        int currentIndex = -1;
                       currentIndex = meshIterator.normalIndex(localIndexArray[i]);
               //        cerr << "Current normal index " << currentIndex << endl;
                       normalIndices.push_back(currentIndex);
                    }
				}
				delete[] localIndex;
				delete[] faceTriangle;
			}
			if (triCount == 1)
			{
                MIntArray localIndex;
                MIntArray faceTriangle;
                faceTriangle.setLength(3);
                faceTriangle[0] = vtxTriIdx[0];
                faceTriangle[1] = vtxTriIdx[1];
                faceTriangle[2] = vtxTriIdx[2];

                localIndex = GetLocalIndex(polyVertexIndex, faceTriangle);
				int localIndexArray[100]; //new int[localIndex.length()];
                localIndex.get(localIndexArray);

                for (size_t i = 0; i < 3; i++)//loops through each vertex in the triangle
                {
                    int currentIndex = -1;
                    currentIndex = meshIterator.normalIndex(localIndexArray[i]);
                    normalIndices.push_back(currentIndex);
          //          cerr << "Current normal index: " << currentIndex << endl;
                }
              //  delete[] localIndexArray;
			}
           
		}

		MayaMesh createdMesh;
		createdMesh.headerType = MsgType::CREATE_MESH;
		
		createdMesh.sizeOfVtxIndex = vtxIndices.size();

		//Getting name for the transform node

		string meshName = newMesh.name().asChar();
		strncpy(createdMesh.name, meshName.c_str(), sizeof(createdMesh.name));
		createdMesh.name[sizeof(createdMesh.name) - 1] = 0;
		
		//Getting vertex data
		MFloatPointArray vtxArray;
		newMesh.getPoints(vtxArray);
		//cerr << "Vertex count: " << vtxArray.length() << endl;
		for (size_t i = 0; i < vtxArray.length(); i++) 
		{
			Vertex vtx;
			vtx.position[0] = vtxArray[i].x;
			vtx.position[1] = vtxArray[i].y;
			vtx.position[2] = vtxArray[i].z;
			vertices.push_back(vtx);
	//		cerr << "Vtx: [" << i << "] X: " << vtx.position[0] << " Y: " << vtx.position[1] << " Z: " << vtx.position[2] << endl;	
		}
		createdMesh.sizeOfVertices = vertices.size(); // getting the size of the vertices vector and storing it in sizeOfVertices to be able to read vector in comlib

													  //Getting normal data
		MFloatVectorArray normals;
		newMesh.getNormals(normals);
		//cerr << "Size of normal arr: " << normals.length() << endl; 
		for (size_t i = 0; i < normals.length(); i++)
		{
			Normal meshNormal;
			float arr[3];
			normals[i].get(arr);
//			cerr << "Normals: X: " << arr[0] << " Y: " << arr[1] << " Z: " << arr[2] << endl;
			meshNormal.normal[0] = arr[0];
			meshNormal.normal[1] = arr[1];
			meshNormal.normal[2] = arr[2];
			meshNormals.push_back(meshNormal);
		}
		createdMesh.sizeOfNormals = meshNormals.size();
		createdMesh.sizeOfNormalIndex = normalIndices.size();
		//**** Send mesh to gameplay
		//Header data
		int floatSize = sizeof(float);
		size_t cpySize = 0;
		size_t head = 0;
		cpySize = sizeof(createdMesh);
		memcpy(Message, &createdMesh, cpySize);
		head += cpySize;

		//Vertex data
		cpySize = (sizeof(int) * vtxIndices.size());
		memcpy(Message + head, vtxIndices.data(), cpySize);
		head += cpySize;

		cpySize = (sizeof(Vertex) * vertices.size());
		memcpy(Message + head, vertices.data(), cpySize);
		head += cpySize;

		//Normal data
		cpySize = normalIndices.size() * sizeof(int);
		memcpy(Message + head, normalIndices.data(), cpySize);
		head += cpySize;

		cpySize = meshNormals.size() * sizeof(Normal);
		memcpy(Message + head, meshNormals.data(), cpySize);
		head += cpySize;

		Comlib->send(Message, head);

		//*******************************
		vertices.clear();
		vtxIndices.clear();
		meshNormals.clear();
		normalIndices.clear();
		MMessage::removeCallback(meshCreatedId);//Removes the callback
	/*	delete[] indexArray;*/

	}
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
			recursiveTransform(childDag, cameraTransform);
		}
	}

	if (status == MS::kSuccess)
	{

		//MString name = plug.partialName();
		double scale[3] = { 0,0,0 };
		float fScale[3] = { 0, 0, 0 };
		double RotationX, RotationY, RotationZ, RotationW;
		float Rx, Ry, Rz, Rw;
		float TranslationX, TranslationY, TranslationZ;

		//if (name == "t" || name == "tx" || name == "ty" || name == "tz")
		//{

		MString changed;

		TranslationX = (float)transNode.getTranslation(MSpace::kWorld).x;
		TranslationY = (float)transNode.getTranslation(MSpace::kWorld).y;
		TranslationZ = (float)transNode.getTranslation(MSpace::kWorld).z;

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
		Rx = RotationX;
		Ry = RotationY;
		Rz = RotationZ;
		Rw = RotationW;

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
		for (size_t i = 0; i < 3; i++)
		{
			fScale[i] = scale[i];
		}

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
			

		MDagPath tempPath;
		Parent.getPath(tempPath);
		tempPath.extendToShape();//Getting the shape node

		string mName = tempPath.partialPathName().asChar();
		strncpy(nodeTransform.name, mName.c_str(), sizeof(nodeTransform.name));
		nodeTransform.name[sizeof(nodeTransform.name) - 1] = 0;

		memcpy(Message, &nodeTransform, sizeof(Translation));

		while (true)
		{

			if (Comlib->send(Message, sizeof(Translation)))
			{
				break;
			}

		}

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
			MGlobal::displayInfo(plug.parent().partialName(false, false, false, false, true, false, NULL));

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
				//recursiveTransform(myNode, true);
			}
			else
			{

				recursiveTransform(myNode, false);
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

	MCallbackId CameraviewID = MUiMessage::add3dViewPostRenderMsgCallback(
		"modelPanel4",
		CameraViewCallback,
		NULL,
		&status);

	if (status == MS::kSuccess)
	{
		if (myCallbackArray.append(CameraviewID) == MS::kSuccess)
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