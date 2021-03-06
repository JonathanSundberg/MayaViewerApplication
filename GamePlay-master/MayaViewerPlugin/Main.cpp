#include "maya_includes .h"
#include "ComlibMaya.h"
#include <iostream>
#include "MayaShared.h"
#include <sstream>
#include <assert.h>
#include <time.h>
#include <vector>
using namespace std;

MCallbackIdArray myCallbackArray;
MCallbackId meshCreatedId;
ComlibMaya* Comlib;

char *Message;

size_t length;

/*		ALL GLOBAL CONTAINERS AND BOOLS		*/	

// CAMERA
MCamera fpsLockCamera;
clock_t camClock;
bool CamInfo = false;


// TRS
TransformData FpsLockTRS;
clock_t TRSclock;
bool TRSInfo = false;
std::vector<TransformData> TRSVec;


// VERTICES
struct storeMeshData
{
	MayaMesh createdMeshFPSLOCK;
	vector<int> normalIndicesFPSLOCK;
	vector<Normal> meshNormalsFPSLOCK;
	vector<int> vtxIndicesFPSLOCK;
	vector<Vertex> verticesFPSLOCK;
	vector<int> UVIndexFPSLOCK;
	vector<UV> UVsFPSLOCK;
};
vector<storeMeshData> VertexStorage;



struct matrix
{
	matrix(): tx(1), ty(1),tz(1),rx(1),ry(1),rz(1),sx(1),sy(1),sz(1) {}
	float 
		tx, ty, tz,
		rx, ry, rz,
		sx, sy, sz;
};
string GetMeshMat(MFnMesh Mesh, MColor& meshColor);

// Locks chosen package indesired FPS.
void SixtyFpsLock(float elapsedTime, float lastTime, void* clientData)
{
	if (CamInfo)
	{
		size_t cpySize = 0;
		cpySize = sizeof(fpsLockCamera);
		memcpy(Message, &fpsLockCamera, cpySize);

		while (true)
		{
			if (Comlib->send(Message, sizeof(fpsLockCamera)))
			{
				break;
			}
		}
		CamInfo = false;
	}

	
	
	
}
void ThirtyFpsLock(float elapsedTime, float lastTime, void* clientData)
{
	
	
	if (VertexStorage.size()>0)
	{
		for (auto& vertexData:VertexStorage)
		{
			int floatSize = sizeof(float);
			size_t cpySize = 0;
			size_t head = 0;
			cpySize = sizeof(vertexData.createdMeshFPSLOCK);
			memcpy(Message, &vertexData.createdMeshFPSLOCK, cpySize);
			head += cpySize;

			//Vertex data
			cpySize = (sizeof(int) * vertexData.vtxIndicesFPSLOCK.size());
			memcpy(Message + head, vertexData.vtxIndicesFPSLOCK.data(), cpySize);
			head += cpySize;

			cpySize = (sizeof(Vertex) * vertexData.verticesFPSLOCK.size());
			memcpy(Message + head, vertexData.verticesFPSLOCK.data(), cpySize);
			head += cpySize;

			//Normal data
			cpySize = vertexData.normalIndicesFPSLOCK.size() * sizeof(int);
			memcpy(Message + head, vertexData.normalIndicesFPSLOCK.data(), cpySize);
			head += cpySize;

			cpySize = vertexData.meshNormalsFPSLOCK.size() * sizeof(Normal);
			memcpy(Message + head, vertexData.meshNormalsFPSLOCK.data(), cpySize);
			head += cpySize;

			//UV data
			cpySize = vertexData.UVsFPSLOCK.size() * sizeof(UV);
			memcpy(Message + head, vertexData.UVsFPSLOCK.data(), cpySize);
			head += cpySize;

			cpySize = vertexData.UVIndexFPSLOCK.size() * sizeof(int);
			memcpy(Message + head, vertexData.UVIndexFPSLOCK.data(), cpySize);
			head += cpySize;

			while (true)
			{
				if (Comlib->send(Message, head))
				{
					break;
				}
			}

			
				
			vertexData.normalIndicesFPSLOCK.clear();
			vertexData.meshNormalsFPSLOCK.clear();
			vertexData.vtxIndicesFPSLOCK.clear();
			vertexData.verticesFPSLOCK.clear();
			vertexData.UVIndexFPSLOCK.clear();
			vertexData.UVsFPSLOCK.clear();
		}
		
		VertexStorage.clear();
	}
	
	if (TRSVec.size() > 0)
	{
		for (auto& TRS : TRSVec)
		{

			memcpy(Message, &TRS, sizeof(TransformData));

			while (true)
			{

				if (Comlib->send(Message, sizeof(TransformData)))
				{
					break;
				}

			}
		}
		TRSVec.clear();
	}

}
void TenFpsLock(float elapsedTime, float lastTime, void* clientData)
{
	
}

float calcAoV(float aperture, float fl)
{
	float fovCalc = aperture * 0.5 / (fl * 0.03937);
	float fov = 2.0f * atan(fovCalc) / 3.14159 * 180.0;
	return fov;
}

