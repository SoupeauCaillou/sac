#!/usr/bin/env python

import sys

from PIL import Image
from collections import namedtuple
from operator import mul

try:
    reduce = reduce
except NameError:
    from functools import reduce # py3k

Info = namedtuple('Info', 'start height')

#retourne le plus petit rectangle contenant tous les pixels non transparent
def tiniest(im, mat):
    maxX = -1
    minX = im.size[0]
    maxY = -1
    minY = im.size[1]

    for j in range(im.size[1]):
        for i in range(im.size[0]):
            if (mat[i][j] == 1):
                if (i < minX):
                    minX = i
                if (i > maxX):
                    maxX = i
                if (j < minY):
                    minY = j
                if (j > maxY):
                    maxY = j


    print "%d,%d,%d,%d" % (maxX - minX + 1, maxY - minY + 1, minX, minY)

def __s2m(s):
    tab = []
    for i in range(im.size[0]):
        new = [s[i,j][3] != 0 for j in range(im.size[1])]
        tab.append(new)
    return tab

if __name__=="__main__":
    if (len(sys.argv) != 2):
        print "Need the image in arg1: ./script.py image.png"
    else:
        im = Image.open(str(sys.argv[1]))
        im = im.convert("RGBA")

        #for i in range(im.size[1]):
        #    print [im.load()[j,i][3] for j in range(im.size[0])]
        tiniest(im, __s2m(im.load()))
