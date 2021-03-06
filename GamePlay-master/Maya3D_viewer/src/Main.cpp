#include "Main.h"
#include "Comlib.h"
#include "MayaShared.h"
unsigned int meshCount;
unsigned int containerSize;
struct MeshData {
	Model* model;
	Mesh* mesh;
	string name;
	vector<Vertex> vertices;
	vector<Normal> normals;
};

struct MeshInfo
{
	char meshName[75];
	float* meshVertexData;
	int sizeOfVtxIndex;
};
struct MeshContainer
{
	vector<Model*> models;
	vector<Mesh*> meshes;
	vector<Material*> materials;
	vector<Node*> nodes;
	vector<string> name;
	vector<string> matName;
	vector<MeshInfo> RecreationData;
	vector<bool> textured;
};


// Declare our game instance
Main game;
Comlib* Receiver;
MeshContainer container;


Main::Main()
    : _scene(NULL), _wireframe(false)
{
}
void expandContainer() {
	//MeshData* temp = new MeshData[containerSize];
	//for (size_t i = 0; i < containerSize; i++)
	//{
	//	temp[i]->model = Model::create(meshData->mesh);
	//	temp->mesh(meshData->mesh);
	//}
	//
}
void Main::initialize()
{
    // Load game scene from file
    _scene = Scene::load("res/demo.scene");
	
	//Initialize light
	Node* lightNode = _scene->addNode("Light");
	Light* light = Light::createPoint(Vector3(0.9f, 0.9f, 0.9f), 30);
	lightNode->setLight(light);
	SAFE_RELEASE(light);
	lightNode->translateUp(2.0f);

    // Get the box model and initialize its material parameter values and bindings
   Node* boxNode = _scene->findNode("box");
   _scene->removeNode(boxNode);
    //Model* boxModel = dynamic_cast<Model*>(boxNode->getDrawable());
    //Material* boxMaterial = boxModel->getMaterial();

    // Set the aspect ratio for the scene's camera to match the current resolution
	Camera* cam = Camera::createPerspective(45.0f, 1920 / 1080.0f, 0.1f, 100.0f);
	Node* camNode = _scene->addNode("camera");
	
	camNode->setCamera(cam);
	_scene->setActiveCamera(cam);
	camNode->setTranslation(0, 0, 10);
	SAFE_RELEASE(cam);


    Receiver = new Comlib(BUFFERSIZE);
}
unsigned int findMesh(string meshName)
{
	unsigned int index = -1;
	for (size_t i = 0; i < container.name.size(); i++)
	{
		if (container.name[i] == meshName)
			return i;
	}

	return index;
}
void Main::CameraUpdated(char* &msg)
{

	
	MCamera* mCam = new MCamera();
	memcpy(mCam, msg, sizeof(MCamera));

	Node* find = _scene->findNode(mCam->name);
	Vector3 translateVec(mCam->fViewMatrix[3][0], mCam->fViewMatrix[3][1], mCam->fViewMatrix[3][2]);
	if (find == NULL)
	{
		if (mCam->isOrtho)
		{
			Camera* newCam = Camera::createOrthographic(20, 20, mCam->aspectRatio, mCam->nearPlane, mCam->farPlane);
			
			Node* orthoCam = _scene->addNode(mCam->name);
			orthoCam->setCamera(newCam);
			_scene->setActiveCamera(newCam);


		}
		else
		{
			Camera* newCam = Camera::createPerspective(mCam->FOV, mCam->aspectRatio, mCam->nearPlane, mCam->farPlane);
			Node* perpCam = _scene->addNode(mCam->name);
			perpCam->setCamera(newCam);
			_scene->setActiveCamera(newCam);
		}
	}
	else
	{
		if (mCam->isOrtho)
		{
			_scene->findNode(mCam->name)->setTranslation(translateVec);
			_scene->findNode(mCam->name)->setRotation(mCam->Rot[0], mCam->Rot[1], mCam->Rot[2], mCam->Rot[3]);
			_scene->findNode(mCam->name)->getCamera()->setZoomX(mCam->zoom);
			_scene->findNode(mCam->name)->getCamera()->setZoomY(mCam->zoom); 
			_scene->setActiveCamera(_scene->findNode(mCam->name)->getCamera());
		}
		else
		{
			_scene->findNode(mCam->name)->setTranslation(translateVec);
			_scene->findNode(mCam->name)->setRotation(mCam->Rot[0], mCam->Rot[1], mCam->Rot[2], mCam->Rot[3]);
			_scene->findNode(mCam->name)->getCamera()->setAspectRatio(mCam->aspectRatio);
			_scene->findNode(mCam->name)->getCamera()->setFieldOfView(mCam->FOV);
			_scene->setActiveCamera(_scene->findNode(mCam->name)->getCamera());
		}
	}

	

	delete[] mCam;
}
void Main::TransformChanged(char* &msg)
{

	TransformData *transformData = new TransformData();
	memcpy(transformData, msg, sizeof(TransformData));
	if (transformData->name == "topShape" || transformData->name == "sideShape" || transformData->name == "frontShape")
	{
		return;
	}

	Vector3 transVec(transformData->Tx, transformData->Ty, transformData->Tz);
	_scene->findNode(transformData->name)->setTranslation(transVec); 
	_scene->findNode(transformData->name)->setRotation(transformData->Rx, transformData->Ry, transformData->Rz, transformData->Rw);
	_scene->findNode(transformData->name)->setScale(transformData->Sx, transformData->Sy, transformData->Sz);
	unsigned int index = findMesh(transformData->name);
	
	Mesh* tempMesh;
	Model* model;
	
	delete[] transformData;
}
void Main::nodeRemoved(char* &msg)
{
	NodeName* nodeName = new NodeName();
	memcpy(nodeName, msg, sizeof(NodeName));
	
	_scene->removeNode(_scene->findNode(nodeName->name));

	delete[] nodeName;
}
void Main::UpdateMeshData(char* &msg)
{
	Vector3 replaceTransVec;
	Quaternion replaceRotQuat;
	Vector3 replaceScaleVec;
	bool replace = true;
	MayaMesh* meshRecieved = new MayaMesh();
	memcpy(meshRecieved, msg, sizeof(MayaMesh));
	Node* node = _scene->findNode(meshRecieved->name);
	_scene->findNode(meshRecieved->name)->getTranslation(&replaceTransVec);
	_scene->findNode(meshRecieved->name)->getRotation(&replaceRotQuat);
	_scene->findNode(meshRecieved->name)->getScale(&replaceScaleVec);
	_scene->removeNode(node);
	
	delete[] meshRecieved;
	CreateMesh(msg,replace,replaceTransVec,replaceRotQuat,replaceScaleVec);
	
}
void Main::ColorUpdate(char* &msg)
{
	Color *myColor = new Color();
	Vector3 replaceTransVec;
	Quaternion replaceRotQuat;
	Vector3 replaceScaleVec;
	memcpy(myColor, msg, sizeof(Color));
	string tempMat = myColor->matName;
	for (size_t i = 0; i < container.name.size(); i++)
	{
		if (container.name[i] == myColor->meshName)
		{
			container.matName[i] = tempMat;
		}
	}
	for (size_t i = 0; i < container.matName.size(); i++)
	{
		if (container.matName[i] == tempMat)
		{
			_scene->findNode(container.name[i].c_str())->getTranslation(&replaceTransVec);
			_scene->findNode(container.name[i].c_str())->getRotation(&replaceRotQuat);
			_scene->findNode(container.name[i].c_str())->getScale(&replaceScaleVec);
			Vector4 newColor = Vector4(myColor->colors[0], myColor->colors[1], myColor->colors[2], 1.0f);
			CreateColoredMesh(i, newColor, replaceTransVec, replaceRotQuat, replaceScaleVec);
		}
	}


	delete myColor;
}
void Main::TextureUpdate(char* &msg)
{
	TextureName *texName = new TextureName();
	memcpy(texName, msg, sizeof(TextureName));
	string tempMat = texName->matName;
	string tempFile = texName->file;

	Vector3 replaceTransVec;
	Quaternion replaceRotQuat;
	Vector3 replaceScaleVec;
	size_t texSize = tempFile.size();
	if (texSize > 3)
	{
		for (size_t i = 0; i < container.matName.size(); i++)
		{

			if (container.matName[i] == tempMat)
			{
				const char* fileName = tempFile.c_str();

				_scene->findNode(container.name[i].c_str())->getTranslation(&replaceTransVec);
				_scene->findNode(container.name[i].c_str())->getRotation(&replaceRotQuat);
				_scene->findNode(container.name[i].c_str())->getScale(&replaceScaleVec);

				CreateTexturedMesh(i, fileName, replaceTransVec, replaceRotQuat, replaceScaleVec);
			}

		}
	}

	delete[] texName;
}
void Main::MaterialChange(char* &msg)
{
	MatChange* newMaterial = new MatChange();
	memcpy(newMaterial, msg, sizeof(MatChange));

	string tempMesh = newMaterial->meshName;
	string tempMat = newMaterial->matName;
	string tempTex = newMaterial->texture;

	for (size_t i = 0; i < container.name.size(); i++)
	{
		if (container.name[i] == tempMesh)
		{
			container.matName[i] = tempMat;
			const char* texFile = tempTex.c_str();
		}
	}
}