//	Callbacks when the camera updates, sends the perspective and TRS changes
void CameraViewCallback(const MString &str, void* clientData)
{
	M3dView view;

	MStatus status = MS::kSuccess;

   	status = M3dView::getM3dViewFromModelPanel(str, view);
	if (status == MS::kSuccess)
	{
		MMatrix viewMatrix;
 		MMatrix cameraPos;
		MDagPath camera;
		MString msg;
		view.modelViewMatrix(viewMatrix);
		double Rx, Ry, Rz, Rw;
		float fRx, fRy, fRz, fRw;

		view.getCamera(camera);
		cameraPos = camera.inclusiveMatrix();
		
		float fViewMatrix[4][4];

		for (size_t i = 0; i < 4; i++)
		{
			// i on 3 is translation (position) of the camera
			for (size_t j = 0; j < 4; j++)
			{
				fViewMatrix[i][j] = cameraPos[i][j];
				/*msg += cameraPos.matrix[i][j];
				msg += " ";*/
			}
		/*	msg += "\n";*/
		}
		/*MGlobal::displayInfo(msg);*/
		MCamera myCamera;

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
		//Getting the rotation from the transform node of the camera
		MFnTransform camTrans = MyCam.parent(0, &status);
		camTrans.getRotationQuaternion(Rx, Ry, Rz, Rw);
		fRx = Rx;
		fRy = Ry;
		fRz = Rz;
		fRw = Rw;
		myCamera.Rot[0] = fRx;
		myCamera.Rot[1] = fRy;
		myCamera.Rot[2] = fRz;
		myCamera.Rot[3] = fRw;

		//Camera attributes
		myCamera.aspectRatio = (float)MyCam.aspectRatio();
		myCamera.FOV = calcAoV((float)MyCam.horizontalFilmAperture(), (float)MyCam.focalLength());
		myCamera.isOrtho = MyCam.isOrtho();
		myCamera.farPlane = (float)MyCam.farClippingPlane();
		myCamera.nearPlane = (float)MyCam.nearClippingPlane();
		//myCamera.zoom = (float)MyCam.zoom();
		myCamera.zoom = (float)MyCam.orthoWidth();

		string CamName = "";
		if (myCamera.isOrtho)
			CamName = "cameraO";
		else
			CamName = "cameraP";
		//string CamName = MyCam.name().asChar();
		strncpy(myCamera.name, CamName.c_str(), sizeof(myCamera.name));
		myCamera.name[sizeof(myCamera.name) - 1] = 0;
		
		MVector rightVec = MyCam.rightDirection();
	

		
		fpsLockCamera = myCamera;
		clock_t timer = clock() - camClock;
	

		if (timer > 16.67)
		{

			size_t cpySize = 0;
			cpySize = sizeof(myCamera);
			memcpy(Message, &myCamera, cpySize);	

			while (true)
			{
				if (Comlib->send(Message,sizeof(myCamera)))
				{
					break;
				}
			}
			camClock = clock();
			CamInfo = false;
		}
		else
		{
			CamInfo = true;
		}
		
	}
}
//void findCamera()
//{
//	MItDag dagIterator(MItDag::kBreadthFirst, MFn::kCamera);
//
//	for (; !dagIterator.isDone(); dagIterator.next())
//	{
//		if (dagIterator.currentItem().apiType() == MFn::Type::kCamera)
//		{
//
//			MFnCamera myCam = dagIterator.currentItem();
//
//			if (myCam.name() == "perspShape")
//			{
//				MString msg = "Perspective camera: ";
//				msg += myCam.absoluteName();
//
//				//MGlobal::displayInfo(msg);
//
//
//			}
//
//
//		}
//	}
//
//}

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

