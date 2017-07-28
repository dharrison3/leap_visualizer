#include "myCam.hpp"
#include "ofMath.h"
#include "ofUtils.h"

// when an ofEasyCam is moving due to momentum, this keeps it
// from moving forever by assuming small values are zero.
static const float minDifference = 0.1e-5f;

// this is the default on windows os
static const unsigned long doubleclickTime = 200;

//----------------------------------------
myCam::myCam(){
    reset();
}

myCam::~myCam(){
   

}

//----------------------------------------
void myCam::update(ofEventArgs & args){
    viewport = getViewport(this->viewport);
    //if(!bDistanceSet && bAutoDistance){
    //    setDistance(getImagePlaneDistance(viewport), true);
    //}
    if(bMouseInputEnabled){
        
        if(events->getMousePressed()) prevMouse = glm::vec2(events->getMouseX(),events->getMouseY());
        
        if (bDoRotate) {
            updateRotation();
        }else if (bDoTranslate || bDoScrollZoom || bIsBeingScrolled) {
            updateTranslation();
            bDoScrollZoom = false;
        }
    }
}

//----------------------------------------
void myCam::begin(ofRectangle _viewport){
    if(!bEventsSet){
        setEvents(ofEvents());
    }
    viewport = getViewport(_viewport);
    ofCamera::begin(viewport);
    
}

//----------------------------------------
void myCam::reset(){
    target.resetTransform();
    
    target.setPosition(0, 0, 0);
    lookAt(target);
    
    resetTransform();
    setPosition(0, 0, lastDistance);
    
    xRot = 0;
    yRot = 0;
    zRot = 0;
    
    moveX = 0;
    moveY = 0;
    moveZ = 0;
    
    bApplyInertia = false;
    bDoTranslate = false;
    bDoRotate = false;
    
    
}

//----------------------------------------
void myCam::setTarget(const glm::vec3& targetPoint){
    target.setPosition(targetPoint);
    lookAt(target);
}

//----------------------------------------
void myCam::setTarget(ofNode& targetNode){
    target = targetNode;
    lookAt(target);
}

//----------------------------------------
const ofNode& myCam::getTarget() const{
    return target;
}

//----------------------------------------
void myCam::setDistance(float distance){
    setDistance(distance, true);
}

//----------------------------------------
void myCam::setDistance(float distance, bool save){//should this be the distance from the camera to the target?
    if(distance > 0.0f){
        if(save){
            this->lastDistance = distance;
        }
        setPosition(target.getPosition() + (distance * getZAxis()));
        bDistanceSet = true;
    }
}

//----------------------------------------
float myCam::getDistance() const{
    return glm::distance(target.getPosition(), getPosition());
}

//----------------------------------------
void myCam::setAutoDistance(bool bAutoDistance){
    this->bAutoDistance = bAutoDistance;
    if(bAutoDistance){
        bDistanceSet = false;
    }
}

//----------------------------------------
void myCam::setDrag(float drag){
    this->drag = drag;
}

//----------------------------------------
float myCam::getDrag() const{
    return drag;
}

//----------------------------------------
void myCam::setTranslationKey(char key){
    doTranslationKey = key;
}

//----------------------------------------
char myCam::getTranslationKey() const{
    return doTranslationKey;
}

//----------------------------------------
void myCam::enableMouseInput(){
    if(!bMouseInputEnabled && events){
        listeners.push_back(events->update.newListener(this, &myCam::update));
        listeners.push_back(events->mouseDragged.newListener(this, &myCam::mouseDragged));
        listeners.push_back(events->mousePressed.newListener(this, &myCam::mousePressed));
        listeners.push_back(events->mouseReleased.newListener(this, &myCam::mouseReleased));
        listeners.push_back(events->mouseScrolled.newListener(this, &myCam::mouseScrolled));
    }
    // if enableMouseInput was called within ofApp::setup()
    // `events` will still carry a null pointer, and bad things
    // will happen. Therefore we only update the flag.
    bMouseInputEnabled = true;
    // setEvents() is called upon first load, and will make sure
    // to enable the mouse input once the camera is fully loaded.
}

//----------------------------------------
void myCam::disableMouseInput(){
    if(bMouseInputEnabled && events){
        listeners.clear();
    }
    // if disableMouseInput was called within ofApp::setup()
    // `events` will still carry a null pointer, and bad things
    // will happen. Therefore we only update the flag.
    bMouseInputEnabled = false;
    // setEvents() is called upon first load, and will make sure
    // to enable the mouse input once the camera is fully loaded.
}

//----------------------------------------
void myCam::setEvents(ofCoreEvents & _events){
    // If en/disableMouseInput were called within ofApp::setup(),
    // bMouseInputEnabled will tell us about whether the camera
    // mouse input needs to be initialised as enabled or disabled.
    // we will still set `events`, so that subsequent enabling
    // and disabling can work.
    
    // we need a temporary copy of bMouseInputEnabled, since it will
    // get changed by disableMouseInput as a side-effect.
    bool wasMouseInputEnabled = bMouseInputEnabled || !events;
    disableMouseInput();
    events = &_events;
    if (wasMouseInputEnabled) {
        // note: this will set bMouseInputEnabled to true as a side-effect.
        enableMouseInput();
    }
    bEventsSet = true;
}

