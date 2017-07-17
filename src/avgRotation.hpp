

#include <stdio.h>
#include <cmath>
#include <ofMain.h>

class avgRotation {
public:

    avgRotation();
    
    void computeAverageRotation(const vector<glm::quat> & rotations, glm::quat * result);

private:
    //glm::vec3 axis;
    
};