//called when making a new mesh
void getNewMeshData(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void* clientData)
{
	MStatus status;
	MFnMesh newMesh(plug.node(), &status);

	if (status == MS::kSuccess)
	{
		MColor meshColor;
		string materialName = GetMeshMat(plug.node(), meshColor);

		vector<int> normalIndices;
		vector<Normal> meshNormals;
		vector<int> vtxIndices;
		vector<Vertex> vertices;
		vector<int> UVIndex;
		vector<UV> UVs;

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
			meshIterator.getTriangles(triPoints, vtxTriIdx, MSpace::kObject);//Triangle vertices and vertex indices
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

			if (triCount >= 1)
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
					indexCount++;
                }

				for (size_t i = 0; i < triCount; i++)//loops through each triangle in the current face
				{
                    localIndex[i] = GetLocalIndex(polyVertexIndex, faceTriangle[i]);
              //      cerr << "Local index length: " << localIndex[i].length() << endl;

					int localIndexArray[1000];// = new int[localIndex[i].length()];
                    localIndex[i].get(localIndexArray);
					MIntArray currentTriVtxIndices;
					MPointArray currentTriVtx;
					meshIterator.getTriangle(i, currentTriVtx, currentTriVtxIndices);
                    for (size_t i = 0; i < 3; i++)//loops through every vertex in the triangle
                    { 
						UV currentUV;
						float2 UVss;
						int uvIndex;

						int triVtxIndex[3];
						float triVtx[4];
						currentTriVtx[i].get(triVtx);
						currentTriVtxIndices.get(triVtxIndex);
						
						meshIterator.getUV(localIndexArray[i], UVss);
						meshIterator.getUVIndex(localIndexArray[i], uvIndex);

						int currentIndex = -1;
						currentIndex = meshIterator.normalIndex(localIndexArray[i]);

						currentUV.U = UVss[0];
						currentUV.V = UVss[1];
						normalIndices.push_back(currentIndex);

						UVIndex.push_back(uvIndex);
                    }
				}
				delete[] localIndex;
				delete[] faceTriangle;
			} 
		}

		MayaMesh createdMesh;
		createdMesh.headerType = MsgType::CREATE_MESH;
		
		createdMesh.sizeOfVtxIndex = vtxIndices.size();

		//Getting name for the transform node

		string meshName = newMesh.name().asChar();
		strncpy(createdMesh.name, meshName.c_str(), sizeof(createdMesh.name));
		createdMesh.name[sizeof(createdMesh.name) - 1] = 0;
		
		//UVs
		MFloatArray uArr;
		MFloatArray vArr;
		newMesh.getUVs(uArr, vArr);
		int nrOfUvs = newMesh.numUVs();
		float *uArray = new float[nrOfUvs];
		float * vArray = new float[nrOfUvs];
		uArr.get(uArray);
		vArr.get(vArray);
		
		for (size_t i = 0; i < nrOfUvs; i++)
		{
			UV currentUV;
			currentUV.U = uArray[i];
			currentUV.V = vArray[i];
			cerr << "U[" << currentUV.U << "]" << " V[" << currentUV.V << endl;
			UVs.push_back(currentUV);
		}


		//Getting vertex data
		MFloatPointArray vtxArray;
		newMesh.getPoints(vtxArray, MSpace::kObject);
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
		newMesh.getNormals(normals, MSpace::kObject);
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
		createdMesh.sizeOfUV = UVs.size();
		createdMesh.sizeOfUVIndex = UVIndex.size();
		strncpy(createdMesh.materialName, materialName.c_str(), sizeof(createdMesh.materialName));
		createdMesh.materialName[sizeof(createdMesh.materialName) - 1] = 0;
		
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

		//UV data
		cpySize = UVs.size() * sizeof(UV);
		memcpy(Message + head, UVs.data(), cpySize);
		head += cpySize;

		cpySize = UVIndex.size() * sizeof(int);
		memcpy(Message + head, UVIndex.data(), cpySize);
		head += cpySize;


		Comlib->send(Message, head);

		//Sending the transformdata after vertex translation to stop it from snapping to origo
		MObject tempObj(newMesh.object());
		MFnDagNode TransformNodeMesh(tempObj);
		//recursiveTransform(TransformNodeMesh, false);

		//*******************************
		vertices.clear();
		vtxIndices.clear();
		meshNormals.clear();
		normalIndices.clear();
		UVIndex.clear();
		UVs.clear();
		MMessage::removeCallback(meshCreatedId);//Removes the callback
	/*	delete[] indexArray;*/

	}
}
//Callback for getting meshdata on creating new mesh
void childAdded(MDagPath &child, MDagPath &parent, void* clientData)
{
	MStatus status;
	if (child.node().apiType() == MFn::kMesh)
	{
		meshCreatedId = MNodeMessage::addAttributeChangedCallback(
			child.node(),
			getNewMeshData,
			NULL,
			&status
		);
	}
}

//	Gathers the TRS from a mesh and sends it, Scale is made into a matrix and sent down the heirarchy and multiplied since it cannot be obtained in world coordinates.
void recursiveTransform(MFnDagNode& Parent, bool topParent, matrix parentMatrix = matrix())
{

	if (Parent.child(0).apiType() == MFn::kCamera)
	{
		return;
	}
	MStatus status;
	//MGlobal::displayInfo("Entered rekursive transform");

	MDagPath path;

	Parent.getPath(path);

	MFnTransform transNode(path, &status);


	

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
	//	MGlobal::displayInfo(changed);
		//}

		// send TRS data

		TransformData nodeTransform;

		nodeTransform.TypeHeader = MsgType::TRANSFORM_NODE_TRANSFORM;
		nodeTransform.Tx = TranslationX;
		nodeTransform.Ty = TranslationY;
		nodeTransform.Tz = TranslationZ;
		nodeTransform.Rx = RotationX;
		nodeTransform.Ry = RotationY;
		nodeTransform.Rz = RotationZ;
		nodeTransform.Rw = RotationW;
		nodeTransform.Sx = fScale[0];
		nodeTransform.Sy = fScale[1];
		nodeTransform.Sz = fScale[2];

		matrix scaleMatrix;
		scaleMatrix.tx = nodeTransform.Sx;
		scaleMatrix.ty = 0;
		scaleMatrix.tz = 0;
		scaleMatrix.rx = 0;
		scaleMatrix.ry = nodeTransform.Sy;
		scaleMatrix.rz = 0;
		scaleMatrix.sx = 0;
		scaleMatrix.sy = 0;
		scaleMatrix.sz = nodeTransform.Sz;

		matrix sendMatrix;
		if (!topParent)
		{
			sendMatrix.tx = parentMatrix.tx*scaleMatrix.tx + parentMatrix.ty*scaleMatrix.rx + parentMatrix.tz*scaleMatrix.sx;
			sendMatrix.ty = parentMatrix.tx*scaleMatrix.ty + parentMatrix.ty*scaleMatrix.ry + parentMatrix.tz*scaleMatrix.sy;
			sendMatrix.tz = parentMatrix.tx*scaleMatrix.tz + parentMatrix.ty*scaleMatrix.rz + parentMatrix.tz*scaleMatrix.sz;

			sendMatrix.rx = parentMatrix.rx*scaleMatrix.tx + parentMatrix.ry*scaleMatrix.rx + parentMatrix.rz*scaleMatrix.sx;
			sendMatrix.ry = parentMatrix.rx*scaleMatrix.ty + parentMatrix.ry*scaleMatrix.ry + parentMatrix.rz*scaleMatrix.sy;
			sendMatrix.rz = parentMatrix.rx*scaleMatrix.tz + parentMatrix.ry*scaleMatrix.rz + parentMatrix.rz*scaleMatrix.sz;

			sendMatrix.sx = parentMatrix.sx*scaleMatrix.tx + parentMatrix.sy*scaleMatrix.rx + parentMatrix.sz*scaleMatrix.sx;
			sendMatrix.sy = parentMatrix.sx*scaleMatrix.ty + parentMatrix.sy*scaleMatrix.ry + parentMatrix.sz*scaleMatrix.sy;
			sendMatrix.sz = parentMatrix.sx*scaleMatrix.tz + parentMatrix.sy*scaleMatrix.rz + parentMatrix.sz*scaleMatrix.sz;

		}
		else
		{
			sendMatrix = scaleMatrix;
		}
		
		nodeTransform.Sx = sendMatrix.tx;
		nodeTransform.Sy = sendMatrix.ry;
		nodeTransform.Sz = sendMatrix.sz;


		MString scaleInfo;
		scaleInfo += " S: ";
		scaleInfo += nodeTransform.Sx;
		scaleInfo += " ";
		scaleInfo += nodeTransform.Sy;
		scaleInfo += " ";
		scaleInfo += nodeTransform.Sz;

		MGlobal::displayInfo(scaleInfo);


		for (size_t i = 0; i < Parent.childCount(); i++)
		{

			if (Parent.child(i).apiType() == MFn::Type::kTransform)
			{
				MFnDagNode childDag = Parent.child(i);
				recursiveTransform(childDag,false,sendMatrix);
			}
		}


		MDagPath tempPath;
		Parent.getPath(tempPath);
		tempPath.extendToShape();//Getting the shape node

		string mName = tempPath.partialPathName().asChar();
		strncpy(nodeTransform.name, mName.c_str(), sizeof(nodeTransform.name));
		nodeTransform.name[sizeof(nodeTransform.name) - 1] = 0;



		bool found = false;
		for (auto& TRS: TRSVec)
		{
			if (strcmp(nodeTransform.name,TRS.name) == 0)
			{
				TRS = nodeTransform;
				found = true;
			}
			
		}
		if (!found)
		{
			TRSVec.push_back(nodeTransform);
		}


		
		
		/*memcpy(Message, &nodeTransform, sizeof(TransformData));

		while (true)
		{

			if (Comlib->send(Message, sizeof(TransformData)))
			{
				break;
			}

		}*/

		


	}
	else
	{
		MGlobal::displayInfo("failed to make a transform");
		MGlobal::displayError(status.errorString());
	}
}

