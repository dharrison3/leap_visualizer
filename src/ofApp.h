#pragma once

#include "ofMain.h"
#include "myCam.hpp"
#include "leapListener.hpp"
#include "bundleReader.hpp"
#include "ofxGui.h"
#include "avgRotation.hpp"

class ofApp : public ofBaseApp{
    
public:
    void setup();
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
    
    ofImage img;
    
    ofTrueTypeFont myFont;
    bool toggleBar = true;
    
    leapListener listener;
    Leap::Controller controller;
    
    bundleReader bundle;
    avgRotation avgRot;
    glm::quat averageCamRot;
    
    GLfloat mats[10][4][4] =
    {
        //black plastic
        {{0, 0, 0, 1}, {0.01, 0.01, 0.01,1}, {0.5,0.5,0.5,1}, {32}},
        
        //brass
        {{.329412, .223529, .027451, 1}, {.780392,.568627,.113725,1}, {.992157, .941176,.807843,1}, {27.8974} },
        //bronze
        {{.2125,.1275,.054,1},{.714,.4284,.18144,1},{.393548,.271906,.166721,1},{25.6}},
        
        //chrome
        {{.25,.25,.25,1},{.4,.4,.4,1},{.774597,.774597,.774597,1},{76.8}},
        
        //copper
        {{.19125,.0735,.0225,1},{.7038,.27048,.0828,1},{.256777,.137622,.086014,1},{12.8}},
        
        //gold
        {{.24725,.1995,.0745,1},{.75164,.60648,.22648,1},{.628281,.555802,.366065,1},{51.2}},
        
        //pewter
        {{.10588,.058824,.113725,1}, {.427451,.470588,.541176,1}, {.3333,.3333,.521569,1},{9.84615}},
        
        //polished silver
        {{.19225,.19225,.19225,1},{.50754,.50754,.50754,1},{.508273,.508273,.508273,1},{51.2}},
        
        //silver
        {{.23125,.23125,.23125,1},{.2775,.2775,.2775,1},{.773911,.773911,.773911,1},{89.6}},
        
        //tut
        {{0.1, 0.1, 0.1, 1}, {0.3, 0.3, 0.3,1}, {1.0,0.5,0.5,1}, {1}}
        
    };
    
    
};
