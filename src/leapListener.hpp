//
//  leapListener.hpp
//  handyCam
//
//  Created by Drake S. Harrison on 6/6/17.
//
//

#include <iostream>
#include "Leap.h"
#include "myCam.hpp"
#include "avgQueue.hpp"

class leapListener : public Leap::Listener{
public:
    virtual void onConnect(const Leap::Controller&);
    virtual void onFrame(const Leap::Controller&);
    virtual void onExit(const Leap::Controller&);
    Leap::Vector sphereCenter;
    Leap::Vector handPos;
    float sphereRadius;
    myCam cam;
    
    avgQueue<ofQuaternion> rotationQueue;
    avgQueue<glm::vec3> translationQueue;
    
    bool rotationMode;
    bool translationMode;
    
private:
    
    bool isRotating;
    bool isTranslating;
    bool isGrabbing;
    bool sphereOn;
    Leap::Frame startFrameRot;
    Leap::Frame startFrameTrans;
    
    ofQuaternion averageRot;
    glm::vec3 averageTrans;
    
    };
