// Compute the geodesic (L2) mean of a list of rotation matrices. This is also
// known as the Karcher mean. There is a closed-form version of this
// computation, but it involves solving an eigenvalue problem. This
// implementation is built on an interated tangent space approximation, and
// does not require any extra dependencies.
//
// The resulting rotation S is the minimizer of
//    sum_i  d(R_i, S)^2
// Where d is the (natural) geodesic distance and {R_i} are the inputs.
//
// # Theory:
// # sum_i  d(R_i, S)^2 = sum_i  d(S^T * R_i, I)^2
// #                    = sum_i 0.5 || log [S^T * R_i] ||^2
// #                    = 0.5 * sum_i || log_S^T [R_i] ||^2
// # At the fixed point, this has the property that
// # sum_i log_S^T [R_i] = 0.
// # This solver searches for such a point with this fixed point iteration:
// # ST^{k+1} = exp_ST^{k} sum_i log_ST^{k} R_i

#include "avgRotation.hpp"

avgRotation::avgRotation(){
    
};

void avgRotation::computeAverageRotation(const vector<glm::quat> & rotations, glm::quat * result){
    
    const int MAX_ITERATIONS = 50;     // Only run this many iterations
    const double RESIDUAL_TOL = 1e-5; // Stop if the size of the improvement
    // is this small or smaller
    const bool DEBUG_MODE = true;      // Show debugging output
    
    if (DEBUG_MODE) {
        std::cout << "[computeAverageRotation] Beginning to average " <<
        rotations.size() << " rotations." << std::endl;
    }
    
    // Check for silly inputs
    if (rotations.size() == 0) {
        if (DEBUG_MODE) {
            std::cout << "[computeAverageRotation] Nothing to do." << std::endl;
        }
        return;
    }

    
    // Initial guess: the identity rotation.
    // We'll store the transpose of the answer, and then flip it at the end
    glm::quat result_T = glm::quat(1, 0, 0, 0);
    
    // Main iteration loop: do this until convergence or until too many
    // iterations have happened.
    int current_iteration = 1;
    double update_size = 10000000; // stupid big number
    while (current_iteration <= MAX_ITERATIONS && update_size > RESIDUAL_TOL) {
        
        // accumlate the tangent space average in this variable
        glm::vec3 tAvg(0, 0, 0);
        for (int i = 0; i < rotations.size(); ++i) {
            
            glm::quat residual = result_T * rotations[i];

            double angle = glm::angle(residual);
            glm::vec3 axis = glm::axis(residual);
            
            //std::cout << "res: " << residual << ".   angle: " << angle << std::endl;
            
            while (angle > 3.1415926535) {
                angle -= 3.1415926535;
            }
            while (angle > 3.1415926535 / 2.0){
                angle = 3.1415926535 - angle;
                axis = -axis;
            }
            tAvg += angle * axis;
        }
        tAvg /= rotations.size();
        // decompose the new (tangent space) estimate of the average into an
        // angle and an axis
        double angle = std::sqrt(tAvg[0] * tAvg[0] +
                                 tAvg[1] * tAvg[1] +
                                 tAvg[2] * tAvg[2]
                                 );
        glm::vec3 axis;
        if (angle > 0)
            axis = tAvg / angle;
        else
            axis = glm::vec3(0, 0, 0);
        
        while (angle > 3.1415926535) {
            angle -= 3.1415926535;
        }
        while (angle > 3.1415926535 / 2.0){
            angle = 3.1415926535 - angle;
            axis = -axis;
        }
        
        // form the new estimate of the average rotation
        result_T *= glm::quat(angle, axis);
        result_T = glm::normalize(result_T);
        
        if (DEBUG_MODE) {
            std::cout << "[computeAverageRotation] after round " <<
            current_iteration << " the update size was " << angle <<
            " radians. " << std::endl;
        }
        
        // Update the convergence condition variables
        ++current_iteration;
        update_size = angle;
    }
    
    if (DEBUG_MODE) {
        if (update_size <= RESIDUAL_TOL)
            std::cout << "[computeAverageRotation] Finished to convergence." << std::endl;
        else
            std::cout << "[computeAverageRotation] Finished after " <<
            MAX_ITERATIONS << " steps without convergence." << std::endl;
    }
    
    // Stuff the answer
    (*result) = glm::conjugate(result_T);
    
}
