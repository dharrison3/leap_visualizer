#include "ofApp.h"
#include "avgRotation.hpp"
#include <iostream>

int N = 100;
ofFbo fb;

//lights
GLfloat lightOnePosition[] = {0.0, 300.0, 0.0, 1.0};
GLfloat lightOneColor[] = {0.99, 0.99, 0.99, 1.0};

//Gold material properties
GLfloat oambient[] = {.24725,.1995,.0745,1};
GLfloat odiffuse[] = {.75164,.60648,.22648,1};
GLfloat ospecular[] = {.628281,.555802,.366065,1};
GLfloat oshininess[] = {51.2};

GLUquadricObj *qobj;
GLuint pointList;

GLfloat vertices[127431*6*3];
GLubyte colors[127431*6*4];

GLfloat modelView[16];

//--------------------------------------------------------------
void ofApp::setup(){

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
    
    
    ofBackground(0,0,0);
    
    ofEnableAlphaBlending();
    ofSetVerticalSync(true);
    
    myFont.load("Ubuntu-M.ttf", 12);
    
    //some model / light stuff
    //ofEnableDepthTest();
    glShadeModel (GL_SMOOTH);
    
    
    /* initialize lighting */
    glLightfv (GL_LIGHT0, GL_POSITION, lightOnePosition);
    glLightfv (GL_LIGHT0, GL_DIFFUSE, lightOneColor);
    glEnable (GL_LIGHT0);
    glEnable (GL_LIGHTING);
    
    /*glColorMaterial (GL_FRONT, GL_DIFFUSE);
     glEnable (GL_COLOR_MATERIAL);
     glMaterialfv(GL_FRONT, GL_DIFFUSE, odiffuse);
     glMaterialfv(GL_FRONT, GL_AMBIENT, oambient);
     glMaterialfv(GL_FRONT, GL_SPECULAR, ospecular);
     glMaterialfv(GL_FRONT, GL_SHININESS, oshininess);
     glEnable(GL_NORMALIZE);*/
    
    controller.addListener(listener);
    
    
    bundle = bundleReader("../../../data/notredame.out");
    
    
    computeAverageRotation(bundle.camQuats, &averageCamRot);
    averageCamRot = averageCamRot * glm::quat(0,0,0,1);

    listener.cam.setOrientation(averageCamRot );
    listener.cam.target.setOrientation(averageCamRot);
    
        //set distance from cam to target
    listener.cam.setDistance(300.0);

    qobj = gluNewQuadric();
    GLint pm;
    glGetIntegerv(GL_POINT_SIZE_MAX, &pm);
    
    cout << "Point size max: " << pm << endl;
    
    glMatrixMode(GL_MODELVIEW);
    
    pointList = glGenLists(1);
    glNewList(pointList, GL_COMPILE);
    
    //enable blending for prettiness
    glEnable(GL_BLEND);
    //glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    int numb = bundle.pointLocs;
    //numb = 100;
    
    glPointSize(1);
    double scale1 = 500;
    double scale2 = 2.5;
    double s = 0.008;
    
    for(int i = 0; i < bundle.camLocs; i++){
        glPushMatrix();
        
        glColor3f(255, 255, 255);
        
        glScalef(scale1, scale1, scale1);
        
        ofMatrix4x4 transform;
        
        bundle.getCamRotations(i).transpose();
        
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
    
    for(int i = 0; i < numb; i++){
        int offset = 18; // 6 vertexes
        vertices[i * offset + 0] = bundle.getPoint(i).x * scale1;
        vertices[i * offset + 1] = bundle.getPoint(i).y * scale1;
        vertices[i * offset + 2] = bundle.getPoint(i).z * scale1;
        
        vertices[i * offset + 3] = bundle.getPoint(i).x * scale1;
        vertices[i * offset + 4] = bundle.getPoint(i).y * scale1 + scale2;
        vertices[i * offset + 5] = bundle.getPoint(i).z * scale1;
        
        vertices[i * offset + 6] = bundle.getPoint(i).x * scale1 + scale2;
        vertices[i * offset + 7] = bundle.getPoint(i).y * scale1 + scale2;
        vertices[i * offset + 8] = bundle.getPoint(i).z * scale1;
        
        vertices[i * offset + 9] = bundle.getPoint(i).x * scale1;
        vertices[i * offset + 10] = bundle.getPoint(i).y * scale1;
        vertices[i * offset + 11] = bundle.getPoint(i).z * scale1;
        
        vertices[i * offset + 12] = bundle.getPoint(i).x * scale1 + scale2;
        vertices[i * offset + 13] = bundle.getPoint(i).y * scale1 + scale2;
        vertices[i * offset + 14] = bundle.getPoint(i).z * scale1;
        
        vertices[i * offset + 15] = bundle.getPoint(i).x * scale1 + scale2;
        vertices[i * offset + 16] = bundle.getPoint(i).y * scale1;
        vertices[i * offset + 17] = bundle.getPoint(i).z * scale1;
        
        int color_offset = 24; //6 vertices with rgba
        for(int j = 0; j < 6; j++){
            colors[i * color_offset + j*4] = bundle.getColor(i).r;
            colors[i * color_offset + j*4 + 1] = bundle.getColor(i).g;
            colors[i * color_offset + j*4 + 2] = bundle.getColor(i).b;
            colors[i * color_offset + j*4 + 3] = 128;
            
        }
    }
    
    
    
    
    // activate and specify pointer to vertex array
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLES, 0, numb * 4);
    
    /*
     for(int i = 0; i < numb; i++){
     glPushMatrix();
     glScalef(500,500,500);
     glTranslatef(bundle.getPoint(i).x, bundle.getPoint(i).y, bundle.getPoint(i).z);
     glScalef(.005, .005, .005);
     glColor4ub(bundle.getColor(i).r, bundle.getColor(i).g, bundle.getColor(i).b, 128);
     
     // draw a square
     glDrawArrays(GL_TRIANGLES, 0, 3);
     
     
     glPopMatrix();
     }
     */
    // deactivate vertex and color arrays after drawing
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    glEndList();
    
}

