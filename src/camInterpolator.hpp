//
//  camInterpolator.hpp
//  3d_grabber
//
//  Created by Drake S. Harrison on 7/20/17.
//
//

#include <stdio.h>
#include <ofMain.h>

class camInterpolator {
    
public:
    camInterpolator();
    
    camInterpolator(const vector <ofQuaternion>& inputQuats, const vector <ofVec3f>& inputVecs);
    
    bool isDone();
    
    void endInterpolation();
    
    void nextPosition(ofQuaternion *q, ofVec3f *x);
    
private:
    vector<ofQuaternion> savepoint_rots;
    vector<ofVec3f> savepoint_positions;
    int curr_savepoint; //basically i
    int curr_stepnum;  //basically j
    int N = 100;
};
