#ifndef Main_H_
#define Main_H_

#include "gameplay.h"
//#include "MayaShared.h"

using namespace gameplay;

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
	void CameraUpdated(char* &msg);
	void TransformChanged(char* &msg);
    Scene* _scene;
    bool _wireframe;
};

#endif