//	Checks if a mesh has any parents with a transform, if so we go up as far as we can in the heirarchy before gathering and sending the TRS, this is  for the scale matrix multiplication.
void GetParent(MFnDagNode& parent)
{
	for (int i = 0; i < parent.parentCount(); i++)
	{


		if (parent.parent(i).apiType() == MFn::Type::kTransform)
		{
			MFnDagNode ParentDag = parent.parent(i);
			GetParent(ParentDag);
			return;
		}
	}
	recursiveTransform(parent, true);
}

//	Collects the material name and color from a mesh.
string GetMeshMat(MFnMesh Mesh,MColor& meshColor)
{
	MObjectArray Shaders;
	MIntArray myInts;

	Mesh.getConnectedShaders(0, Shaders, myInts);

	for (size_t i = 0; i < Shaders.length(); i++)
	{
		MPlug plug;
		MPlugArray connections;

		MFnDependencyNode shaderGroup(Shaders[i]);

		MString shaderName = shaderGroup.absoluteName();
		MColor myColor;
		MPlug shaderPlug = shaderGroup.findPlug("surfaceShader");

		shaderPlug.connectedTo(connections, true, false);
		for (size_t j = 0; j < connections.length(); j++)
		{
			
			if (connections[j].node().hasFn(MFn::kLambert))
			{
				MFnLambertShader myLambert(connections[j].node());
				MGlobal::displayInfo("Material name: ");
				MGlobal::displayInfo(myLambert.name());
				string name = myLambert.name().asChar();

				meshColor = myLambert.color();
				return name;
			}
		}
	}
}

