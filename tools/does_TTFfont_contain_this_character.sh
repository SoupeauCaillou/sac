#!/bin/bash

if [ $# -lt 2 ]; then
    echo "expected 2 args (.ttf + char)"
    exit 10
elif [ ! -f $1 ]; then
    echo "could not find $1"
    exit 11
fi

font=$1
char=$2
shift #first arg is .ttf
shift #second arg is char

is_hexa=0
log_level=0
while [ "$1" != "" ]; do
    case $1 in
        "-x" | "-hexa")
            is_hexa=1
            ;;
        "--v" | "--verbose")
            shift
            log_level=$1
            ;;
        *)
            echo "Unknown option $1"
    esac
    shift
done
            

function debug {
    if [ $log_level -ge $1 ]; then
        echo $2
    fi
}

# Step 1: convert the UTF-8 given argument to hexadecimal value (0xValue)
if [ $is_hexa = 0 ]; then
    dec=$(printf "0x%x" "'$char")
else
    dec=0x$char
fi
debug 2 "$char -> $dec"

# Step 2: find the matching column in the .ttf file, and set it on 4 digits (eg: 0 -> 0000)
column=$(($dec / 0x100))
column=$(printf "%04x" "$column")

debug 2 "column is $column"

# Step 3: find the row matching the column value, and keep only the 8 32-bits(8Hexa) blocks
match_line=$(fc-query $font | grep $column: | cut -d ':' -f2)
debug 2 "match_line is $match_line"

# Step 4: keep row value which is needed to find the block
row=$(($dec % 0x100))
debug 2 "row is $row"

# Step 5: find the block ID (0: left, 7: right)
index=$(($row / 0x20))
debug 2 "index is $index"

# Step 6: retrieve the block and convert it to UPPER case (needed for bc operations)
match_block=$(echo $match_line | cut -d " " -f $(($index +1)) | tr '[:lower:]' '[:upper:]')
debug 2 "match_block is $match_block"

# Step 7: convert hexadecimal block value to right-to-left(inverted) binary value
match_block_to_binary=$(echo "obase=2; ibase=16; $match_block" | bc | rev)
# we force it to be 32-length long, (filling with left zeros)
zero_missing_number=$(expr 32 - $(echo $match_block_to_binary | wc -c))
if [ $zero_missing_number -gt 0 ]; then
    match_block_to_binary=$(printf "%0*d\n" $zero_missing_number 0)$match_block_to_binary
fi
debug 2 "match_block_to_binary is $match_block_to_binary"

# Step 8: find the byte position in the block
byte_position=$(($row % 0x20))
debug 2 "byte_position is $byte_position"

# Step 9: finally, get it and we're done!
match_byte=${match_block_to_binary:$byte_position:1}
debug 2 "match_byte is $match_byte"

if [ "$match_byte" = 1 ]; then
    debug 1 "Character $char is available in font $font!"
    exit 0
else
    debug 1 "Character $char not available in font $font..."
    exit 1
fi
