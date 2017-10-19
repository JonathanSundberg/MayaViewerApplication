#include "Main.h"
#include "Comlib.h"
#include "MayaShared.h"

// Declare our game instance
Main game;
Comlib* Receiver;

void CreateMesh(char* msg)
{
	vector<Vertex> vtxVector;
	MayaMesh meshRecieved;
	memcpy(&meshRecieved, msg, sizeof(MayaMesh));
	

	int* normIndexArr = new int[meshRecieved.sizeOfVtxIndex];
	memcpy(normIndexArr, msg + sizeof(MayaMesh), sizeof(int) * meshRecieved.sizeOfVtxIndex);
	Vertex vertex;
	for (size_t i = 0; i < meshRecieved.sizeOfVertices; i++)
	{
		memcpy(&vertex, msg + sizeof(MayaMesh) + (sizeof(int) * meshRecieved.sizeOfVtxIndex), sizeof(double) * 3);
	}



}
void unPack()
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

	case 0:	
		//	CREATE_MESH
		CreateMesh(Package);
	case 1:
		//	Another Type

	default:
		break;
	}
	
}

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
