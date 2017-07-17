//
//  bundleReader.cpp
//  handyCam
//
//  Created by Drake S. Harrison on 6/30/17.
//
//

#include "bundleReader.hpp"

bundleReader::bundleReader(){
    
}

bundleReader::bundleReader(const string& filename){
    ifstream bundleFile;
    ofVec3f ifPoint;
    ofColor ifColor;
    
    bundleFile = ifstream(filename);
    if(bundleFile.is_open()){
        bundleFile.ignore(100, '\n');
        bundleFile >> camLocs;
        bundleFile >> pointLocs;
        cout << "There are " << camLocs << " camera locations and " << pointLocs << " building points." << endl;
        
        
        
        for(int i = 0; i < camLocs; i++){
            double temp;
            ofMatrix3x3 tempRot;
            ofMatrix4x4 tempMat4;
            ofQuaternion tempQuat;
            ofVec3f tempTrans;
            bundleFile >> temp;
            camFocal.push_back(temp);
            bundleFile >> temp >> temp;
            
            bundleFile >> tempRot.a >> tempRot.b >> tempRot.c >> tempRot.d >>
            tempRot.e >> tempRot.f >> tempRot.g >> tempRot.h >> tempRot.i;
            camRotations.push_back(tempRot);
            
            //Convert to quaternion
            tempMat4(0,0) = tempRot.a;
            tempMat4(0,1) = tempRot.b;
            tempMat4(0,2) = tempRot.c;
            tempMat4(1,0) = tempRot.d;
            tempMat4(1,1) = tempRot.e;
            tempMat4(1,2) = tempRot.f;
            tempMat4(2,0) = tempRot.g;
            tempMat4(2,1) = tempRot.h;
            tempMat4(2,2) = tempRot.i;
            tempQuat.set(tempMat4);
            //cout << "TempQuat: " << tempQuat << endl;
            camQuats.push_back(tempQuat);
            //cout << "CamQuat: " << camQuats.back()<< endl;
            
            bundleFile >> tempTrans.x >> tempTrans.y >> tempTrans.z;
            camTranslations.push_back(tempTrans);
            
            //bundleFile >> temp;
        }
        for(int i = 0; i < pointLocs; i++){
            bundleFile >> ifPoint.x >> ifPoint.y >> ifPoint.z;
            int rtemp, gtemp, btemp;
            bundleFile >> rtemp >> gtemp >> btemp;
            ifColor = ofColor(rtemp, gtemp, btemp);
            int numViews;
            double tmp;
            bundleFile >> numViews;
            for (int j = 0; j < 4 * numViews; ++j) {
                bundleFile >> tmp;
            }
            
            points.push_back(ifPoint);
            colors.push_back(ifColor);
        }
        
        
        
    }else{
        cout << "File not opened." << endl;
    }
}

bundleReader::~bundleReader(){
    
}

ofVec3f bundleReader::getPoint(int i){
    return points.at(i);
}

ofColor bundleReader::getColor(int i){
    return colors.at(i);
}

double bundleReader::getCamFocal(int i){
    return camFocal.at(i);
}

ofMatrix3x3 bundleReader::getCamRotations(int i){
    return camRotations.at(i);
}

glm::quat bundleReader::getCamQuats(int i){
    return camQuats.at(i);
}

ofVec3f bundleReader::getCamTranslations(int i){
    return camTranslations.at(i);
}

void bundleReader::printCams(int num){
    
    for(int i = 0; i < camLocs; i++){
        cout << "Focal length: " << camFocal.at(i) << endl;
        cout << "Rotations: " << camRotations.at(i) << endl;
        cout << "Translations: " << camTranslations.at(i) << endl << endl;
    }
    
    cout << "Focal length(back): " << camFocal.back() << endl;
    cout << "Rotations(back): " << camRotations.back() << endl;
    cout << "Translations(back): " << camTranslations.back() << endl << endl;

}
