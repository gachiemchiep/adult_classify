#!/usr/bin/python

#First remove all space from file name
# find . -type f -name "*.jpg" -execdir rename 's/ /_/g' "{}" \;
# find . -type f -name "*.jpg" -execdir rename 's/\(/1/g' "{}" \;
# find . -type f -name "*.jpg" -execdir rename 's/\)/1/g' "{}" \;
# after bash command list is generated 
# bash command_list.txt or sh command_list.txt to run it

# after running copy files from data folder to desire folder
# for example _rgb.jpg -> _rgb

import sys
import os

argc = len(sys.argv);
argv = sys.argv;

if (argc != 3):
	print "Usage %s data_folder result_name" % (argv[0]);
	sys.exit();

if (os.path.exists(argv[1])):
	print "%s is  a directory" % (argv[1]);
else :
	print "%s is not a directory" % (argv[1]);
	sys.exit(); 

def list_files(dir, extension):
	r = [];
	subdirs = [x[0] for x in os.walk(dir)];
	for subdir in subdirs:
		files = os.walk(subdir).next()[2]
		for file in files:
			if extension in file:
				r.append(os.path.abspath(subdir + "/" + file));
				#r.append(subdir + "/" + file);
	return r;
	
files = list_files(argv[1], ".jpg");
with open(argv[2], 'w') as f:
	for file in files:
		command = "./remove_background " + file + "\n";
		f.write(command);
		
f.close();		

methods = ["RGB, "];
	
			
	