//--------------------------------------------------------------
void ofApp::update(){
    stringstream strm;
    strm << "fps: " << ofGetFrameRate();
    ofSetWindowTitle(strm.str());
}

//--------------------------------------------------------------
void ofApp::draw(){
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    
    
    listener.cam.begin();
    
    

    double rtcScale = 15;
    //Create target reticule
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    ofNoFill();
    ofBeginShape();
    ofSetColor(240, 60, 40);
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
    
    //int material = 7;
    
    glDisable(GL_LIGHTING);
    
    /*glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
     glBegin(GL_TRIANGLES);
     glVertex3f(0, 200, 100);
     glVertex3f(-100, -50, 100);
     glVertex3f(100, -50, 100);
     glEnd();
     */
    
    //glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
    //glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    
    //ofEnableAlphaBlending();
    //ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    //fb.begin();
    glPushMatrix();
    
    /*
     //Billboarding
     glGetFloatv (GL_MODELVIEW_MATRIX, modelView);
     for(int i=0; i<3; i++ )
     for(int j=0; j<3; j++ ) {
     if ( i==j )
     modelView[i*4+j] = 1.0; ///notice a 2D->1D conversion
     else
     modelView[i*4+j] = 0.0;
     }
     glLoadMatrixf(modelView); //put our new matrix in as the modelview
     */
    
    glPointSize(1);
    glCallList(pointList);
    glPopMatrix();
    //fb.end();
    //ofDisableAlphaBlending();
    //fb.draw(0,0);
    
    /* for(int i = 0; i < bundle.pointLocs; i++){
     glPushMatrix();
     glColor3ub(bundle.getColor(i).r, bundle.getColor(i).g, bundle.getColor(i).b);
     double scale_factor = 200.0;
     glScalef(scale_factor,scale_factor,scale_factor);
     glBegin(GL_POINTS);
     glPointSize(1);
     glVertex3f(bundle.getPoint(i).x, bundle.getPoint(i).y, bundle.getPoint(i).z);
     
     //         glTranslatef(bundle.getPoint(i).x, bundle.getPoint(i).y, bundle.getPoint(i).z);
     //       gluSphere(qobj, 0.5f / scale_factor, 6, 6);
     glEnd();
     glPopMatrix();
     }*/
    
    /*glMaterialfv(GL_FRONT, GL_DIFFUSE, odiffuse);
     glMaterialfv(GL_FRONT, GL_AMBIENT, oambient);
     glMaterialfv(GL_FRONT, GL_SPECULAR, ospecular);
     glMaterialfv(GL_FRONT, GL_SHININESS, oshininess);
     
     glMaterialfv(GL_FRONT, GL_DIFFUSE, mats[material][0]);
     glMaterialfv(GL_FRONT, GL_AMBIENT, mats[material][1]);
     glMaterialfv(GL_FRONT, GL_SPECULAR, mats[material][2]);
     glMaterialfv(GL_FRONT, GL_SHININESS, mats[material][3]);
     
     glLightfv (GL_LIGHT0, GL_POSITION, lightOnePosition);
     
     ofSetColor(255 * mats[material][1][0], 255 * mats[material][1][1], 255 * mats[material][1][2]);
     
     
     for(int z = -200; z <= 200; z += 200){
     for(int y = - 200; y <= 200; y += 200){
     for(int x = -200; x <= 200; x += 200){
     ofDrawSphere(x, y, z, 80);
     }
     }
     }*/
    
    
    listener.cam.end();
    
    ofTranslate(-ofGetWidth()/2, -ofGetHeight()/2);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_BUFFER);
    
    if(toggleBar == true) {
        ofEnableAlphaBlending();
        ofSetColor(31, 96, 149, 200);
        ofDrawRectRounded(30, 7, ofGetWidth() - 60, 35, 17);
        ofDisableAlphaBlending();
        ofSetColor(255,255,255);
        if(listener.translationMode == false){
            myFont.drawString("Currently in Rotation mode. To change modes, press 1 (rotation), 2 (translation), or 3 (combined). To hide this panel, press x.", 48, 30);
        } else if(listener.rotationMode == false){
            myFont.drawString("Currently in Translation mode. To change modes, press 1 (rotation), 2 (translation), or 3 (combined). To hide this panel, press x.", 37, 30);
        } else if(listener.rotationMode && listener.translationMode == true){
            myFont.drawString("Currently in Combined mode. To change modes, press 1 (rotation), 2 (translation), or 3 (combined). To hide this panel, press x.", 45, 30);
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
    
    
    if(key == 'b'){
        cout << "Point location: " << bundle.getPoint(0) << endl;
        cout << "Point color: " << bundle.getColor(0) << endl << endl;
        
    }
    
    if(key == 'x'){
        if(toggleBar == true){
            toggleBar = false;
        } else if(toggleBar == false){
            toggleBar = true;
        }
    }
    
    if(key == 'c'){
        img.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
        img.save("screenshot.png");
        cout << "Screen captured." << endl;
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
