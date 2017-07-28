#include "ofApp.h"
#include "avgRotation.hpp"
#include <iostream>
#include <limits>
#include <algorithm>

//--------------------------------------------------------------
void ofApp::setup(){

    vertices = NULL;
    colors = NULL;
    
#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif
    
    // ----------------------------------------------------------------------------
    // This makes relative paths work in C++ in Xcode by changing directory to the Resources folder inside the .app bundle
#ifdef __APPLE__
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        // error!
    }
    CFRelease(resourcesURL);
    
    chdir(path);
    std::cout << "Current Path: " << path << std::endl;
#endif
    // ----------------------------------------------------------------------------
    
    ofBackground(0, 0, 0);
    
    ofEnableAlphaBlending();
    ofSetVerticalSync(true);
    
    myFont.load("Ubuntu-M.ttf", 12);
    
    controller.addListener(listener);
    bundleFile = "../../../data/notredame.out";
    setupNewModel();
    
}

//--------------------------------------------------------------
void ofApp::setupNewModel(){
    
    bundle = bundleReader(bundleFile);

    if(vertices!= NULL){
        delete[] vertices;
    }
    if(colors != NULL){
        delete [] colors;
    }
    
    vertices = new GLfloat[bundle.numPoints() * 6 * 3];
    colors = new GLubyte[bundle.numPoints() * 6 * 4];
   
    pointSize = bundle.point_scale / 100;
    camSize = bundle.point_scale / 100;

    
    //Set initial camera position and rotation to the average of the bundle cameras
    computeAverageRotation(bundle.camQuats, &averageCamRot);
    
    avgCamPos = avgQueue<ofVec3f> (bundle.numCams());
    for(int i = 0; i < bundle.numCams(); i++){
        avgCamPos.push(bundle.getCamLocations(i));
    }
    averageCamRot = glm::conjugate(averageCamRot);
    listener.cam.setPosition(avgCamPos.average());
    
    //listener.cam.setOrientation(averageCamRot );
    
    
    listener.cam.target.setPosition(ofVec3f(0,0,0));
    listener.cam.setNearClip(bundle.point_scale / 1000);
    listener.cam.setFarClip(bundle.point_scale * 1000);
    listener.translations_speed = bundle.point_scale / 100;
    
    glm::vec3 up_vec = glm::vec3(bundle.getCamRotations(0)[3], bundle.getCamRotations(0)[4], bundle.getCamRotations(0)[5]);
    listener.cam.lookAt(listener.cam.target, up_vec);
    
    billboardingOrientation = ofQuaternion(listener.cam.getOrientationQuat());
    updateDrawingLists();
}

//--------------------------------------------------------------
void ofApp::updateDrawingLists() {
    glMatrixMode(GL_MODELVIEW);
    
    pointAndCamsList = glGenLists(1);
    glNewList(pointAndCamsList, GL_COMPILE);
    
    //enable blending for prettiness
    glEnable(GL_BLEND);
    //glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    int numb = bundle.numPoints();
    
    glPointSize(1);
    
    updateQuadCoords();
    updateQuadColors();
    
    // activate and specify pointer to vertex array
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLES, 0, numb * 6);
    
    // deactivate vertex and color arrays after drawing
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    
    // Draw all the cameras
    for(int i = 0; i < bundle.numCams(); i++){
        glPushMatrix();
        
        glColor3f(255, 255, 255);
        
        //glScalef(scale1, scale1, scale1);
        
        ofMatrix4x4 transform;
        
        //bundle.getCamRotations(i).transpose();
        float s = camSize;
        int count = 0;
        for(int j = 0; j < 4; j++){
            for(int k = 0; k < 4; k++){
                if(j < 3 && k < 3){
                    transform(j, k) = bundle.getCamRotations(i)[count];
                    count++;
                } else if(j == 3 && k == 3){
                    transform(j, k) = 1;
                } else {
                    transform(j, k) = 0;
                }
            }
        }
        glMultMatrixf(transform.getPtr());
        
        glTranslatef(-bundle.getCamTranslations(i).x, -bundle.getCamTranslations(i).y, -bundle.getCamTranslations(i).z);
        
        glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(-s, -s, -s * 2.5);
        glVertex3f(0, 0, 0);
        glVertex3f(s, -s, -s * 2.5);
        glVertex3f(0, 0, 0);
        glVertex3f(s, s, -s * 2.5);
        glVertex3f(0, 0, 0);
        glVertex3f(-s, s, -s * 2.5);
        
        glVertex3f(-s, -s, -s * 2.5);
        glVertex3f(s, -s, -s * 2.5);
        
        glVertex3f(s, -s, -s * 2.5);
        glVertex3f(s, s, -s * 2.5);
        
        glVertex3f(s, s, -s * 2.5);
        glVertex3f(-s, s, -s * 2.5);
        
        glVertex3f(-s, s, -s * 2.5);
        glVertex3f(-s, -s, -s * 2.5);
        
        glEnd();
        glPopMatrix();
    }
    
    glEndList();
}

