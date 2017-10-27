#include "Main.h"
#include "Comlib.h"
#include "MayaShared.h"

// Declare our game instance
Main game;
Comlib* Receiver;

Main::Main()
    : _scene(NULL), _wireframe(false)
{
}

void Main::initialize()
{
    // Load game scene from file
    _scene = Scene::load("res/demo.scene");

    // Get the box model and initialize its material parameter values and bindings
    Node* boxNode = _scene->findNode("box");
    Model* boxModel = dynamic_cast<Model*>(boxNode->getDrawable());
    Material* boxMaterial = boxModel->getMaterial();

    // Set the aspect ratio for the scene's camera to match the current resolution
    _scene->getActiveCamera()->setAspectRatio(getAspectRatio());
	

    Receiver = new Comlib(BUFFERSIZE);
}
void Main::CameraUpdated(char* &msg)
{

}
void Main::TransformChanged(char* &msg)
{
	Translation *translate = new Translation();
	memcpy(translate, msg, sizeof(Translation));

	char* nodeName = translate->name;

	Vector3 transVec(translate->Tx, translate->Ty, translate->Tz);
	Node *node = _scene->findNode(nodeName);
	_scene->findNode(translate->name)->setTranslation(transVec);
}
void Main::CreateMesh(char* &msg)
{
    
	vector<Vertex> vtxVector;
	vector<Normal> NrmVector;
	size_t head = 0;
	MayaMesh* meshRecieved = new MayaMesh();
	memcpy(meshRecieved, msg, sizeof(MayaMesh));
	head += sizeof(MayaMesh);


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
	int arrayIt = meshRecieved->sizeOfVtxIndex * 6;
	float* meshVertexData = new float[meshRecieved->sizeOfVtxIndex * 6];


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
	}

	int *indices = new int[meshRecieved->sizeOfVtxIndex];
	for (size_t i = 0; i < meshRecieved->sizeOfVtxIndex; i++)
	{
		indices[i] = i;
	}

	VertexFormat::Element elements[] = {
		VertexFormat::Element(VertexFormat::POSITION, 3),
		VertexFormat::Element(VertexFormat::NORMAL, 3),
	};

	Mesh* newMesh = Mesh::createMesh(VertexFormat(elements, 2), meshRecieved->sizeOfVtxIndex, false);
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

	Node* lightNode = _scene->addNode("light");
	Light* light = Light::createPoint(Vector3(0.5f, 0.5f, 0.5f), 20);
	lightNode->setLight(light);
	SAFE_RELEASE(light);
	lightNode->translate(Vector3(0, 1, 5));

    models[0] = Model::create(newMesh);
	mats[0] = models[0]->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "POINT_LIGHT_COUNT 1");
	mats[0] = models[0]->setMaterial("res/demo.material#lambert2");
	mats[0]->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
	mats[0]->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");

	mats[0]->getParameter("u_ambientColor")->setValue(Vector3(0.2f, 0.1f, 0.4f));
	mats[0]->getStateBlock()->setCullFace(true);
	mats[0]->getStateBlock()->setDepthTest(true);
	mats[0]->getStateBlock()->setDepthWrite(true);
	
    char nodeName[75] = {};
    sprintf(nodeName, meshRecieved->name);
    
	Node *node = _scene->addNode(nodeName);
	
	node->setDrawable(models[0]);
	SAFE_RELEASE(models[0]);
	delete[] meshRecieved;
}

void Main::unPack()
{
	char* Package = nullptr;
	size_t* length;
	int Type = -1;

	
	if (Receiver->receive(Package, length))
	{
		memcpy(&Type, Package, sizeof(int));

	}
	
	

	switch (Type)
	{

	case (int)MsgType::CREATE_MESH:	
		//	CREATE_MESH
		CreateMesh(Package);		
		break;
	case (int)MsgType::VERTEX_TRANSLATION:
		//	Another Type
		break;
	case (int)MsgType::TRANSFORM_NODE_TRANSFORM:
		TransformChanged(Package);
		break;
	case(int)MsgType::CAMERA_UPDATE:
		CameraUpdated(Package);
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
    // Rotate model
    _scene->findNode("box")->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));

	

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