//----------------------------------------
void myCam::setRotationSensitivity(float x, float y, float z){
    sensitivityRotX = x;
    sensitivityRotY = y;
    sensitivityRotZ = z;
}

//----------------------------------------
void myCam::setTranslationSensitivity(float x, float y, float z){
    sensitivityX = x;
    sensitivityY = y;
    sensitivityZ = z;
}

//----------------------------------------
bool myCam::getMouseInputEnabled() const{
    return bMouseInputEnabled;
}

//----------------------------------------
void myCam::enableMouseMiddleButton(){
    bEnableMouseMiddleButton = true;
}

//----------------------------------------
void myCam::disableMouseMiddleButton(){
    bEnableMouseMiddleButton = false;
}

//----------------------------------------
bool myCam::getMouseMiddleButtonEnabled() const{
    return bEnableMouseMiddleButton;
}

//----------------------------------------
glm::vec3 myCam::up() const{
    if(relativeYAxis){
        if(bApplyInertia){
            return getYAxis();
        }else{
            return prevAxisY;
        }
    }else{
        return upAxis;
    }
}

//----------------------------------------
void myCam::setRelativeYAxis(bool relative){
    relativeYAxis = relative;
}

//----------------------------------------
bool myCam::getRelativeYAxis() const{
    return relativeYAxis;
}

//----------------------------------------
void myCam::setUpAxis(const glm::vec3 & _up){
    upAxis = _up;
}

//----------------------------------------
const glm::vec3 & myCam::getUpAxis() const{
    return upAxis;
}

//----------------------------------------
void myCam::enableInertia(){
    doInertia = true;
}

//----------------------------------------
void myCam::disableInertia(){
    doInertia = false;
}

//----------------------------------------
bool myCam::getInertiaEnabled() const{
    return doInertia;
}

//----------------------------------------
void myCam::updateTranslation(){
    if(bApplyInertia){
        moveX *= drag;
        moveY *= drag;
        moveZ *= drag;
        cout << "INERTIA" << endl;
        if(ABS(moveZ) >= minDifference){
            bIsBeingScrolled = true;
        } else {
            bIsBeingScrolled = false;
        }
        
        if(ABS(moveX) <= minDifference && ABS(moveY) <= minDifference && ABS(moveZ) <= minDifference){
            bApplyInertia = false;
            bDoTranslate = false;
        }
        move((getXAxis() * moveX) + (getYAxis() * moveY) + (getZAxis() * moveZ));
    }else if(bDoTranslate){
        
        setPosition(prevPosition + glm::vec3(prevAxisX * moveX) + (prevAxisY * moveY)/* + (prevAxisZ * moveZ)*/);
        
                //Translates the target
        target.setPosition(prevTarget + glm::vec3(prevAxisX * moveX) + (prevAxisY * moveY)/* + (prevAxisZ * moveZ)*/);

        
    }else if(bIsBeingScrolled){
        setPosition(prevPosition + glm::vec3(prevAxisZ * moveZ));
        
        bIsBeingScrolled = false;
    }
}

//----------------------------------------
void myCam::updateRotation(){
    if(bApplyInertia){
        xRot *=drag;
        yRot *=drag;
        zRot *=drag;
        
        if(ABS(xRot) <= minDifference && ABS(yRot) <= minDifference && ABS(zRot) <= minDifference){
            xRot = 0;
            yRot = 0;
            zRot = 0;
            bApplyInertia = false;
            bDoRotate = false;
        }
        curRot = glm::angleAxis(zRot, getZAxis()) * glm::angleAxis(yRot, up()) * glm::angleAxis(xRot, getXAxis());
        setPosition(curRot * (getGlobalPosition()-target.getGlobalPosition()) + target.getGlobalPosition());
        rotate(curRot);
    }else if(bDoRotate){
        curRot = glm::angleAxis(xRot, glm::vec3(1,0,0)) * glm::angleAxis(yRot, glm::vec3(0,1,0)) * glm::angleAxis(zRot, glm::vec3(0,0,1));
        
        
        //setPosition(curRot * (prevPosition-target.getGlobalPosition()) + target.getGlobalPosition());
        //setOrientation(curRot * prevOrientation);
        
    
        const ofQuaternion q_old(  prevOrientation  );
        const ofQuaternion q_delta(  curRot  );
        const ofQuaternion q_new(q_delta * q_old);
        
        // glm type conversions
        const glm::quat q_new_glm(q_new);
        
        //Change cam orientation and position
        setOrientation(q_new_glm);
        setPosition(-getLookAtDir() * getDistance() + target.getGlobalPosition());
    }
}

//----------------------------------------
void myCam::setControlArea(const ofRectangle & _controlArea) {
    controlArea = _controlArea;
}

//----------------------------------------
void myCam::clearControlArea() {
    controlArea = ofRectangle();
}