//--------------------------------------------------------------
void ofApp::updateQuadColors(){
    for(int i = 0; i < bundle.numPoints(); i++){
        int color_offset = 24; //6 vertices with rgba
        for(int j = 0; j < 6; j++){
            colors[i * color_offset + j*4] = bundle.getColor(i).r;
            colors[i * color_offset + j*4 + 1] = bundle.getColor(i).g;
            colors[i * color_offset + j*4 + 2] = bundle.getColor(i).b;
            colors[i * color_offset + j*4 + 3] = 128;
        }
    }
}

//--------------------------------------------------------------
void ofApp::updateQuadCoords() {
    
    //--------------------------------------------------------------
    /*
    // Problem: This sorting method doesn't sort the point colors, only their
    // coordinates. That's not right! Instead, create a vector<int> of depth
    // order indices. Sort the indices, not the vectors themselves.
    class comparatorClass {
    public:
        comparatorClass();
        comparatorClass(leapListener& _listener) : listener(_listener) {
            lookatdir = listener.cam.getLookAtDir();
        }
        
        bool operator() (const ofVec3f & first, const ofVec3f & second) const {
            double depth1 = lookatdir.dot(first);
            double depth2 = lookatdir.dot(second);

            if(depth1 < depth2){
                return true;
            } else {
                return false;
            }
        }
        
    private:
        ofVec3f lookatdir;
        leapListener& listener;
    } ;
    comparatorClass comparatorObject(listener);
    
   std::stable_sort(bundle.points.begin(), bundle.points.end(), comparatorObject);
     */
    vector<ofVec3f> quadCorners = {
        ofVec3f(- pointSize / 2, - pointSize / 2, 0),
        ofVec3f( - pointSize / 2, pointSize - pointSize / 2, 0),
        ofVec3f(pointSize - pointSize / 2, pointSize - pointSize / 2, 0),
        ofVec3f(- pointSize / 2, - pointSize / 2, 0),
        ofVec3f(pointSize - pointSize / 2, pointSize - pointSize / 2, 0),
        ofVec3f(pointSize - pointSize / 2, - pointSize / 2, 0)
    };
    
    int offset = quadCorners.size() * 3;
    for(int i = 0; i < bundle.numPoints(); ++i) {
        for (int j = 0; j < quadCorners.size(); ++j) {
            
            rotatedCorner = billboardingOrientation * quadCorners[j];
            
            transformedCorner = rotatedCorner + bundle.getPoint(i);

            vertices[i * offset + j * 3 + 0] = transformedCorner.x;
            vertices[i * offset + j * 3 + 1] = transformedCorner.y;
            vertices[i * offset + j * 3 + 2] = transformedCorner.z;
        }
    }
}




