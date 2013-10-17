#!/bin/bash

if [ $# != 2 ]; then
    echo "expected 2 args (.ttf + char)"
    exit
elif [ ! -f $1 ]; then
    echo "could not find $1"
    exit
fi

function debug {
    # echo $@
    :
}

# Step 1: convert the UTF-8 given argument to hexadecimal value (0xValue)
dec=$(printf "0x%x" "'$2")

debug "$2 -> $dec"

# Step 2: find the matching column in the .ttf file, and set it on 4 digits (eg: 0 -> 0000)
column=$(($dec / 0x100))
column=$(printf "%04x" "$column")

debug "column is $column"

# Step 3: find the row matching the column value, and keep only the 8 32-bits(8Hexa) blocks
match_line=$(fc-query $1 | grep $column: | cut -d ':' -f2)
debug "match_line is $match_line"

# Step 4: keep row value which is needed to find the block
row=$(($dec % 0x100))
debug "row is $row"

# Step 5: find the block ID (0: left, 7: right)
index=$(($row / 0x20))
debug "index is $index"

# Step 6: retrieve the block and convert it to UPPER case (needed for bc operations)
match_block=$(echo $match_line | cut -d " " -f $(($index +1)) | tr '[:lower:]' '[:upper:]')
debug "match_block is $match_block"

# Step 7: convert hexadecimal block value to right-to-left(inverted) binary value
match_block_to_binary=$(echo "obase=2; ibase=16; $match_block" | bc | rev)
match_block_to_binary=$(printf '0' $(expr 32 - $(echo $match_block_to_binary | wc -c)))$match_block_to_binary
debug "match_block_to_binary is $match_block_to_binary"

# Step 8: find the byte number in the block
byte=$(($row % 0x20))
debug "byte is $byte"

# Step 9: finally, get it and we're done!
match_byte=${match_block_to_binary:$byte:1}
debug "match_byte is $match_byte"

if [ "$match_byte" = 1 ]; then
    echo "Character $2 is available in font $1!"
else
    echo "Character $2 not available in font $1..."
fi