// Gets the mesh(es?) connected to a specific material, probobly never used.
string GetMeshFromMat(MFnLambertShader myLambert)
{
	string name;
	MPlugArray plugArray;
	myLambert.getConnections(plugArray);

	for (size_t j = 0; j < plugArray.length(); j++)		// outColor or Message
	{
		//MGlobal::displayInfo(plugArray[j].name());
		MPlugArray connections;
		plugArray[j].connectedTo(connections, true, true);
		for (size_t i = 0; i < connections.length(); i++)
		{
			//MGlobal::displayInfo(connections[i].name());
			MFnDependencyNode myNode(connections[i].node());
			MPlugArray secondArray;

			myNode.getConnections(secondArray);
			for (size_t l = 0; l < secondArray.length(); l++)
			{
				//MGlobal::displayInfo(secondArray[l].name());
				MFnAttribute myAttr = secondArray[l].attribute();
				MString attributeName = myAttr.name();
				//MGlobal::displayInfo(attributeName);
				if (attributeName == "dagSetMembers")
				{
					MPlugArray finalArray;
					MPlug myPlug = secondArray[l];
					myPlug.connectedTo(finalArray, true, true);
					for (size_t h = 0; h < finalArray.length(); h++)
					{
						//MGlobal::displayInfo(finalArray[h].name());
						MStatus status;
						MFnMesh myMesh(finalArray[h].node(), &status);
						if (status)
						{
							//MGlobal::displayInfo(myMesh.name());
							string name = myMesh.name().asChar();
							return name;
						}
					}
				}
				for (size_t u = 0; u < secondArray.length(); u++)
				{
					MPlugArray thirdArray;
					secondArray[u].connectedTo(thirdArray,true,true);
				//	MGlobal::displayInfo(thirdArray[u].name());
				}
			}
		
			MPlugArray meshConnections;
			secondArray[i].connectedTo(meshConnections, true, true);
			for (size_t p = 0; p < meshConnections.length(); p++)
			{
				//MGlobal::displayInfo(meshConnections[p].name());
			}
		
			//MGlobal::displayInfo(connections[i].name());
			name = connections[i].name().asChar();
		}
	}

	return name;
}
//Called when updating a mesh attribute
void updateMesh(MPlug &plug)
{
	MStatus status;
	MFnMesh newMesh(plug.node(), &status);

	if (status == MS::kSuccess)
	{
		MColor meshColor;
		string materialName = GetMeshMat(plug.node(), meshColor);

		vector<int> normalIndices;
		vector<Normal> meshNormals;
		vector<int> vtxIndices;
		vector<Vertex> vertices;
		vector<int> UVIndex;
		vector<UV> UVs;

		MIntArray meshTris;
		MIntArray meshTriVerts;
		newMesh.getTriangles(meshTris, meshTriVerts);
		
		MItMeshPolygon meshIterator(newMesh.object(), &status);
		if (status == MS::kSuccess)
		{
			
		}
		int triCount = 0;

		for (; !meshIterator.isDone(); meshIterator.next())
		{
			MIntArray polyVertexIndex;
			meshIterator.getVertices(polyVertexIndex);//Polygon vertex indices

			MPointArray triPoints;
			MIntArray vtxTriIdx;
			meshIterator.numTriangles(triCount);
			meshIterator.getTriangles(triPoints, vtxTriIdx, MSpace::kObject);//Triangle vertices and vertex indices
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

			if (triCount >= 1)
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
					indexCount++;
				}
				for (size_t i = 0; i < triCount; i++)//loops through each triangle in the current face
				{
					localIndex[i] = GetLocalIndex(polyVertexIndex, faceTriangle[i]);
					//      cerr << "Local index length: " << localIndex[i].length() << endl;

					int localIndexArray[1000];// = new int[localIndex[i].length()];
					localIndex[i].get(localIndexArray);

					for (size_t i = 0; i < 3; i++)//loops through every vertex in the triangle
					{
						UV currentUV;
						float2 UVss;
						int uvIndex;
						meshIterator.getUV(localIndexArray[i], UVss);
						meshIterator.getUVIndex(localIndexArray[i], uvIndex);
						
						int currentIndex = -1;
						currentIndex = meshIterator.normalIndex(localIndexArray[i]);
						//        cerr << "Current normal index " << currentIndex << endl;
						currentUV.U = UVss[0];
						currentUV.V = UVss[1];
						normalIndices.push_back(currentIndex);
						UVIndex.push_back(uvIndex);					
					}
				}
				delete[] localIndex;
				delete[] faceTriangle;
			}
			//if (triCount == 1)
			//{
			//	MIntArray localIndex;
			//	MIntArray faceTriangle;
			//	faceTriangle.setLength(3);
			//	faceTriangle[0] = vtxTriIdx[0];
			//	faceTriangle[1] = vtxTriIdx[1];
			//	faceTriangle[2] = vtxTriIdx[2];

			//	localIndex = GetLocalIndex(polyVertexIndex, faceTriangle);
			//	int localIndexArray[100]; //new int[localIndex.length()];
			//	localIndex.get(localIndexArray);

			//	for (size_t i = 0; i < 3; i++)//loops through each vertex in the triangle
			//	{
			//		UV currentUV;
			//		float2 UVss;
			//		unsigned int uvIndex;
			//		meshIterator.getUV(localIndexArray[i], UVss);
			//		
			//		MGlobal::displayInfo("Nein");
			//		int currentIndex = -1;
			//		currentIndex = meshIterator.normalIndex(localIndexArray[i]);
			//		normalIndices.push_back(currentIndex);
			//		//          cerr << "Current normal index: " << currentIndex << endl;
			//	}
				//  delete[] localIndexArray;
			//}

		}
		MayaMesh createdMesh;
		createdMesh.headerType = MsgType::VERTEX_TRANSLATION;

		createdMesh.sizeOfVtxIndex = vtxIndices.size();

		//Getting name for the transform node

		string meshName = newMesh.name().asChar();
		strncpy(createdMesh.name, meshName.c_str(), sizeof(createdMesh.name));
		createdMesh.name[sizeof(createdMesh.name) - 1] = 0;

		MFloatArray uArr;
		MFloatArray vArr;
		newMesh.getUVs(uArr, vArr);
		int nrOfUvs = newMesh.numUVs();
		float *uArray = new float[nrOfUvs];
		float * vArray = new float[nrOfUvs];
		uArr.get(uArray);
		vArr.get(vArray);

		for (size_t i = 0; i < nrOfUvs; i++)
		{
			UV currentUV;
			currentUV.U = uArray[i];
			currentUV.V = vArray[i];
		//	cerr << "U[" << currentUV.U << "]" << " V[" << currentUV.V << endl;
			UVs.push_back(currentUV);
		}

		//Getting vertex data
		MFloatPointArray vtxArray;
		newMesh.getPoints(vtxArray, MSpace::kObject);
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
		newMesh.getNormals(normals, MSpace::kObject);
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
		createdMesh.sizeOfUV = UVs.size();
		createdMesh.sizeOfUVIndex = UVIndex.size();
		createdMesh.color[0] = meshColor.r;
		createdMesh.color[1] = meshColor.g;
		createdMesh.color[2] = meshColor.b;
		createdMesh.color[3] = meshColor.a;
		strncpy(createdMesh.materialName, materialName.c_str(), sizeof(createdMesh.materialName));
		createdMesh.materialName[sizeof(createdMesh.materialName) - 1] = 0;

		//**** Send mesh to gameplay
		//Header data


		bool found = false;
		for (auto& vertexData:VertexStorage)
		{
			if (strcmp(vertexData.createdMeshFPSLOCK.name, createdMesh.name) == 0)
			{
				vertexData.createdMeshFPSLOCK = createdMesh;
				vertexData.normalIndicesFPSLOCK = normalIndices;
				vertexData.meshNormalsFPSLOCK = meshNormals;
				vertexData.vtxIndicesFPSLOCK = vtxIndices;
				vertexData.verticesFPSLOCK = vertices;
				vertexData.UVIndexFPSLOCK = UVIndex;
				vertexData.UVsFPSLOCK = UVs;
				found = true;
				break;
			}
		}
		if (!found)
		{
			storeMeshData newSave;
			newSave.createdMeshFPSLOCK = createdMesh;
			newSave.normalIndicesFPSLOCK = normalIndices;
			newSave.meshNormalsFPSLOCK = meshNormals;
			newSave.vtxIndicesFPSLOCK = vtxIndices;
			newSave.verticesFPSLOCK = vertices;
			newSave.UVIndexFPSLOCK = UVIndex;
			newSave.UVsFPSLOCK = UVs;
			VertexStorage.push_back(newSave);
		}

		



		//int floatSize = sizeof(float);
		//size_t cpySize = 0;
		//size_t head = 0;
		//cpySize = sizeof(createdMesh);
		//memcpy(Message, &createdMesh, cpySize);
		//head += cpySize;

		////Vertex data
		//cpySize = (sizeof(int) * vtxIndices.size());
		//memcpy(Message + head, vtxIndices.data(), cpySize);
		//head += cpySize;

		//cpySize = (sizeof(Vertex) * vertices.size());
		//memcpy(Message + head, vertices.data(), cpySize);
		//head += cpySize;

		////Normal data
		//cpySize = normalIndices.size() * sizeof(int);
		//memcpy(Message + head, normalIndices.data(), cpySize);
		//head += cpySize;

		//cpySize = meshNormals.size() * sizeof(Normal);
		//memcpy(Message + head, meshNormals.data(), cpySize);
		//head += cpySize;

		////UV data
		//cpySize = UVs.size() * sizeof(UV);
		//memcpy(Message + head, UVs.data(), cpySize);
		//head += cpySize;

		//cpySize = UVIndex.size() * sizeof(int);
		//memcpy(Message + head, UVIndex.data(), cpySize);
		//head += cpySize;

		//while (true)
		//{
		//	if (Comlib->send(Message, head))
		//	{
		//		break;
		//	}
		//}
		
		
		//*******************************
		vertices.clear();
		vtxIndices.clear();
		meshNormals.clear();
		normalIndices.clear();
		UVIndex.clear();
		UVs.clear();
	}
}