void Main::CreateTexturedMesh(int infoIndex, const char* texture, Vector3 replaceTransVec, Quaternion replaceRotQuat, Vector3 replaceScaleVec)
{


	int Size = container.RecreationData[infoIndex].sizeOfVtxIndex;
	container.textured[infoIndex] = true;

	int *indices = new int[Size];
	for (size_t i = 0; i < Size; i++)
	{
		indices[i] = i;
	}

	VertexFormat::Element elements[] = {
		VertexFormat::Element(VertexFormat::POSITION, 3),
		VertexFormat::Element(VertexFormat::NORMAL, 3),
		VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
	};

	Mesh* newMesh = Mesh::createMesh(VertexFormat(elements, 3), Size, false);
	if (newMesh == NULL)
	{
		GP_ERROR("Failed to create mesh");
		return;
	}

	newMesh->setVertexData(container.RecreationData[infoIndex].meshVertexData, 0, Size);
	MeshPart* meshPart = newMesh->addPart(Mesh::TRIANGLES, Mesh::INDEX32, Size);
	meshPart->setIndexData(indices, 0, Size);


	Model* models[10];
	Material *mats[10];
	Texture::Sampler* sampler;

	Node* lightNode = _scene->addNode("light");
	Light* light = Light::createPoint(Vector3(0.5f, 0.5f, 0.5f), 20);
	lightNode->setLight(light);
	SAFE_RELEASE(light);
	lightNode->translate(Vector3(0, 1, 5));

	models[0] = Model::create(newMesh);
	mats[0] = models[0]->setMaterial("res/shaders/textured.vert", "res/shaders/textured.frag", "POINT_LIGHT_COUNT 1");

	mats[0]->getParameter("u_ambientColor")->setValue(Vector3(0.25f, 0.25f, 0.25f));
	mats[0]->getParameter("u_pointLightColor[0]")->setValue(_scene->findNode("Light")->getLight()->getColor());
	mats[0]->getParameter("u_pointLightPosition[0]")->bindValue(_scene->findNode("Light"), &Node::getTranslationWorld);
	mats[0]->getParameter("u_pointLightRangeInverse[0]")->bindValue(_scene->findNode("Light")->getLight(), &Light::getRangeInverse);
	mats[0]->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
	mats[0]->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
	sampler = mats[0]->getParameter("u_diffuseTexture")->setValue(texture, true);

	sampler->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR);

	mats[0]->getStateBlock()->setCullFace(true);
	mats[0]->getStateBlock()->setDepthTest(true);
	mats[0]->getStateBlock()->setDepthWrite(true);


	char nodeName[75] = {};
	sprintf(nodeName, container.RecreationData[infoIndex].meshName);
	_scene->removeNode(_scene->findNode(nodeName));
	Node *node = _scene->addNode(nodeName);

	_scene->findNode(container.RecreationData[infoIndex].meshName)->setTranslation(replaceTransVec);
	_scene->findNode(container.RecreationData[infoIndex].meshName)->setRotation(replaceRotQuat);
	_scene->findNode(container.RecreationData[infoIndex].meshName)->setScale(replaceScaleVec);
	
	node->setDrawable(models[0]);
	SAFE_RELEASE(models[0]);
	meshCount++;
}

