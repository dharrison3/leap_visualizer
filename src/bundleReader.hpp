//
//  bundleReader.hpp
//  handyCam
//
//  Created by Drake S. Harrison on 6/30/17.
//
//

#include <stdio.h>
#include <ofMain.h>
#include <fstream>

class bundleReader {
public:
    
    //Default Constructor
    bundleReader();
    
    //Constructor
    bundleReader(const string& filename);
    
    //Destructor
    ~bundleReader();
    
    //Return point at back of vector and pop
    ofVec3f getPoint(int i);
    
    //Return color at back of vector and pop
    ofColor getColor(int i);
    
    //Return focal length at i
    double getCamFocal(int i);
    
    //Return rotation matrix at i
    ofMatrix3x3 getCamRotations(int i);
    
    glm::quat getCamQuats(int i);
    
    //Return translation vector at i
    ofVec3f getCamTranslations(int i);
    
    
    
    //Print camera info from last to first
    void printCams(int num);
   
    vector<double> camFocal;
    vector<ofMatrix3x3> camRotations;
    vector<ofVec3f> camTranslations;
    vector<glm::quat> camQuats;
    
    vector<ofVec3f> points;
    
    vector<ofColor> colors;
    
    int camLocs;
    
    int pointLocs;

private:
    
    
    
    
};
