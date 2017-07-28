#pragma once

#include "ofMain.h"
#include "myCam.hpp"
#include "leapListener.hpp"
#include "bundleReader.hpp"
#include "ofxGui.h"
#include "avgRotation.hpp"
#include "camInterpolator.hpp"


class ofApp : public ofBaseApp{
    
public:
    void setup();
    void setupNewModel();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void updateDrawingLists();
    void updateQuadCoords();
    void updateQuadColors();
    void updateCameraCoords();
    
    void updatePointSorting();
    bool comparator(const ofVec3f & first, const ofVec3f & second);
    vector<ofVec3f> sortedPoints;
    
    
    //Bundle file
    string bundleFile;
    bundleReader bundle;
    
    //Calculate average cam info
    glm::quat averageCamRot;
    avgQueue<ofVec3f> avgCamPos;
    
    
    vector<ofVec3f> camVecCaptured;
    vector<ofQuaternion> camQuatCaptured;
    ofVec3f savedCamPos;
    ofQuaternion savedCamQuat;
 
    bool resumePos = false;
    
    
    camInterpolator interpolate;
    ofImage img;
    int frameCounter = 0;
    
    ofTrueTypeFont myFont;
    bool infoPanelOn = true;
    bool infoPanelInitial = true;
    bool isInterpolating = false;
    ofQuaternion mvQuat;
    ofVec3f mvVec;
    bool reticleOn = true;
  
    
    leapListener listener;
    Leap::Controller controller;
    
    
    GLuint pointAndCamsList;
    GLfloat * vertices;
    GLubyte * colors;
    float pointSize;
    float pointSize_scaleFactor = 1.15;
    float camSize;
    float camSize_scaleFactor = 1.15;
    ofQuaternion billboardingOrientation;
    ofVec3f rotatedCorner;
    ofVec3f transformedCorner;
    
    
};
