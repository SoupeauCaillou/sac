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

def max_size(mat, value=0):
	"""Find height, width of the largest rectangle containing all `value`'s.

	For each row solve "Largest Rectangle in a Histrogram" problem [1]:
	
	[1]: http://blog.csdn.net/arbuckle/archive/2006/05/06/710988.aspx
	"""
	it = iter(mat)
	
	hist = [(el==value) for el in next(it, [])]
	max_size = max_rectangle_size(hist)[0]
	cpt = 0
	pointX = cpt
	for row in it:
		cpt = cpt + 1
		hist = [(1+h) if el == value else 0 for h, el in zip(hist, row)]

		m = max_rectangle_size(hist)
		if (max(max_size, m[0], key=area) != max_size):
			pointX = cpt - m[0][0]
			pointY = m[1]
			max_size = m[0]
	return (max_size, (pointX, pointY))

def max_rectangle_size(histogram):
	"""Find height, width of the largest rectangle that fits entirely under
	the histogram.

	>>> f = max_rectangle_size
	>>> f([5,3,1])
	(3, 2)
	>>> f([1,3,5])
	(3, 2)
	>>> f([3,1,5])
	(5, 1)
	>>> f([4,8,3,2,0])
	(3, 3)
	>>> f([4,8,3,1,1,0])
	(3, 3)
	>>> f([1,2,1])
	(1, 3)

	Algorithm is "Linear search using a stack of incomplete subproblems" [1].

	[1]: http://blog.csdn.net/arbuckle/archive/2006/05/06/710988.aspx
	"""
	stack = []
	top = lambda: stack[-1]
	max_size = (0, 0) # height, width of the largest rectangle
	pos = 0 # current position in the histogram
	pointY = 0
	for pos, height in enumerate(histogram):
		start = pos # position where rectangle starts
		while True:
			if not stack or height > top().height:
				stack.append(Info(start, height)) # push
			elif stack and height < top().height:
				m = max(max_size, (top().height, (pos - top().start)), key=area)
				if (m != max_size):
					pointY = top().start
					max_size = m
					
				start, _ = stack.pop()
				continue
			break # height == top().height goes here
				
	pos += 1
	for start, height in stack:
		m = max(max_size, (height, (pos - start)), key=area)
		if (m != max_size):
			pointY = -1 #que fait ce code ? 
			max_size = m
	
	return (max_size, pointY)

def area(size):
	return reduce(mul, size)

def __s2m(s):
	tab = []
	for i in range(im.size[0]):
		new = [s[i,j][3]<255 for j in range(im.size[1])]
		tab.append(new)
	return tab
		
		
if __name__=="__main__":
	if (len(sys.argv) != 2):
		print "Need the image in arg1: ./image.py image.png"
	else:
		im = Image.open(str(sys.argv[1]))
		
		# m[0] is the size of the rect, m[1] the top left corner point
		m = max_size(__s2m(im.load()))
		
		# return the rectangle only if hes > 10% of the total size
		if ( m[0][0] * m[0][1] > 0.10 * im.size[0] * im.size[1] ):
			print "%d,%d,%d,%d" % (m[1][0]+1, m[1][1]+1, m[0][0]+m[1][0], m[0][1]+m[1][1]-1)