//----------------------------------------
ofRectangle myCam::getControlArea() const {
    if (controlArea.isZero()) {
        if (viewport.isZero()) {
            return getRenderer()->getCurrentViewport();
        }
        return viewport;
    }
    return controlArea;
}

//----------------------------------------
void myCam::mousePressed(ofMouseEventArgs & mouse){
    ofRectangle area = getControlArea();
    if(area.inside(mouse.x, mouse.y)){
        lastMouse = mouse;
        prevMouse = mouse;
        prevAxisX = getXAxis();
        prevAxisY = getYAxis();
        prevAxisZ = getZAxis();
        prevPosition = ofCamera::getPosition();
        prevOrientation = ofCamera::getOrientationQuat();
        prevTarget = target.getPosition();
        
        
        
        if((bEnableMouseMiddleButton && mouse.button == OF_MOUSE_BUTTON_MIDDLE) || events->getKeyPressed(doTranslationKey)  || mouse.button == OF_MOUSE_BUTTON_RIGHT){
            bDoTranslate = true;
            bDoRotate = false;
        }else if(mouse.button == OF_MOUSE_BUTTON_LEFT){
            bDoTranslate = false;
            bDoRotate = true;
            if(glm::length(glm::vec2(mouse.x - area.x - (area.width/2), mouse.y - area.y - (area.height/2))) < std::min(area.width/2, area.height/2)){
                bInsideArcball = true;
            }else{
                bInsideArcball = false;
            }
        }
        bApplyInertia = false;
    }
}

//----------------------------------------
void myCam::mouseReleased(ofMouseEventArgs & mouse){
    ofRectangle area = getControlArea();
    
    if(area.inside(mouse)){
        // Check if it's double click
        unsigned long curTap = ofGetElapsedTimeMillis();
        if(lastTap != 0 && curTap - lastTap < doubleclickTime){
            reset();
            return;
        }
        lastTap = curTap;
    }
    
    if(doInertia){
        bApplyInertia = true;
        mouseVel = mouse  - prevMouse;
        
        updateMouse(mouse);
        glm::vec2 center(area.width/2, area.height/2);
        int vFlip;
        if(isVFlipped()){
            vFlip = -1;
        }else{
            vFlip =  1;
        }
        zRot = -vFlip * glm::orientedAngle(
                                           glm::normalize(glm::vec2(mouse.x - area.x - center.x, mouse.y - area.y - center.y)),
                                           glm::normalize(prevMouse - glm::vec2(area.x, area.y) - center));
    }else{
        bDoRotate = false;
        xRot = 0;
        yRot = 0;
        zRot = 0;
        
        bDoTranslate = false;
        moveX = 0;
        moveY = 0;
        moveZ = 0;
    }
}

//----------------------------------------
void myCam::mouseDragged(ofMouseEventArgs & mouse){
    mouseVel = mouse  - lastMouse;
    
    updateMouse(mouse);
}

//----------------------------------------
void myCam::mouseScrolled(ofMouseEventArgs & mouse){
    if (doInertia) {
        bApplyInertia = true;
    }
    ofRectangle area = getControlArea();
    prevPosition = ofCamera::getGlobalPosition();
    prevAxisZ = getZAxis();
    moveZ = mouse.scrollY * 30 * sensitivityZ * (getDistance() + FLT_EPSILON)/ area.height;
    bDoScrollZoom = true;
    bIsBeingScrolled = true;
}

//----------------------------------------
void myCam::updateMouse(const ofMouseEventArgs & mouse){
    ofRectangle area = getControlArea();
    int vFlip;
    if(isVFlipped()){
        vFlip = -1;
    }else{
        vFlip =  1;
    }
    if(bDoTranslate){
        moveX = 0;
        moveY = 0;
        moveZ = 0;
        if(mouse.button == OF_MOUSE_BUTTON_RIGHT){
            moveZ = mouseVel.y * (sensitivityZ * 0.7f) * (getDistance() + FLT_EPSILON)/ area.height;
        }else{
            moveX = -mouseVel.x * (sensitivityX * 0.5f) * (getDistance() + FLT_EPSILON)/ area.width;
            moveY = vFlip * mouseVel.y * (sensitivityY* 0.5f) * (getDistance() + FLT_EPSILON)/ area.height;
        }
    }else if(bDoRotate){
        xRot = 0;
        yRot = 0;
        zRot = 0;
        if(bInsideArcball){
            xRot = vFlip * -mouseVel.y * sensitivityRotX * glm::pi<float>() / std::min(area.width, area.height);
            yRot = -mouseVel.x * sensitivityRotY * glm::pi<float>() / std::min(area.width, area.height);
        }else{
            glm::vec2 center(area.width/2, area.height/2);
            zRot = -vFlip * glm::orientedAngle(glm::normalize(glm::vec2(mouse.x - area.x - center.x, mouse.y - area.y - center.y)),
                                               glm::normalize(lastMouse - glm::vec2(area.x, area.y) - center));
            zRot *=  sensitivityRotZ;
        }
    }
}

