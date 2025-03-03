#ifndef __GAME_ENGINE_H
#define __GAME_ENGINE_H
#include "pgr.h"
#include "render_stuff.h"

extern SCommonShaderProgram shaderProgram;  // main shader
extern SCommonShaderProgram waterShader;  // water shader
extern GameUniformVariables gameUniVars;   // uniform variables
extern GameState gameState;	      // scene variables
extern ExplosionShaderProgram explosionShader; // explosion shader

bool cameraPosition = false;
bool onPositionStatic = false;
bool onPositionFree = false;
bool changingPosition = false;
bool once = false;
bool firstTime = true;
extern glm::vec3 actualPosition;
extern glm::vec3 actualCenter;
//to know last position in the free mode
glm::vec3 freeCamPos;
glm::vec3 freeCamDir;
float teleportSpeed = 0.2;
// to show day colors
glm::vec4 day = glm::vec4(0.95f, 0.95f, 0.85f, 1.0f);
glm::vec4 currentColor = day;  //current state od daytime
std::list<Explosion*> explosions; //list of explosions

class gameEngine {
public:
    gameEngine() {
        m_corpseHandler = corpseHandler();
        m_screenHandler = screenHandler();
        m_keyBoardHandler = keyBoardHandler();
    }

    Object* createBanner();
    void evalLightIntensity();
    void changePointLight();
    void updateObjects(float elapsedTime);
    void initializeApplication();
    static void gameMenu(int choice);
    void createMenu(void);

    struct corpseHandler {
    public:
        void createCorpse();
        void updateCorpse(float elapsedTime);
    };

    struct screenHandler {
    public:
        static void timerCallback(int);
        static void passiveMouseMotionCallback(int mouseX, int mouseY);
        static void mouseCallback(int buttonPressed, int buttonState, int mouseX, int mouseY);

    };

    struct keyBoardHandler {
    public:
        static void keyboardCallback(unsigned char keyPressed, int mouseX, int mouseY);
        static void keyboardUpCallback(unsigned char keyReleased, int mouseX, int mouseY);
        static void specialKeyboardCallback(int specKeyPressed, int mouseX, int mouseY);
        static void specialKeyboardUpCallback(int specKeyReleased, int mouseX, int mouseY);

    };

    void restartGame();

private:
    corpseHandler m_corpseHandler;
    screenHandler m_screenHandler;
    keyBoardHandler m_keyBoardHandler;
};

#endif 