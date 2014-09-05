/**
 * Name        : main.cpp
 * Author      : gachiemchiep
 * Description : Adult classifier's main program
 */

#include <iostream>
#include <stdio.h>
#include <boost/program_options.hpp>
#include "params.h"
#include "bg_remove.h"
#include "feature_extractor.h"
#include <algorithm>
#include <vector>
#include <string>

using namespace std;
using namespace boost::program_options;

int main(int argc, char *argv[]) {

	// Init options
	boost::program_options::options_description main_options("main options");
	boost::program_options::options_description rm_bg_options("Remove background options");
	boost::program_options::options_description extract_feature_options("Extract features options");
	boost::program_options::options_description do_testing_options("Execute testing options");

	main_options.add_options()
		("help, h",    "Print help message")
		("rm_bg, r",    "Remove background ")
		("extract_feature, e",    "Extract feature (scd+ehd+cd)")
		("execute_testing, t",    "Execute testing")
	;

	rm_bg_options.add_options()
		("bg_img, b",    value<string>(),    "Image path which is used for removing background")
		("rm_method",    value<string>()->default_value("all"),    "Method for removing background")
	;

	extract_feature_options.add_options()
		("img_path, p",    value<string>(),    "Image path which is used for extracting feature")
		("feature_type",    value<string>()->default_value("all"),    "Method for extracting feature.\nCurrently not usable")
		("feature_file",    value<string>()->default_value("feature.txt",    "File in which feature will be appended to"));
	;

	do_testing_options.add_options()
	    ("adult_features_file, x",    value<string>(),    "File contains all adult content images's feature vectors")
	    ("non_adult_features_file, y",    value<string>(),    "File contains all no_adult content images's feature vectors")
	;

	// Combine into 1 option
	main_options.add(rm_bg_options).add(extract_feature_options).add(do_testing_options);

	// Store parsed values
	boost::program_options::variables_map parsed_values;

	// Begin parsing values
	try {
		// Parse all parsed to variable map
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, main_options), parsed_values);

		// Store into variable map
		boost::program_options::notify(parsed_values);

		if (parsed_values.count("help") || (argc == 1)) {
			std::cerr << main_options << "\n";
			return 1;
		}

		// Remove background
		if (parsed_values.count("rm_bg")) {
			// No input image path
			if (!parsed_values.count("bg_img")) {
				std::cerr << main_options << "\n";
			} else { // Have image
				std::string img_path = parsed_values["bg_img"].as<string>();
				std::string method = parsed_values["rm_method"].as<string>();
				std::cerr << "Image contain background:" << img_path << " \nMethod" << method << "\n";

				std::vector<bg_remove*> bg_removes;
				bg_removes.push_back(new rgb());
				bg_removes.push_back(new ycrcb());
				bg_removes.push_back(new hsv());
				bg_removes.push_back(new hls());
				bg_removes.push_back(new rgb_norm());
				bg_removes.push_back(new hsi());

				std::vector<std::string>::iterator methods_it ;
				methods_it = std::find(RM_BG_METHODS.begin(), RM_BG_METHODS.end(), method);
				if (methods_it == RM_BG_METHODS.end()) { // method name is not found, use all available method
					std::cerr << "Input method is not valid. Remove background using all available methods \n";
					for (std::vector<bg_remove*>::iterator bg = bg_removes.begin();
									bg != bg_removes.end(); ++bg) {
						(*bg)->set_img(img_path);
						(*bg)->init();
						(*bg)->run();
						int i = bg - bg_removes.begin();
						(*bg)->save_crop(i);
					}
				} else { // method name is found, point to corresponding method and run
					int method_count = methods_it - RM_BG_METHODS.begin();
					std::vector<bg_remove*>::iterator bg_it = bg_removes.begin() + method_count;
					(*bg_it)->set_img(img_path);
					(*bg_it)->init();
					(*bg_it)->run();
					(*bg_it)->save_crop(method_count);
				}
			}
			std::cerr << "Removing background finish!!! \n";
			return 0;
		}

		// Extract features
		if (parsed_values.count("extract_feature")) {
			// No input image path
			if (!parsed_values.count("img_path")) {
				std::cerr << main_options << "\n";
			} else { // Have image
				std::string img_path = parsed_values["img_path"].as<string>();
				std::string feature_type = parsed_values["feature_type"].as<string>();
				std::string feature_file = parsed_values["feature_file"].as<string>();
				std::cerr <<"Image:" << img_path << " \nFeature type:" << feature_type <<
						" \nFeature_file:" << feature_file <<"\n";

				//TODO now use only ALL as default

				feature_extractor fe ;
				fe.set_img_path(img_path);
				fe.set_method("ALL");
				fe.calculate_feature();
				fe.save_result(feature_file);
			}
			std::cerr << "Extracting feature finish!!! \n";
			return 0;
		}

		// Execute testing to evaluate classifier's accuracy
		if (parsed_values.count("execute_testing")) {
			// Dont have adult_features_file or non_adult_features_file
			if ((!parsed_values.count("adult_features_file"))
			   || (!parsed_values.count("non_adult_features_file"))
			) {
				std::cerr << main_options << "\n";
			} else { // Have image
				std::string adult_features_file = parsed_values["adult_features_file"].as<string>();
				std::string non_adult_features_file = parsed_values["non_adult_features_file"].as<string>();
				std::cerr << adult_features_file << " " << non_adult_features_file << "\n";
			}
			std::cerr << "Testing finish!!! \n";
			return 1;
		}


	} catch (std::exception &error) {
		std::cerr << error.what() << "\n";
	}

	return 0;
}