void Main::CreateColoredMesh(int infoIndex, Vector4 meshColor, Vector3 replaceTransVec, Quaternion replaceRotQuat, Vector3 replaceScaleVec)
{

	int Size = container.RecreationData[infoIndex].sizeOfVtxIndex;
	container.textured[infoIndex] = false;

	int *indices = new int[Size];
	for (size_t i = 0; i < Size; i++)
	{
		indices[i] = i;
	}

	VertexFormat::Element elements[] = {
		VertexFormat::Element(VertexFormat::POSITION, 3),
		VertexFormat::Element(VertexFormat::NORMAL, 3),
		VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
	};

	Mesh* newMesh = Mesh::createMesh(VertexFormat(elements, 3), Size, false);
	if (newMesh == NULL)
	{
		GP_ERROR("Failed to create mesh");
		return;
	}

	newMesh->setVertexData(container.RecreationData[infoIndex].meshVertexData, 0, Size);
	MeshPart* meshPart = newMesh->addPart(Mesh::TRIANGLES, Mesh::INDEX32, Size);
	meshPart->setIndexData(indices, 0, Size);


	Model* models[10];
	Material *mats[10];
	Texture::Sampler* sampler;

	Node* lightNode = _scene->addNode("light");
	Light* light = Light::createPoint(Vector3(0.5f, 0.5f, 0.5f), 20);
	lightNode->setLight(light);
	SAFE_RELEASE(light);
	lightNode->translate(Vector3(0, 1, 5));


	models[0] = Model::create(newMesh);
	mats[0] = models[0]->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "POINT_LIGHT_COUNT 1");
	
	mats[0]->getParameter("u_ambientColor")->setValue(Vector3(0.25f, 0.25f, 0.25f));
	mats[0]->getParameter("u_pointLightColor[0]")->setValue(_scene->findNode("Light")->getLight()->getColor());
	mats[0]->getParameter("u_pointLightPosition[0]")->bindValue(_scene->findNode("Light"), &Node::getTranslationWorld);
	mats[0]->getParameter("u_pointLightRangeInverse[0]")->bindValue(_scene->findNode("Light")->getLight(), &Light::getRangeInverse);
	mats[0]->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
	mats[0]->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
	mats[0]->getParameter("u_diffuseColor")->setValue(meshColor);


	mats[0]->getStateBlock()->setCullFace(true);
	mats[0]->getStateBlock()->setDepthTest(true);
	mats[0]->getStateBlock()->setDepthWrite(true);


	char nodeName[75] = {};
	sprintf(nodeName, container.RecreationData[infoIndex].meshName);
	_scene->removeNode(_scene->findNode(nodeName));
	Node *node = _scene->addNode(nodeName);

	_scene->findNode(container.RecreationData[infoIndex].meshName)->setTranslation(replaceTransVec);
	_scene->findNode(container.RecreationData[infoIndex].meshName)->setRotation(replaceRotQuat);
	_scene->findNode(container.RecreationData[infoIndex].meshName)->setScale(replaceScaleVec);

	node->setDrawable(models[0]);
	SAFE_RELEASE(models[0]);
	meshCount++;

}

