//
//  leapListener.cpp
//  handyCam
//
//  Created by Drake S. Harrison on 6/6/17.
//
//

#include "leapListener.hpp"

void leapListener::onConnect(const Leap::Controller& controller){
    std::cout << "Controller connected..." << std::endl;
    controller.enableGesture(Leap::Gesture::TYPE_CIRCLE);
    isRotating = false;
    isTranslating = false;
    rotationMode = true;
    translationMode = false;
    rotationQueue = avgQueue<ofQuaternion>(10);
    handInputDisabled == false;
}

void leapListener::onFrame(const Leap::Controller& controller){
    
    const Leap::Frame frame = controller.frame();
    Leap::Hand hand = frame.hands().frontmost();
    //Leap::CircleGesture circle = frame.gesture();
    //If hands are detected:
    if(frame.hands().count() > 0 && handInputDisabled == false){
        
        
/*--------------------] Translation [--------------------*/
        if(hand.pinchStrength() == 1 && translationMode == true){
            
            if(isTranslating == false) {    //Get initial position and frame
                startFrameTrans = frame;
                cam.prevPosition = cam.getGlobalPosition();
                cam.prevTarget = cam.target.getGlobalPosition();
                cam.prevOrientation = cam.getGlobalOrientation();
            }
            
            isTranslating = true;
            
            //Get translation vector and convert to ofVec3f
            Leap::Vector handMovement = hand.translation(startFrameTrans);
            ofVec3f ofHandMovement = handMovement.toVector3<ofVec3f>();
            
            //Orient to camera vs. world coordinates
            ofQuaternion ofOrientation(cam.prevOrientation);
            ofHandMovement = ofOrientation * ofHandMovement;
            glm::vec3 handMovement_glm(ofHandMovement);
          
            //Push translation vector onto queue and average
            translationQueue.push(handMovement_glm * translations_speed);
            averageTrans = translationQueue.average();
            
            //Change cam and target position
            cam.setPosition(cam.prevPosition - averageTrans);//handMovement_glm);
            cam.target.setPosition(cam.prevTarget - averageTrans);//handMovement_glm);
            
        }else{
            isTranslating = false;
            translationQueue.emptyQueue();
        }
        
        
/*--------------------] Rotation [--------------------*/
        if(hand.pinchStrength() == 1 && rotationMode == true){
            
            if (isRotating == false) {    //Get initial position and frame
                startFrameRot = frame;
                cam.prevPosition = cam.getGlobalPosition();
                cam.prevOrientation = cam.getGlobalOrientation();
            }
            
            isRotating = true;
            
            //Get rotation matrix and convert to glm::quat
            Leap::FloatArray handMatrix = hand.rotationMatrix(startFrameRot).toArray4x4();
            ofMatrix4x4 ofHandMatrix(handMatrix);
            cam.handRot.set(ofHandMatrix);
            
            //Push rotation quat onto queue, then average
            rotationQueue.push(cam.handRot);
            averageRot = rotationQueue.average();
            averageRot.normalize();
            //rotationQueue.print();
            
            const ofQuaternion q_old(    cam.prevOrientation  );
            const ofQuaternion q_delta(  averageRot  );
            const ofQuaternion q_new(q_delta.conj() * q_old);
            
            // glm type conversions
            const glm::quat q_new_glm(q_new);
            
            //Change cam orientation and position
            cam.setOrientation(q_new_glm);
            cam.setPosition(-cam.getLookAtDir() * cam.getDistance() + cam.target.getGlobalPosition());
            
        } else {
            isRotating = false;
            rotationQueue.emptyQueue();
        }
        
        //if(circle.isValid){
           
    } else {
        isRotating = false;
        isTranslating = false;
        rotationQueue.emptyQueue();
        translationQueue.emptyQueue();
    }

}

void leapListener::onExit(const Leap::Controller& controller){
    
}
