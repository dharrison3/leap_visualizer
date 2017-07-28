//
//  bundleReader.cpp
//  handyCam
//
//  Created by Drake S. Harrison on 6/30/17.
//
//

#include "bundleReader.hpp"
#include <cmath>

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
        
        pointSumX, pointSumY, pointSumZ = 0;
        
        //First run
        double discard;
        for(int i = 0; i < camLocs * 3 * 5; i++){
            bundleFile >> discard;
        }
        for(int i = 0; i < pointLocs; i++){
            double tempX, tempY, tempZ;
            bundleFile >> tempX >> tempY >> tempZ;
            pointSumX += tempX;
            pointSumY += tempY;
            pointSumZ += tempZ;
            
            bundleFile >> discard >> discard >> discard;
            
            int numViews;
            bundleFile >> numViews;
            for(int j = 0; j < 4 * numViews; j++){
                bundleFile >> discard;
            }
        }
        avgX = pointSumX / pointLocs;
        avgY = pointSumY / pointLocs;
        avgZ = pointSumZ / pointLocs;
        cout << "Average Point Location: " << avgX << ", " << avgY << ", " << avgZ << endl;
        
        bundleFile.close();
        
        bundleFile.open(filename);
        
        //Second run
        bundleFile.ignore(100, '\n');
        bundleFile >> camLocs;
        bundleFile >> pointLocs;
        cout << "There are " << camLocs << " camera locations and " << pointLocs << " building points." << endl;
        
        for(int i = 0; i < camLocs; i++){
            double R[9];
            
            double temp;
            ofMatrix3x3 tempRot;
            ofMatrix4x4 tempMat4;
            ofQuaternion tempQuat;
            ofVec3f tempTrans;
            bundleFile >> temp;
            if(temp != 0.0){
                camFocal.push_back(temp);
            }
            
            bundleFile >> temp >> temp;
            
            for(int j = 0; j < 9; j++){
                bundleFile >> R[j];
            }
            
            double t[3];
            
            for(int j = 0; j < 3; j++){
                bundleFile >> t[j];
            }
            
            double t_new[3];
            t_new[0] = t[0] + R[0] * avgX + R[1] * avgY + R[2] * avgZ;
            
            t_new[1] = t[1] + R[3] * avgX + R[4] * avgY + R[5] * avgZ;
            
            t_new[2] = t[2] + R[6] * avgX + R[7] * avgY + R[8] * avgZ;
            
            
            if(temp != 0.0){
                tempRot = ofMatrix3x3(R[0], R[1], R[2], R[3], R[4], R[5], R[6], R[7], R[8]);
                
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
                tempMat4(3,3) = 1.0;
                tempQuat.set(tempMat4);
                //cout << "TempQuat: " << tempQuat << endl;
                camQuats.push_back(tempQuat);
                //cout << "CamQuat: " << camQuats.back()<< endl;
                
            }
            
            
            
            if(temp != 0.0){
                tempTrans.x = t_new[0];
                tempTrans.y = t_new[1];
                tempTrans.z = t_new[2];
                camTranslations.push_back(tempTrans);
            }
            
            //bundleFile >> temp;
        }
        for(int i = 0; i < pointLocs; i++){
            double p[3];
            bundleFile >> p[0] >> p[1] >> p[2];
            
            int rtemp, gtemp, btemp;
            bundleFile >> rtemp >> gtemp >> btemp;
            ifColor = ofColor(rtemp, gtemp, btemp);
            
            int numViews;
            double tmp;
            bundleFile >> numViews;
            for (int j = 0; j < 4 * numViews; j++) {
                bundleFile >> tmp;
            }
            
            ifPoint.x = p[0] - avgX;
            ifPoint.y = p[1] - avgY;
            ifPoint.z = p[2] - avgZ;
            
            points.push_back(ifPoint);
            colors.push_back(ifColor);
        }
        
        point_scale = 0;
        for (int i = 0; i < points.size(); ++i) {
            point_scale += points[i].length();
        }
        point_scale /= points.size();
        
        
        
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

ofVec3f bundleReader::getCamLocations(int i){
    ofVec3f result;
    ofVec3f T = camTranslations.at(i);
    ofMatrix3x3 rTranspose = ofMatrix3x3(camRotations.at(i));
    rTranspose.transpose();
    
    result.x = -(T.x * rTranspose.a + T.y * rTranspose.b + T.z * rTranspose.c);
    result.y = -(T.x * rTranspose.d + T.y * rTranspose.e + T.z * rTranspose.f);
    result.z = -(T.x * rTranspose.g + T.y * rTranspose.h + T.z * rTranspose.i);
    
    return result;
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
