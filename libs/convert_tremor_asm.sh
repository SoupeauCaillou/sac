#!/bin/sh

for asm_file in `find tremor -name "*.s"`;
do
	./tremor/arm2gnu.pl $asm_file > /tmp/a && mv /tmp/a $asm_file
done
