/*
 * params.h
 *
 *  Created on: Sep 5, 2014
 *      Author: gachiemchiep
 */

#ifndef PARAMS_H_
#define PARAMS_H_

#include <vector>
#include <string>
#include <cmath>

// Remove background's detail
extern std::vector<std::string> RM_BG_METHODS ; /**< Remove background's methods*/

// Extract feature's detail
extern std::vector<std::string> EXTRACT_FEATURE_METHODS; /**< Extract feature's methods */
extern std::vector<float> VER_EDGE_FILTER; /**< Vertical edge's filter*/
extern std::vector<float> HOR_EDGE_FILTER; /**< Horizontal edge's filter*/
extern std::vector<float> DIA45_EDGE_FILTER; /**< 45 degree rotated edge's filter*/
extern std::vector<float> DIA135_EDGE_FILTER ; /**< 135 degree rotated edge's filter*/
extern std::vector<float> NOND_EDGE_FILTER; /**< Is not edge*/
extern std::vector<std::vector<float> > EDGE_FILTERS ;
extern float EDGE_THRES; /**<  Edge's threshold value */
extern int EHD_BLOCK_COUNT; /**<  Block count for calculating ehd feature.\nHeight*width = [new_h*sqrt(count)]*[new_w*sqrt(count)]  */
extern int NORM_SIZE; /**< NORMALIZE size.\nSmaller value reduce more noise from feature*/

// Testing's detail
extern float ADULT_TEST_PER; /**< How many percentage of adult features should be used for testing*/
extern float NON_ADULT_TEST_PER; /**< How many percentage of non_adult features should be used for testing*/
extern float NN_PER; /**< How many percentage of nearest neighbors should be ranked*/

extern int RANK_MAX_SCORE; /**< Score for ranking*/
extern float THRES; /**< Thres value to decide whether image is adult or non*/

#endif /* PARAMS_H_ */
