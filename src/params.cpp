#include "params.h"

// Remove background's detail
std::vector<std::string> RM_BG_METHODS = { "RGB", "YCrCb", "HSV", "HLS", "RGB_norm", "HSI" };

// Extract feature's detail

// methods
std::vector<std::string> EXTRACT_FEATURE_METHODS = { "SCD", "EHD", "CD", "ALL" };

// edge's filters
std::vector<float> VER_EDGE_FILTER { 1, -1, 1, -1 };
std::vector<float> HOR_EDGE_FILTER { 1, 1, -1, -1 };
std::vector<float> DIA45_EDGE_FILTER { std::sqrt(2), 0, 0, -std::sqrt(2) };
std::vector<float> DIA135_EDGE_FILTER { 0, std::sqrt(2), -std::sqrt(2), 0 };
std::vector<float> NOND_EDGE_FILTER { 2, -2, -2, 2 };
std::vector<std::vector<float> > EDGE_FILTERS { VER_EDGE_FILTER,
	HOR_EDGE_FILTER, DIA45_EDGE_FILTER, DIA135_EDGE_FILTER, NOND_EDGE_FILTER };

float EDGE_THRES = 50;
int EHD_BLOCK_COUNT = 1024;
int NORM_SIZE = 512;

// Testing detail parameters
float ADULT_TEST_PER = 0.2;
float NON_ADULT_TEST_PER = 0.2;
float NN_PER = 0.1;
int RANK_MAX_SCORE = 1000;
float THRES = 0.7;
