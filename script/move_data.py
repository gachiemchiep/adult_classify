#!/usr/bin/python

print "Usage: move background removed files to another folder \n";
print "corresponding to method name \n";

import sys
import os
import shutil
from shutil import copytree, ignore_patterns
import distutils.dir_util.create_tree

argc = len(sys.argv);
argv = sys.argv;

if (argc != 2):
	print "Usage : %s full_path" %(argv[0])
	sys.exit();

def list_files(dir, string):
	r = [];
	subdirs = [x[0] for x in os.walk(dir)];
	for subdir in subdirs:
		files = os.walk(subdir).next()[2]
		for file in files:
			if string in file:
				r.append(os.path.abspath(subdir + "/" + file));
				#r.append(subdir + "/" + file);
	return r;
def ignore_file(dir, files):
	return [file for file in files if os.path.isfile(os.path.join(dir, file))]

path = argv[1];
#path = '/home/gachiemchiep/git/adult_classify/imgs/data'
methods = ["RGB", "YCrCb", "HSV", "HLS", "RGB_norm", "HSI"];
method_based_names = ["_RGB.jpg", "_YCrCb.jpg", "_HSV.jpg", "_HLS.jpg", "_RGB_norm.jpg", "_HSI.jpg"]

for method in methods:
	path_for_method = path + "_" + method;
	if (os.path.exists(path_for_method)):
		print "%s is not empty" % (path_for_method);
	else :
		print "%s is not exist. Create %s " % (path_for_method, path_for_method);
		# copy entire tree (ignore file)
		shutil.copytree(path, path_for_method, ignore=ignore_file);

for i in range(len(methods)):
	method_based_name = method_based_names[i];
	method = methods[i];
	method_files = list_files(path, method_based_name);	
	folder_name = os.path.basename(path);	
	folder_name_new = folder_name + "_" + method;
	for file in method_files:
		file_new = file.replace(folder_name, folder_name_new);
		shutil.copyfile(file, file_new);
				