void Main::CreateMesh(char* &msg, bool replace, Vector3 replaceTransVec, Quaternion replaceRotQuat, Vector3 replaceScaleVec)
{
    
	vector<Vertex> vtxVector;
	vector<Normal> NrmVector;
	vector<UV> UVs;

	size_t head = 0;
	MayaMesh* meshRecieved = new MayaMesh();
	memcpy(meshRecieved, msg, sizeof(MayaMesh));
	head += sizeof(MayaMesh);
	container.matName.push_back(meshRecieved->materialName);

    ///////////////     VERTEX INDEX    \\\\\\\\\\\\\\\\\\

	int* vtxIndexArr = new int[meshRecieved->sizeOfVtxIndex];
	memcpy(vtxIndexArr, msg + head, sizeof(int) * meshRecieved->sizeOfVtxIndex);
	head += sizeof(int)*meshRecieved->sizeOfVtxIndex;
	int a = -1;
	for (size_t i = 0; i < 36; i++)
	{
		a = vtxIndexArr[i];

	}
    ///////////////     VERTEX INFORMAION    \\\\\\\\\\\\\\\\\\

	Vertex vertex;
	for (size_t i = 0; i < meshRecieved->sizeOfVertices; i++)
	{
		memcpy(&vertex, msg + head + (sizeof(Vertex) * i), sizeof(Vertex));
		
		vtxVector.push_back(vertex);
	}
	head += sizeof(Vertex) * meshRecieved->sizeOfVertices;
    ///////////////     NORMAL INDEX    \\\\\\\\\\\\\\\\\\

	int* NrmIndexArr = new int[meshRecieved->sizeOfNormalIndex];
	memcpy(NrmIndexArr, msg + head, sizeof(int)*meshRecieved->sizeOfNormalIndex);

	for (size_t i = 0; i < meshRecieved->sizeOfNormalIndex; i++)
	{
		int b = NrmIndexArr[i];
	}
	head += sizeof(int)*meshRecieved->sizeOfNormalIndex;
    ///////////////     NORMALS   \\\\\\\\\\\\\\\\\\

	Normal norm;
	for (size_t i = 0; i < meshRecieved->sizeOfNormals; i++)
	{
		memcpy(&norm, msg + head + sizeof(Normal) * i, sizeof(Normal));
		NrmVector.push_back(norm);
	}

	head += sizeof(Normal)*meshRecieved->sizeOfNormals;
	float* meshVertexData = new float[meshRecieved->sizeOfVtxIndex * 8];

	///////////////		UV DATA        \\\\\\\\\\\\\\\\\\

	UV uv;
	for (size_t i = 0; i < meshRecieved->sizeOfUV; i++)
	{
		memcpy(&uv, msg + head + sizeof(UV) * i, sizeof(UV));
		UVs.push_back(uv);
	}
	head += sizeof(UV) * meshRecieved->sizeOfUV;

	int* UvIndexArr = new int[meshRecieved->sizeOfUVIndex];
	memcpy(UvIndexArr, msg + head, sizeof(int)*meshRecieved->sizeOfUVIndex);

    ///////////////     CREATE MESH   \\\\\\\\\\\\\\\\\\

	size_t index = 0;
	for (size_t i = 0; i < meshRecieved->sizeOfVtxIndex; i++)
	{
		// Vertices
		meshVertexData[index] = vtxVector[vtxIndexArr[i]].position[0];
		index++;
		meshVertexData[index] = vtxVector[vtxIndexArr[i]].position[1];
		index++;
		meshVertexData[index] = vtxVector[vtxIndexArr[i]].position[2];
		index++;

		// Normals
		meshVertexData[index] = NrmVector[NrmIndexArr[i]].normal[0];
		index++;
		meshVertexData[index] = NrmVector[NrmIndexArr[i]].normal[1];
		index++;
		meshVertexData[index] = NrmVector[NrmIndexArr[i]].normal[2];
		index++;

		//UVs
		meshVertexData[index] = UVs[UvIndexArr[i]].U;
		index++;
		meshVertexData[index] = UVs[UvIndexArr[i]].V;
		index++;
	}

	int *indices = new int[meshRecieved->sizeOfVtxIndex];
	for (size_t i = 0; i < meshRecieved->sizeOfVtxIndex; i++)
	{
		indices[i] = i;
	}

	VertexFormat::Element elements[] = {
		VertexFormat::Element(VertexFormat::POSITION, 3),
		VertexFormat::Element(VertexFormat::NORMAL, 3),
		VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
	};

	Mesh* newMesh = Mesh::createMesh(VertexFormat(elements, 3), meshRecieved->sizeOfVtxIndex, false);
	if (newMesh == NULL)
	{
		GP_ERROR("Failed to create mesh");
		return;
	}

	newMesh->setVertexData(meshVertexData, 0, meshRecieved->sizeOfVtxIndex);
	MeshPart* meshPart = newMesh->addPart(Mesh::TRIANGLES, Mesh::INDEX32, meshRecieved->sizeOfVtxIndex);
    meshPart->setIndexData(indices, 0, meshRecieved->sizeOfVtxIndex);
	
	
    Model* models[10];
	Material *mats[10];
	Texture::Sampler* sampler;

	Node* lightNode = _scene->addNode("light");
	Light* light = Light::createPoint(Vector3(0.5f, 0.5f, 0.5f), 20);
	lightNode->setLight(light);
	SAFE_RELEASE(light);
	lightNode->translate(Vector3(0, 1, 5));


    models[0] = Model::create(newMesh);
	mats[0] = models[0]->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "POINT_LIGHT_COUNT 1");

	mats[0]->getParameter("u_ambientColor")->setValue(Vector3(0.25f, 0.25f, 0.25f));
	mats[0]->getParameter("u_pointLightColor[0]")->setValue(_scene->findNode("Light")->getLight()->getColor());
	mats[0]->getParameter("u_pointLightPosition[0]")->bindValue(_scene->findNode("Light"), &Node::getTranslationWorld);
	mats[0]->getParameter("u_pointLightRangeInverse[0]")->bindValue(_scene->findNode("Light")->getLight(), &Light::getRangeInverse);
	mats[0]->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
	mats[0]->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
	mats[0]->getParameter("u_diffuseColor")->setValue(Vector4(0.4f, 0.4f, 0.4f, 1.0f));


	mats[0]->getStateBlock()->setCullFace(true);
	mats[0]->getStateBlock()->setDepthTest(true);
	mats[0]->getStateBlock()->setDepthWrite(true);
	
    char nodeName[75] = {};
    sprintf(nodeName, meshRecieved->name);
    
	Node *node = _scene->addNode(nodeName);

	if (replace)
	{
		_scene->findNode(meshRecieved->name)->setTranslation(replaceTransVec);
		_scene->findNode(meshRecieved->name)->setRotation(replaceRotQuat);
		_scene->findNode(meshRecieved->name)->setScale(replaceScaleVec);
	}
	//Adding the meshes and models to the meshcontainer struct

	MeshInfo myInfo;

	sprintf(myInfo.meshName, meshRecieved->name);
	myInfo.meshVertexData = new float[meshRecieved->sizeOfVtxIndex * 8];
	memcpy(myInfo.meshVertexData, meshVertexData, sizeof(float) * (meshRecieved->sizeOfVtxIndex * 8));
	myInfo.sizeOfVtxIndex = meshRecieved->sizeOfVtxIndex;



	container.textured.push_back(false);
	container.RecreationData.push_back(myInfo);
	container.nodes.push_back(node);
	container.meshes.push_back(newMesh);
	container.models.push_back(Model::create(newMesh));
	container.name.push_back(nodeName);
	container.materials.push_back(mats[0]);

	node->setDrawable(models[0]);
	SAFE_RELEASE(models[0]);
	meshCount++;
	delete[] NrmIndexArr;
	delete[] vtxIndexArr;
	delete[] meshVertexData;
	delete[] meshRecieved;
	delete[] UvIndexArr;
	vtxVector.clear();
	NrmVector.clear();
	UVs.clear();
}

