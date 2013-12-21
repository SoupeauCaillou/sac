#!/bin/bash

cd $(dirname $0)

temp=$(mktemp)
for asm_file in $(find tremor -name "*.s"); do
	./tremor/arm2gnu.pl $asm_file > $temp && mv $temp $asm_file
done
