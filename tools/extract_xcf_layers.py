#!/usr/bin/env python
# coding: utf-8

import os, glob, sys, time

allowedFormat = [".xcf", ".psd"]

DESC = "[General]\n"\
	   "page_count = {}\n"

PAGE = "[page_{}]\n"\
	   "element_count = {}\n"

ELEMENT = "element_{0}_texture = {1}\n"\
		  "element_{0}_size = {2}, {3}\n"\
		  "element_{0}_position = {4}, {5}\n"

OTHER = "[others]\n"\
		"element_count = {}\n"


def run(directory, rewrite = False):
	from gimpfu import pdb
	start = time.time()
	#for all xcf files in working directory
	print("Running on directory '{}'".format(directory))
	for filename in os.listdir(directory):
		if os.path.splitext(filename)[-1] in allowedFormat:
			print("Found a file : '{}'".format(filename))
			image = pdb.gimp_file_load(os.path.join(directory, filename), os.path.join(directory, filename))
			groupLayer = {'others':[]}
			for layer in image.layers:
				for c in [layer] + layer.children:
					if c.name[:-1].isdigit():
						if c.name[:-1] not in groupLayer:
							groupLayer[c.name[:-1]] = []
						groupLayer[c.name[:-1]].append([c.name, c.width, c.height, c.offsets[0], c.offsets[1]])
					else:
						groupLayer['others'].append([c.name, c.width, c.height, c.offsets[0], c.offsets[1]])

					print("Write layer '{0}' in '{0}.png'".format(c.name))
					# write the new png file
					pdb.file_png_save(image, c, os.path.join(directory, c.name +".png"), os.path.join(directory, c.name +".png"), 0, 9, 0, 0, 0, 0, 0)
			with open(os.path.join(directory, os.path.splitext(filename)[0] + ".desc"), "wb") as descfile:
				descfile.write(DESC.format(len(groupLayer)))

				for key in sorted(groupLayer.keys()):
					if key == 'others':
						descfile.write(OTHER.format(key, len(groupLayer[key])))
					else : 
						descfile.write(PAGE.format(key, len(groupLayer[key])))
					element_count = 1
					for element in groupLayer[key]:
						descfile.write(ELEMENT.format(element_count, *element))
						element_count += 1

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
