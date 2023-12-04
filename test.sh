#!/bin/bash

# Initial value for w
w=5

# Loop to double the value of w 8 times
for i in {1..8}
do
    regTable -s 3 -p 1 00 -w $w
    w=$(( w * 2 ))
done
