#ifndef Main_H_
#define Main_H_
#include <string>
#include <vector>
#include "gameplay.h"
//#include "MayaShared.h"

using namespace gameplay;


//struct MeshContainer 
//{
//	vector<Model> models;
//	vector<Mesh*> meshes;
//	vector<string> name;
//	
//};
/**
 * Main game class.
 */
class Main: public Game
{
public:
    /**
     * Constructor.
     */
    Main();

    /**
     * @see Game::keyEvent
     */
	void keyEvent(Keyboard::KeyEvent evt, int key);
	
    /**
     * @see Game::touchEvent
     */
    void touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);

protected:

    /**
     * @see Game::initialize
     */
    void initialize();

    /**
     * @see Game::finalize
     */
    void finalize();

    /**
     * @see Game::update
     */
    void update(float elapsedTime);

    /**
     * @see Game::render
     */
    void render(float elapsedTime);

private:

    /**
     * Draws the scene each frame.
     */
    bool drawScene(Node* node);
	/*
	* Creates a new mesh from a newly created mesh in maya
	* @param[in] msg The message containing data about the mesh
	*/
	void CreateMesh(char* &msg);
	/*
	*	Unpakcs the message sent from maya
	*/
	void unPack();
	/*
	* Handles transform changes made in maya
	*/
	void TransformChanged(char* &msg);
	/*
	* Updates changes to camera
	*/
	void CameraUpdated(char* &msg);
	/*
	* Expands the mesh container if the number of meshes exceeds the current capacity
	*/
	/*
	* Updates the mesh's vertes positions(deletes a mesh and then creates a new one with the updated values)
	*/
	void UpdateMeshData(char* &msg);
	
	void ColorUpdate(char * &msg);


	//MeshContainer meshContainer;
	
	unsigned int meshCount;
    Scene* _scene;
    bool _wireframe;
};

#endif