void Main::unPack()
{
	char* Package = nullptr;
	size_t* length;
	int Type = -1;

	
	if (Receiver->receive(Package, length))
	{
		memcpy(&Type, Package, sizeof(int));

		int a = Type;
	}
	
	
	switch (Type)
	{

	case (int)MsgType::CREATE_MESH:	
		//	CREATE_MESH
		CreateMesh(Package,false);		
		break;
	case (int)MsgType::VERTEX_TRANSLATION:
		UpdateMeshData(Package);
		break;
	case (int)MsgType::TRANSFORM_NODE_TRANSFORM:
		TransformChanged(Package);
		break;
	case(int)MsgType::CAMERA_UPDATE:
		CameraUpdated(Package);
		break;
	case(int)MsgType::COLOR_UPDATE:
		ColorUpdate(Package);
		break;
	case(int)MsgType::NODE_REMOVED:
		nodeRemoved(Package);
		break;
	case(int)MsgType::TEXTURE_UPDATE:
		TextureUpdate(Package);
			break;
	case(int)MsgType::MATERIAL_CHANGE:
		MaterialChange(Package);
		break;
	default:
		break;
	}
	
}



void Main::finalize()
{
    SAFE_RELEASE(_scene);
}

void Main::update(float elapsedTime)
{
	unPack();
}

void Main::render(float elapsedTime)
{
    // Clear the color and depth buffers
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);

    // Visit all the nodes in the scene for drawing
    _scene->visit(this, &Main::drawScene);
}

bool Main::drawScene(Node* node)
{
    // If the node visited contains a drawable object, draw it
    Drawable* drawable = node->getDrawable(); 
    if (drawable)
        drawable->draw(_wireframe);

    return true;
}

void Main::keyEvent(Keyboard::KeyEvent evt, int key)
{
    if (evt == Keyboard::KEY_PRESS)
    {
        switch (key)
        {
        case Keyboard::KEY_ESCAPE:
            exit();
            break;
        }
    }
}

void Main::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    switch (evt)
    {
    case Touch::TOUCH_PRESS:
        _wireframe = !_wireframe;
        break;
    case Touch::TOUCH_RELEASE:
        break;
    case Touch::TOUCH_MOVE:
        break;
    };
}

