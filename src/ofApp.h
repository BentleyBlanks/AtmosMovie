#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include <Atmos.h>
#include "ThemeTest.h"
#include "AtmosShapeData.h"
#include "AtmosLightData.h"

class ofApp : public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    void imGuiTheme();
    void initAtmos();

    // ImGui
    void initImGui();
    void renderingMenu();
    void cameraMenu();
    void shapeMenu();
    void lightMenu();

    // shape
    void shapeSphere(int index);
    void shapeTriangle(int index);
    void shapeTriangleMesh(int index);
    void shapeDisk(int index);
    void shapePlane(int index);
    void shapeInfinitePlane(int index);
    void shapeDelete(int index);
    void setBSDF(int index, shapeData* shape);

    // light
    void lightArea(int index);
    void lightSpot(int index);
    void lightPoint(int index);
    void lightInfinite(int index);
    void lightDelete(int index);

    // process of rendering
    void renderingPanel();

    // about window
    void about();

    // Atmos
    a3GridRenderer* renderer;
    a3Scene* scene;

    ofPixels previewPixels;
    ofTexture preview;

    t3Timer timer;

    // ImGui
    ofxImGui gui;
    bool openRenderingWindow, openCameraWindow, openShapeWindow, openLightWindow, openAboutWindow;
    ofImage logo;
    GLuint logoButtonID;

    // Atmos with ImGui
    // config
    int startFrame, endFrame;
    int spp;
    bool hasKeyFrame;
    bool startRendering;
    int level[2];

    // image
    int imageWidth, imageHeight;
    int localStartPos[2], localRenderSize[2];
    char saveToPath[1024];
    char saveImageName[1024];

    // integrator / primitive set
    bool enablePath, enableBVH;
    int maxDepth, russianRouletteDepth;

    // post effect
    bool enableGammaCorrection, enableToneMapping;

    // camera
    float cameraLookat[3], cameraOrigin[3], cameraUp[3];
    float cameraFov;
    float cameraFocalDistance, cameraLensRadius;
    
    // shape
    vector<shapeData*> shapeList;

    // light
    vector<lightData*> lightList;

    // ImGui Start Rendering
    bool stopRendering;
    int currentFrame;
    float progress;

    // Atmos begin / end once
    bool atmosInitOnce, atmosEndOnce;

    // window
    int windowWidth, windowHeight;
};
