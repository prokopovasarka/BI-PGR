#ifndef __GAME_ENGINE_H
#define __GAME_ENGINE_H
#include "pgr.h"
#include "render_stuff.h"
#include "camera.h"
#include "configLoader.h"


// path to config.txt
const char* CONFIG_PATH = "data/config.txt";

extern SCommonShaderProgram shaderProgram;  // main shader
extern SCommonShaderProgram waterShader;  // water shader
extern GameUniformVariables gameUniVars;   // uniform variables
extern GameState gameState;	      // scene variables
extern ExplosionShaderProgram explosionShader; // explosion shader

bool cameraPosition = false;
bool once = false;
bool firstTime = true;
extern glm::vec3 actualPosition;
extern glm::vec3 actualCenter;
//to know last position in the free mode
glm::vec3 freeCamPos;
glm::vec3 freeCamDir;
float teleportSpeed = 0.2f;
// to show day colors
glm::vec4 day = glm::vec4(0.95f, 0.95f, 0.85f, 1.0f);
glm::vec4 currentColor = day;  //current state od daytime
std::list<Explosion*> explosions; //list of explosions

std::map<std::string, ObjectProp> m_loadProps;
Object* duck; //duck object

struct GameObjects {

    Camera* camera; //camera object

} gameObjects;

class gameEngine {
public:
    gameEngine() {
        m_duckHandler = duckHandler();
        m_screenHandler = screenHandler();
        m_keyBoardHandler = keyBoardHandler();
    }

    void evalLightIntensity();
    void changePointLight();
    void updateObjects(float elapsedTime);
    void initializeApplication();
    static void gameMenu(int choice);
    void createMenu(void);
    static void finalizeApplication();

    struct duckHandler {
    public:
        void createDuck();
        void updateDuck(float elapsedTime);
    };

    struct screenHandler {
    public:
        static void timerCallback(int);
        static void passiveMouseMotionCallback(int mouseX, int mouseY);
        static void mouseCallback(int buttonPressed, int buttonState, int mouseX, int mouseY);
        static void displayCallback();
        static void reshapeCallback(int newWidth, int newHeight);
        static void drawWindowContents(bool drawWater);

    };

    struct keyBoardHandler {
    public:
        static void keyboardCallback(unsigned char keyPressed, int mouseX, int mouseY);
        //static void keyboardUpCallback(unsigned char keyReleased, int mouseX, int mouseY);
        static void specialKeyboardCallback(int specKeyPressed, int mouseX, int mouseY);
        static void specialKeyboardUpCallback(int specKeyReleased, int mouseX, int mouseY);

    };

    void restartGame();

private:
    duckHandler m_duckHandler;
    screenHandler m_screenHandler;
    keyBoardHandler m_keyBoardHandler;
    static cameraHandler camHandler;
    static renderObjects renderHandler;
    static splineHandler splineFucHandler;
};

#endif 