#! /bin/sh
reset	

if [ $# -ge 1 ] && [ "$0" = "./build.sh" ]; then
	echo "cleaning"
	rm -r C* c* l* M* s* t* 2>/dev/null
fi
cmake ../..
make -j4

if [ $# -eq 2 ]; then
	./linux/recursiveRunner --verbose
fi