//--------------------------------------------------------------
void ofApp::update(){
    stringstream strm;
    strm << "fps: " << ofGetFrameRate();
    ofSetWindowTitle(strm.str());
    

    if(!interpolate.isDone()){
        listener.cam.disableMouseInput();
        listener.handInputDisabled = true;
        isInterpolating = true;
        resumePos = true;
        
        billboardingOrientation = ofQuaternion(listener.cam.getOrientationQuat());

        updateDrawingLists();
        interpolate.nextPosition(& mvQuat, & mvVec);
        listener.cam.setPosition(mvVec);
        listener.cam.setOrientation(mvQuat);
        
    } else {
        isInterpolating = false;
        if(resumePos == true){
            listener.cam.setPosition(savedCamPos);
            listener.cam.setOrientation(savedCamQuat);
        }
        
        listener.cam.enableMouseInput();
        listener.handInputDisabled = false;
        resumePos = false;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    listener.cam.begin();
    
    // Draw the point quads and cam lines
    glPushMatrix();
    glPointSize(1);
    glCallList(pointAndCamsList);
    glPopMatrix();
    
    double rtcScale = bundle.point_scale / 30;
    //Create target reticule
    if(reticleOn == true && isInterpolating == false){
        glDisable(GL_LIGHTING);
        glDisable(GL_BLEND);
        ofNoFill();
        ofBeginShape();
        ofSetColor(240, 60, 40);
        int N = 100;
        for(int i = 0; i <= N; i++){
            ofVertex(rtcScale * cos((2 * PI * i) / N) + listener.cam.getTarget().getX(), rtcScale * sin((2 * PI * i) / N) + listener.cam.getTarget().getY(), listener.cam.getTarget().getZ());
        }
        ofEndShape();
        
        ofBeginShape();
        ofSetColor(40, 160, 240);
        for(int i = 0; i <= N; i++){
            ofVertex(listener.cam.getTarget().getX(), rtcScale * cos((2 * PI * i) / N) + listener.cam.getTarget().getY(), rtcScale * sin((2 * PI * i) / N) + listener.cam.getTarget().getZ());
        }
        ofEndShape();
        
        ofBeginShape();
        ofSetColor(2, 190, 50);
        for(int i = 0; i <= N; i++){
            ofVertex(rtcScale * cos((2 * PI * i) / N) + listener.cam.getTarget().getX(), listener.cam.getTarget().getY(), rtcScale * sin((2 * PI * i) / N) + listener.cam.getTarget().getZ());
        }
        ofEndShape();
        ofFill();
        glEnable(GL_BLEND);
    }
    
    
    //int material = 7;
    
    glDisable(GL_LIGHTING);
    
    if(isInterpolating == true){
        stringstream filename;
        filename << "frame_" << frameCounter << ".png";
        img.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
        img.save(filename.str(), OF_IMAGE_QUALITY_MEDIUM);
        frameCounter++;
    }
    
    listener.cam.end();
    
    ofTranslate(-ofGetWidth()/2, -ofGetHeight()/2);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_BUFFER);
    
    // UI elements
    if(infoPanelOn == true && isInterpolating == false) {
        ofEnableAlphaBlending();
        ofSetColor(14, 67, 111, 200);
        ofDrawRectRounded(ofGetWidth() / 2 - 970 / 2, 7, 970 , 35, 17);
        ofDrawRectRounded(ofGetWidth() / 2 - 970 / 2, ofGetHeight() - 7 - 35, 970 , 35, 17);

        ofDrawRectRounded(27, 50, 255, 60, 17);
        ofDrawRectRounded(ofGetWidth() - 283, 50, 255, 60, 17);
        
        ofDisableAlphaBlending();
        
      
        ofSetColor(255,255,255);
        ostringstream infoBox1;
        infoBox1 << "Press s to save current camera. \n"
        "Press c to clear camera list. \n"
        "Number of items stored: " << camVecCaptured.size();
        myFont.drawString(infoBox1.str(), 40, 67);
        
        ostringstream infoBox2;
        infoBox2 << "Press m to record fly-through. \n"
        "\n"
        "Press l to load a new model.";
        myFont.drawString(infoBox2.str(), ofGetWidth() - 271, 67);
        
        myFont.drawString("Press b to orient points to camera. Use up/down keys to change point size. Use left/right keys to change camera size." , ofGetWidth()/2 - myFont.stringWidth("Press b to orient points to camera. Use up/down keys to change point size. Use left/right keys to change camera size.") / 2, ofGetHeight() - myFont.stringHeight("Press b to orient points to camera. Use up/down arrow keys to change point size. Use left/right keys to change camera size."));
        
        if(listener.translationMode == false){
            myFont.drawString("Currently in Rotation mode. To change modes, press 1 (rotation), 2 (translation), or 3 (combined). To toggle this UI, press x.", ofGetWidth()/2 - myFont.stringWidth("Currently in Rotation mode. To change modes, press 1 (rotation), 2 (translation), or 3 (combined). To toggle this UI, press x.") / 2, 30);
        } else if(listener.rotationMode == false){
            myFont.drawString("Currently in Translation mode. To change modes, press 1 (rotation), 2 (translation), or 3 (combined). To toggle this UI, press x.", ofGetWidth()/2 - myFont.stringWidth("Currently in Translation mode. To change modes, press 1 (rotation), 2 (translation), or 3 (combined). To toggle this UI, press x.") / 2, 30);
        } else if(listener.rotationMode && listener.translationMode == true){
            myFont.drawString("Currently in Combined mode. To change modes, press 1 (rotation), 2 (translation), or 3 (combined). To toggle this UI, press x.", ofGetWidth()/2 - myFont.stringWidth("Currently in Combined mode. To change modes, press 1 (rotation), 2 (translation), or 3 (combined). To toggle this UI, press x.") / 2, 30);
        }
        
        
        
    }

    
}