//	whenever an attribute changes in the scene this is called, mostly used for mesh TRS and Vertex TRS calls, also does textures and materials.
void AttrChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void* clientData)
{

	MGlobal::displayInfo(plug.name());
	MGlobal::displayInfo(plug.info());

	

	if (msg & MNodeMessage::AttributeMessage::kAttributeSet)
	{

	

		if (plug.node().apiType() == MFn::Type::kFileTexture)
		{
			MGlobal::displayInfo(plug.name());
			
			MFnDependencyNode mf(plug.node());
						
			MGlobal::displayInfo(plug.info());
			MString value;
			plug.getValue(value);

			MGlobal::displayInfo(value);

			MPlugArray plugArray;
			string matName;
			mf.getConnections(plugArray);
			for (size_t i = 0; i < plugArray.length(); i++)
			{
				MGlobal::displayInfo(plugArray[i].name());


				MPlugArray SecondArray;
				plugArray[i].connectedTo(SecondArray, true, true);

				for (size_t j = 0; j < SecondArray.length(); j++)
				{
					if (SecondArray[j].node().hasFn(MFn::Type::kLambert))
					{
						
						MFnLambertShader lambShader(SecondArray[j].node());
						MGlobal::displayInfo(lambShader.name());
						TextureName texName;
						string matName = lambShader.name().asChar();
						strncpy(texName.matName, lambShader.name().asChar(), sizeof(texName.matName));
						texName.matName[sizeof(texName.matName) - 1] = 0;

						MPlug colorPlug = lambShader.findPlug("color");
						MPlug outCPlug;
						MPlugArray colorConnections;
						colorPlug.connectedTo(colorConnections, true, false);

						if (colorConnections.length() > 0)
						{
							for (size_t i = 0; i < colorConnections.length(); i++)
							{
								//MGlobal::displayInfo(colorConnections[i].name());
								outCPlug = colorConnections[i];
							}
							MFnDependencyNode texNode(outCPlug.node());
							MPlug texturePlug = texNode.findPlug("fileTextureName");

							MString texPath;
							texturePlug.getValue(texPath);
							MGlobal::displayInfo("texturename: " + texPath);
							string pathName = texPath.asChar();
							strncpy(texName.file, pathName.c_str(), sizeof(texName.file));
							texName.file[sizeof(texName.file) - 1] = 0;

							texName.headerType = MsgType::TEXTURE_UPDATE;

							memcpy(Message, &texName, sizeof(TextureName));
							while (true)
							{
								if (Comlib->send(Message, sizeof(TextureName)))
								{
									break;
								}
							}
						}
					//	MPlug PathPlug = textureNode.findPlug("fileTextureName");
					}
				}
				/*if (1)
				{
					MFnLambertShader lamb(plugArray[i].node());
					matName = lamb.name().asChar();
					break;
				}*/

			}
			int a = 0;
		}
		/////////////////	MATERIAL	///////////////////
		if (plug.node().hasFn(MFn::kLambert))
		{
			
			MStatus status;
			/*MGlobal::displayInfo(plug.name());
			MGlobal::displayInfo("Material attribute changed");
			MGlobal::displayInfo(plug.node().apiTypeStr());*/
			MFnLambertShader MyLambert(plug.node());
			string meshName = GetMeshFromMat(plug.node());
			//MGlobal::displayInfo(MyLambert.absoluteName());

			MPlugArray myArrray;
			MyLambert.getConnections(myArrray);
		/*	for (size_t i = 0; i < myArrray.length(); i++)
			{
				MGlobal::displayInfo(myArrray[i].name());
			}
			MGlobal::displayInfo(myArrray[1].node().apiTypeStr());*/
			MFnLambertShader anotherOne(myArrray[1].node());
			anotherOne.getConnections(myArrray);
			for (size_t i = 0; i < myArrray.length(); i++)
			{
				MGlobal::displayInfo(myArrray[i].name());
			}
			MGlobal::displayInfo(myArrray[1].node().apiTypeStr());

			MColor transp = MyLambert.transparency();
			MColor incan = MyLambert.incandescence();
			MColor diffuse = MyLambert.diffuseCoeff(); // ??
			MColor myColor = MyLambert.color();
			MString print;
			MPlugArray myPlugs;
			MPlug myPlug = MyLambert.findPlug("color");
			MPlug outColorPlug;
			myPlug.connectedTo(myPlugs, true, false);

			if (myPlugs.length() > 0)
			{

				for (size_t i = 0; i < myPlugs.length(); i++)
				{
				//	MGlobal::displayInfo("myPlug connected plugs: ");
				//	MGlobal::displayInfo(myPlugs[i].name());
					outColorPlug = myPlugs[i];
				//	MGlobal::displayInfo(outColorPlug.node().apiTypeStr());
				}
				string matName = MyLambert.name().asChar();
				MFnDependencyNode textureNode = outColorPlug.node();
				MPlug PathPlug = textureNode.findPlug("fileTextureName");

				MString texturename;
				PathPlug.getValue(texturename);

				MGlobal::displayInfo("Texture name: ");
				MGlobal::displayInfo(texturename);
				
				MatChange materialChange;

				materialChange.headerType = MsgType::MATERIAL_CHANGE;
				strncpy(materialChange.texture, texturename.asChar(), sizeof(materialChange.texture));
				materialChange.texture[sizeof(materialChange.texture) - 1] = 0;
				strncpy(materialChange.matName, matName.c_str(), sizeof(materialChange.matName));
				materialChange.matName[sizeof(materialChange.matName) - 1] = 0;
				strncpy(materialChange.meshName, meshName.c_str(), sizeof(materialChange.meshName));
				materialChange.meshName[sizeof(materialChange.meshName) - 1] = 0;

				memcpy(Message, &materialChange, sizeof(MatChange));
				while (true)
				{
					if (Comlib->send(Message, sizeof(MatChange)))
					{
						break;
					}
				}

			}
			else
			{
				Color* newColor = new Color();
				newColor->headerType = MsgType::COLOR_UPDATE;
				 newColor->colors[0] = myColor.r;
				 newColor->colors[1] = myColor.g;
				 newColor->colors[2] = myColor.b;
				 
				 strncpy(newColor->meshName, meshName.c_str(), sizeof(newColor->meshName));
				 newColor->meshName[sizeof(newColor->meshName) - 1] = 0;
				 string matName = MyLambert.name().asChar();
				 strncpy(newColor->matName, matName.c_str(), sizeof(newColor->matName));
				 newColor->matName[sizeof(newColor->matName) - 1] = 0;

				 memcpy(Message, newColor, sizeof(Color));

				 while (true)
				 {
					 if (Comlib->send(Message,sizeof(Color)))
					 {
						 break;
					 }
				 }


			}

		}
		MStatus status;

		if (plug.node().apiType() != MFn::Type::kCamera)
		{
			/*MGlobal::displayInfo(plug.parent().partialName(false, false, false, false, true, false, NULL));

			MGlobal::displayInfo(plug.name());

			MGlobal::displayInfo(plug.partialName());

			MGlobal::displayInfo(plug.node().apiTypeStr());*/
		}

		/////////////////	CAMERA	///////////////////
		if (plug.node().apiType() == MFn::Type::kCamera)
		{
	/*		MGlobal::displayInfo("Camera attribute changed");
			MGlobal::displayInfo(plug.name());
			MGlobal::displayInfo(plug.node().apiTypeStr());*/
		}

		/////////////////	TRANSFORM	///////////////////
		if (plug.node().apiType() == MFn::Type::kTransform)
		{

			MFnTransform myTrans(plug.node());
			MFnDagNode myNode(plug.node());

		
			
			//recursiveTransform(myNode,true);
			GetParent(myNode);
			
			

		}


		/////////////////	MESH	///////////////////
		else if (plug.node().apiType() == MFn::Type::kMesh)
		{
		
			
			MFnAttribute myAttr = plug.attribute();
			MString attributeName = myAttr.name();

			if (attributeName == "pnts")
			{
				updateMesh(plug);
				MFnMesh myMesh(plug.node(), &status);
				MFnDagNode dagNode = myMesh.parent(0);
				GetParent(dagNode);
				

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


						//MGlobal::displayInfo(Vertex);

					}
				}
				else
				{
				/*	MGlobal::displayInfo("failed to make a Mesh");
					MGlobal::displayError(status.errorString());*/
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
					//MGlobal::displayInfo(msg);
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
					//MGlobal::displayInfo(msg);
				}
			}
		}

		/////////////////	POINTLIGHTS		///////////////////
		if (plug.node().apiType() == MFn::kPointLight)
		{
			//MGlobal::displayInfo("Light Attribute Changed!");
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

void nodeRemoved(MObject &node, void* clientData)
{
	if (node.hasFn(MFn::kMesh))
	{
		NodeName nodeName;
		MStatus status;
		MFnMesh mesh(node, &status);
		if (status == MS::kSuccess)
		{
	/*		string meshName = newMesh.name().asChar();
			strncpy(createdMesh.name, meshName.c_str(), sizeof(createdMesh.name));
			createdMesh.name[sizeof(createdMesh.name) - 1] = 0;
*/
			nodeName.headerType = MsgType::NODE_REMOVED;
			string strName = mesh.name().asChar();
			strncpy(nodeName.name, strName.c_str(), sizeof(nodeName.name));
			nodeName.name[sizeof(nodeName.name) - 1] = 0;
			

			memcpy(Message, &nodeName, sizeof(NodeName));
			Comlib->send(Message, sizeof(NodeName));
		}
	}
}

//	when plugin is loaded this runs, initialization.
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
		0.03334,
		ThirtyFpsLock,
		NULL, &status
	);

	if (status == MS::kSuccess) {

		if (myCallbackArray.append(id) == MS::kSuccess)
		{

		}
	}

	MCallbackId sixtyFpsId = MTimerMessage::addTimerCallback
	(
		0.01667,
		SixtyFpsLock,
		NULL, &status
	);

	MCallbackId tenFpsID = MTimerMessage::addTimerCallback
	(
		0.1,
		TenFpsLock,
		NULL, &status
	);

	if (status == MS::kSuccess) {

		if (myCallbackArray.append(tenFpsID) == MS::kSuccess)
		{

		}
	}

	if (status == MS::kSuccess) {

		if (myCallbackArray.append(sixtyFpsId) == MS::kSuccess)
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
	MCallbackId removeNodeID = MDGMessage::addNodeRemovedCallback
	(
		nodeRemoved,
		"dependNode",
		NULL,
		&status
	);
	if (status == MS::kSuccess)
	{
		if (myCallbackArray.append(removeNodeID) == MS::kSuccess)
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

	MCallbackId CameraviewID1 = MUiMessage::add3dViewPostRenderMsgCallback(
		"modelPanel4",
		CameraViewCallback,
		NULL,
		&status);

	if (status == MS::kSuccess)
	{
		if (myCallbackArray.append(CameraviewID1) == MS::kSuccess)
		{
			MGlobal::displayInfo("Camera changed CALLBAAACK!!");
		}
	}

	MCallbackId CameraviewID2 = MUiMessage::add3dViewPostRenderMsgCallback(
		"modelPanel1",
		CameraViewCallback,
		NULL,
		&status);

	if (status == MS::kSuccess)
	{
		if (myCallbackArray.append(CameraviewID2) == MS::kSuccess)
		{
			MGlobal::displayInfo("Camera changed CALLBAAACK!!");
		}
	}

	MCallbackId CameraviewID3 = MUiMessage::add3dViewPostRenderMsgCallback(
		"modelPanel2",
		CameraViewCallback,
		NULL,
		&status);

	if (status == MS::kSuccess)
	{
		if (myCallbackArray.append(CameraviewID3) == MS::kSuccess)
		{
			MGlobal::displayInfo("Camera changed CALLBAAACK!!");
		}
	}

	MCallbackId CameraviewID4 = MUiMessage::add3dViewPostRenderMsgCallback(
		"modelPanel3",
		CameraViewCallback,
		NULL,
		&status);

	if (status == MS::kSuccess)
	{
		if (myCallbackArray.append(CameraviewID4) == MS::kSuccess)
		{
			MGlobal::displayInfo("Camera changed CALLBAAACK!!");
		}
	}
	
	//MStringArray eventNames;
	//MEventMessage::getEventNames(eventNames);
	//for (size_t i = 0; i < eventNames.length(); i++)
	//{
	//	MGlobal::displayInfo(eventNames[i]);
	//}

	
	return res;
}

//	called when we exit the plugin, to safely clear our callback array.
EXPORT MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin App(obj);

	MMessage::removeCallbacks(myCallbackArray);
	MGlobal::displayInfo("Maya application unloaded!");

	delete Comlib;
	return MS::kSuccess;

}