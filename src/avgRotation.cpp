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
#include "rotation.h"
#include <cmath>

int chooseCentralVertex(const vector<glm::quat> & input_rotations) {
    const int N = input_rotations.size();
    std::vector<double> affinity(N, 0);
    
    // Type-convert the input into ceres-style buffers
    double * rotations_q = new double[N * 4];
    for (int k = 0; k < N; ++k) {
        rotations_q[4*k + 0] = input_rotations[k][3]; // glm::quat is (x,y,z,w), but ceres quat is (w,x,y,z)
        rotations_q[4*k + 1] = input_rotations[k][0];
        rotations_q[4*k + 2] = input_rotations[k][1];
        rotations_q[4*k + 3] = input_rotations[k][2];
    }
    // normalize the input quaternions too, to guarantee that they are unit length.
    // I don't trust glm::quat inputs!
    for (int k = 0; k < N; ++k) {
        double sumsq = 0;
        for (int i = 0; i < 4; ++i)
            sumsq += rotations_q[4*k + i] * rotations_q[4*k + i];
        double norm = std::sqrt(sumsq);
        for (int i = 0; i < 4; ++i)
            rotations_q[4*k + i] /= norm;
    }
    // Validate the instance. Is it likely to work?
    double max_dist = 0.0;
    for (int i = 0; i < N; ++i) {
        for (int j = i+1; j < N; ++j) {
            double ri_T_q[4];
            ri_T_q[0] =  rotations_q[4*i + 0];
            ri_T_q[1] = -rotations_q[4*i + 1];
            ri_T_q[2] = -rotations_q[4*i + 2];
            ri_T_q[3] = -rotations_q[4*i + 3];
            double riT_rj_q[4];
            ceres::QuaternionProduct(ri_T_q, rotations_q + 4*j, riT_rj_q);
            double riT_rj_aa[3];
            ceres::QuaternionToAngleAxis(riT_rj_q, riT_rj_aa);
            double distance_sq = riT_rj_aa[0] * riT_rj_aa[0] +
                                 riT_rj_aa[1] * riT_rj_aa[1] +
                                 riT_rj_aa[2] * riT_rj_aa[2];
            if (distance_sq > 0) {
                affinity[i] += std::exp(-distance_sq);
                affinity[j] += std::exp(-distance_sq);
            }
        }

    }
    int max_affinity_index = 0;
    for (int k = 0; k < N; ++k)
        if (affinity[k] > affinity[max_affinity_index])
            max_affinity_index = k;
    
    std::cout << "[centralRotation] Highest affinity was: " << affinity[max_affinity_index] << std::endl;
    
    
    // release the dynamic memory
    delete[] rotations_q;
    
    return max_affinity_index;
}

void pruneAveragingInput(const vector<glm::quat>& input_rotations, int central_index, vector<glm::quat> * pruned_rotations) {
    double rot_q[4];
    rot_q[0] = input_rotations[central_index][3]; // glm::quat is (x,y,z,w), but ceres quat is (w,x,y,z)
    rot_q[1] = input_rotations[central_index][0];
    rot_q[2] = input_rotations[central_index][1];
    rot_q[3] = input_rotations[central_index][2];
    
    const int N = input_rotations.size();
    for (int k = 0; k < N; ++k) {
        double temp_q[4];
        temp_q[0] =  input_rotations[k][3]; // glm::quat is (x,y,z,w), but ceres quat is (w,x,y,z)
        temp_q[1] = -input_rotations[k][0];
        temp_q[2] = -input_rotations[k][1];
        temp_q[3] = -input_rotations[k][2];
        double res_q[4];
        ceres::QuaternionProduct(rot_q, temp_q, res_q);
        double res_aa[3];
        ceres::QuaternionToAngleAxis(res_q, res_aa);
        double dist = std::sqrt(res_aa[0] * res_aa[0] + res_aa[1] * res_aa[1] + res_aa[2] * res_aa[2]);
        if (dist < 3.1415926535 / 4)
            pruned_rotations->push_back(input_rotations[k]);
    }
}

