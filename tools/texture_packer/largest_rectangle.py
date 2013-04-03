#!/usr/bin/env python
# From zed on github: https://gist.github.com/776423
import sys

from PIL import Image
from collections import namedtuple
from operator import mul

try:
    reduce = reduce
except NameError:
    from functools import reduce # py3k

Info = namedtuple('Info', 'start height row_start')
Histo = namedtuple('Histo', 'height row_start')

def max_size(mat, value=0):
    """Find height, width of the largest rectangle containing all `value`'s.

    For each row solve "Largest Rectangle in a Histrogram" problem [1]:

    [1]: http://blog.csdn.net/arbuckle/archive/2006/05/06/710988.aspx
    """
    it = iter(mat)

    hist = [Histo(el==value, 0) for el in next(it, [])]

    max_size = max_rectangle_size(hist)[0]
    cpt = 0
    pointX = cpt
    row_index = 1
    for row in it:
        #print (row_index, row)
        cpt = cpt + 1
        hist = [Histo(1+h.height, row_index - h.height) if el == value else Histo(0, row_index) for h, el in zip(hist, row)]

        m, (x,y) = max_rectangle_size(hist)
        if (max(max_size, m, key=area) != max_size):
            pointX = x
            pointY = y
            max_size = m

        row_index = row_index + 1

    #print (row_index)
    return (max_size, pointX, pointY)

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
    pointX = 0
    pointY = 0
    #print (histogram)
    for pos, h in enumerate(histogram):
        height = h.height
        start = pos # position where rectangle starts
        while True:
            if not stack or height > top().height:
                stack.append(Info(start, height, h.row_start)) # push
                #print ("append:", top())
            elif stack and height < top().height:
                m = max(max_size, (top().height, (pos - top().start)), key=area)
                if (m != max_size):
                    pointX = top().start
                    pointY = top().row_start
                    max_size = m
                    #print ("interm", top(), m)

                start, _, _ = stack.pop()
                continue
            break # height == top().height goes here

    pos += 1
    for start, h, row_start in stack:
        m = max(max_size, (h, (pos - start)), key=area)
        if (m != max_size):
            pointX = start
            pointY = row_start
            #print("ici", m, row_start)
            max_size = m

    #print ("result: ", max_size, (pointX, pointY))
    return (max_size, (pointX, pointY))

def area(size):
    return reduce(mul, size)

def __s2m(s):
    tab = []
    for j in range(im.size[1]):
        new = [(int)(s[i,j][3]<255) for i in range(im.size[0])]
        tab.append(new)
    return tab


if __name__=="__main__":
    if (len(sys.argv) != 2):
        print "Need the image in arg1: ./image.py image.png"
    else:
        im = Image.open(str(sys.argv[1]))

        # m[0] is the size of the rect, m[1] the top left corner point
        size, posX, posY = max_size(__s2m(im.load()))

        # print (size, posX, posY)
        # return the rectangle only if hes > 10% of the total size
        # if ( m[0][0] * m[0][1] > 0.10 * im.size[0] * im.size[1] ):
        print "%d,%d,%d,%d" % (posX, posY, size[1], size[0])

