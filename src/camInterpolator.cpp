//
//  camInterpolator.cpp
//  3d_grabber
//
//  Created by Drake S. Harrison on 7/20/17.
//
//

#include "camInterpolator.hpp"

camInterpolator::camInterpolator(){
    curr_savepoint = 0;
    curr_stepnum = 0;
}

camInterpolator::camInterpolator(const vector <ofQuaternion>& inputQuats, const vector <ofVec3f>& inputVecs){
    savepoint_rots = inputQuats;
    savepoint_positions = inputVecs;
    curr_savepoint = 0;
    curr_stepnum = 0;
}

bool camInterpolator::isDone(){
    if(curr_savepoint == savepoint_rots.size()){
        return true;
    } else {
        return false;
    }
}

void camInterpolator::endInterpolation(){
    if(isDone() == false){
        curr_savepoint = savepoint_rots.size();
    }
}

void camInterpolator::nextPosition(ofQuaternion *q, ofVec3f *x){
    if(curr_stepnum == 100){
        curr_stepnum = 0;
        curr_savepoint++;
    }
    
    if(curr_savepoint != savepoint_rots.size() - 1){
        q -> slerp(curr_stepnum/float(N), savepoint_rots.at(curr_savepoint), savepoint_rots.at(curr_savepoint + 1));
        (* x) = savepoint_positions.at(curr_savepoint).getInterpolated(savepoint_positions.at(curr_savepoint + 1), curr_stepnum/float(N));
        
        curr_stepnum++;
    } else if(curr_savepoint == savepoint_rots.size() - 1){
        curr_savepoint++;
    }
    
    
}