void computeAverageRotation(const vector<glm::quat> & input_rotations, glm::quat * result){
    
    for (int i = 0; i < input_rotations.size(); ++i) {
    std::cout << "input rot " << i << ": " << input_rotations[i] << std::endl;
    }
    
    const bool DEBUG_MODE = true;
    const double STEP_CONVERGENCE_TOL = 1e-8;
    const double MAX_ITERATIONS = 20;
    
    
    
    if (DEBUG_MODE) {
        std::cout << "[computeAverageRotation] Beginning to average " <<
        input_rotations.size() << " rotations." << std::endl;
    }
    
    int central_index = chooseCentralVertex(input_rotations);
    std::cout << "Central vertex:" << central_index << std::endl;
    std::vector<glm::quat> pruned_rotations;
    pruneAveragingInput(input_rotations, central_index, &pruned_rotations);
    if (DEBUG_MODE) {
        std::cout << "[computeAverageRotation] Pruned down to " <<
        pruned_rotations.size() << " rotations." << std::endl;
    }
    
    const int N = pruned_rotations.size();
    
    // Type-convert the input into buffers
    double * rotations_q = new double[N * 4];
    for (int k = 0; k < N; ++k) {
        rotations_q[4*k + 0] =  pruned_rotations[k][3]; // glm::quat is (x,y,z,w), but ceres quat is (w,x,y,z)
        rotations_q[4*k + 1] = -pruned_rotations[k][0];
        rotations_q[4*k + 2] = -pruned_rotations[k][1];
        rotations_q[4*k + 3] = -pruned_rotations[k][2];
    }
    // normalize the input quaternions too, to guarantee that they are unit length.
    // I don't trust glm::quat inputs!
    int count_bad_input = 0;
    for (int k = 0; k < N; ++k) {
        double sumsq = 0;
        for (int i = 0; i < 4; ++i)
            sumsq += rotations_q[4*k + i] * rotations_q[4*k + i];
        double norm = std::sqrt(sumsq);
        for (int i = 0; i < 4; ++i)
            rotations_q[4*k + i] /= norm;
        if (1.0 - 1e-6 > norm || 1.0 + 1e-6 < norm)
            count_bad_input++;
    }
    if (DEBUG_MODE) {
        std::cout << "[computeAverageRotation] Found " << count_bad_input <<
        " unnormalized input quaternions." << std::endl;
    }
    
    // Validate the instance. Is it likely to work?
    double max_dist = 0.0;
    for (int i = 0; i < N; ++i) {
        for (int j = i+1; j < N; ++j) {
            double ri_T_q[4];
            ri_T_q[0] =  rotations_q[4*i + 0];
            ri_T_q[1] = -rotations_q[4*i + 1];
            ri_T_q[2] = -rotations_q[4*i + 2];
            ri_T_q[3] = -rotations_q[4*i + 3];
            double riT_rj_q[4];
            ceres::QuaternionProduct(ri_T_q, rotations_q + 4*j, riT_rj_q);
            double riT_rj_aa[3];
            ceres::QuaternionToAngleAxis(riT_rj_q, riT_rj_aa);
            double distance = std::sqrt(
                riT_rj_aa[0] * riT_rj_aa[0] +
                riT_rj_aa[1] * riT_rj_aa[1] +
                riT_rj_aa[2] * riT_rj_aa[2]
            );
            if (distance > max_dist)
                max_dist = distance;
        }
    }
    if (DEBUG_MODE) {
        std::cout << "[computeAverageRotation] Max data input distance is "
        << max_dist * 180 / 3.1415926535 << " degrees." << std::endl;
    }
    
    // Choose an initial guess rotation
    double curr_guess_q[4];
    for (int i = 0; i < 4; ++i)
        curr_guess_q[i] = rotations_q[0*4 + i];
    
    // Main optimization loop
    int num_iterations = 0;
    while (true) {
        
        double curr_guess_q_T[4];
        curr_guess_q_T[0] =  curr_guess_q[0];
        curr_guess_q_T[1] = -curr_guess_q[1];
        curr_guess_q_T[2] = -curr_guess_q[2];
        curr_guess_q_T[3] = -curr_guess_q[3];
        
        // Compute a step
        double step_aa[3] = {0.0, 0.0, 0.0};
        for (int k = 0; k < N; ++k) {
            double res_q[4];
            ceres::QuaternionProduct(curr_guess_q_T, rotations_q + 4*k, res_q);
            double res_aa[3];
            ceres::QuaternionToAngleAxis(res_q, res_aa);
            
            step_aa[0] += res_aa[0];
            step_aa[1] += res_aa[1];
            step_aa[2] += res_aa[2];
        }
        step_aa[0] /= N;
        step_aa[1] /= N;
        step_aa[2] /= N;
        
        // Check for convergence
        double step_size = std::sqrt(
            step_aa[0] * step_aa[0] +
            step_aa[1] * step_aa[1] +
            step_aa[2] * step_aa[2]
        );
        if (step_size < STEP_CONVERGENCE_TOL) {
            if (DEBUG_MODE) {
                std::cout << "[computeAverageRotation] Terminating upon seeing step size of " <<
                step_size << std::endl;
            }
            break;
        }
        
        // Apply the step
        double step_q[4];
        ceres::AngleAxisToQuaternion(step_aa, step_q);
        double temp_q[4];
        ceres::QuaternionProduct(curr_guess_q, step_q, temp_q);
        for (int i = 0; i < 4; ++i)
            curr_guess_q[i] = temp_q[i];
        if (DEBUG_MODE) {
            std::cout << "[computeAverageRotation] Took a step of size " <<
            step_size << std::endl;
        }
        
        // Check if we've reached the max iteration count
        ++num_iterations;
        if (num_iterations >= MAX_ITERATIONS) {
            if (DEBUG_MODE) {
                std::cout << "[computeAverageRotation] Quitting after reaching " << num_iterations
                << " iterations." << std::endl;
            }
            break;
        }
    }
    
    // Stuff the result
    result->w = curr_guess_q[1];
    result->x = curr_guess_q[2];
    result->y = curr_guess_q[3];
    result->z = curr_guess_q[0];
    
    
    // release the dynamic memory
    delete[] rotations_q;
}