//--------------------------------------------------------------
void ofApp::exit(){
    
    controller.removeListener(listener);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == '1'){
        listener.rotationMode = true;
        listener.translationMode = false;
        if(listener.rotationMode == true){
            cout << "Rotation On" << endl;
        }
    } else if(key == '2'){
        listener.translationMode = true;
        listener.rotationMode = false;
        if(listener.translationMode == true){
            cout << "Translation On" << endl;
        }
    } else if(key == '3'){
        listener.translationMode = true;
        listener.rotationMode = true;
        cout << "Rotation + Translation On" << endl;
    }
    
    if(key == 'x'){
        if(infoPanelOn == true){
            infoPanelOn = false;
            reticleOn = false;
        } else if(infoPanelOn == false){
            infoPanelOn = true;
            reticleOn = true;
        } else if(isInterpolating == true){
            interpolate.endInterpolation();
        }
    }
    
    if(key == 's'){
        camVecCaptured.push_back(listener.cam.getPosition());
        camQuatCaptured.push_back(listener.cam.getOrientationQuat());
    }
    
    if(key == 'c'){
        camVecCaptured.clear();
        camQuatCaptured.clear();
    }
    
    if(key == 'm'){
        savedCamPos = listener.cam.getPosition();
        savedCamQuat = listener.cam.getOrientationQuat();
        interpolate = camInterpolator(camQuatCaptured, camVecCaptured);
    }
    
    if(key == 'l'){
        ofFileDialogResult openFileResult= ofSystemLoadDialog("Select a bundle file to load: ");
        if(openFileResult.bSuccess){
            camVecCaptured.clear();
            camQuatCaptured.clear();
            
            bundleFile = openFileResult.getPath();
            cout << bundleFile << endl;
            setupNewModel();
        }
    }
    
    if(key == OF_KEY_UP){
        pointSize *= pointSize_scaleFactor;
        updateDrawingLists();
    }
    if(key == OF_KEY_DOWN){
        pointSize /= pointSize_scaleFactor;
        updateDrawingLists();
    }
    
    if(key == OF_KEY_RIGHT){
        camSize *= camSize_scaleFactor;
        updateDrawingLists();
    }
    if(key == OF_KEY_LEFT){
        camSize /= camSize_scaleFactor;
        updateDrawingLists();
    }
    
    if(key == 'b'){
        billboardingOrientation = ofQuaternion(listener.cam.getOrientationQuat());
        updateDrawingLists();
    }
    
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
