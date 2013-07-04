#!/usr/bin/env python
# coding: utf-8

import os, glob, sys, time

def run(directory, rewrite = False):
	from gimpfu import pdb
	start = time.time()
	#for all xcf files in working directory
	print("Running on directory '{0}'".format(directory))
	for filename in os.listdir(directory):
		if ".xcf" in filename[-4:]:
			print("Found a xcf file : '{0}'".format(filename))
			image = pdb.gimp_file_load(directory+filename, directory+filename)

			for layer in image.layers:
				output = layer.name
				#check if an existing as already this name
				while output+".png" in os.listdir(directory) and not rewrite:
					output += '1'
				print("Write layer '{0}' in '{1}.png'".format(layer.name, output))
				# write the new png file
				pdb.file_png_save(image, layer, directory + output +".png", directory + output + ".png", 0, 9, 0, 0, 0, 0, 0)

	end = time.time()
	print("Finished, total processing time : {0:.{1}f}".format(end-start, 2))


if __name__ == '__main__':
	directory = os.getcwd() + '/'
	script_directory = os.path.abspath(os.path.dirname(sys.argv[0]))
	print("This script will convert a (multilayer) xcf of current directory into as many as needed png files.")
	print("Let's go !")
	# execute gimp with this script
	os.chdir(script_directory)
	command = 'gimp -idfs --batch-interpreter python-fu-eval -b '\
			  '"import sys; sys.path=[\'.\']+sys.path;'\
			  'import extract_xcf_layers as e; e.run(\''+directory+'\')" -b "pdb.gimp_quit(1)"'
	os.system(command)
	os.chdir(directory)

	print("Alright, auf wiedersehen !")
