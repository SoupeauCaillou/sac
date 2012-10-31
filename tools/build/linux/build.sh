#! /bin/sh -x
if [ $# -eq 1 ] && [ "$0" = "./build.sh" ]; then
	echo "cleaning"
	rm -r C* c* l* M* s* t* 2>/dev/null
fi
	
cmake ../..
make -j4

echo "launch it ? (y/N) "
read confirm

if [ "$confirm" = "y" ]; then
	./linux/heriswap --verbose
fi
