#!/usr/env python

import sys
import os

if len(sys.argv) < 2:
	print ("This script requires at least 1 argument : file to convert")
	sys.exit()

TEMP1="""unsigned char {}[] = """
TEMP2="""unsigned int {}_len = {};"""

path = os.path.split(sys.argv[1])[0]
filename = os.path.split(sys.argv[1])[-1].replace('.','_')
outputFile = os.path.join(path, filename+'.h')

if len(sys.argv) > 2 :
	outputFile = sys.argv[2]

with open(sys.argv[1], 'r') as i:
	with open(outputFile, 'w') as o:
		hexList = []
		for line in i:
			for c in line:
				hexList.append(hex(ord(c)))
		
		o.write(TEMP1.format(filename) + '{\n')

		for i in range (0, (int)(len(hexList)/12)+1):
			o.write(', '.join(hexList[i*12:i*12+12])+',\n')
		o.write('0x00 };\n')
		o.write(TEMP2.format(filename, len(hexList)))


