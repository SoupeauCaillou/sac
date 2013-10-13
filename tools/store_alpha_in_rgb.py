import sys
import math

from PIL import Image
from collections import namedtuple
from operator import mul

if __name__=="__main__":
    if (len(sys.argv) != 2):
        print "Need the image in arg1: ./image.py image.png"
    else:
        im = Image.open(str(sys.argv[1]))
        im = im.convert("RGB")
        pixels = im.load()

        output = []
        im2 = Image.new("RGB", (int(math.ceil(im.size[0] / 3.0)), im.size[1]), "white")
        
        print("NEW SIZE: " + str(im.size) + " -> " + str(im2.size))
        for j in range(im.size[1]):
            r = 0
            g = 0
            b = 0
            for i in range(im.size[0]):
                if i % 3 == 0:
                    r = pixels[i, j][0]
                elif (i % 3) == 1:
                    g = pixels[i, j][0]
                else:
                    b = pixels[i, j][0]
                    output.append((r, g, b))

            if im.size[0] % 3 != 0:
                output.append((r, g, b))

        print (str(len(output)) + " - " + str(im.size[0] * im.size[1]) + " - " + str(im2.size[0] * im2.size[1]))
        im2.putdata(output)
        im2.save('/tmp/a.png')