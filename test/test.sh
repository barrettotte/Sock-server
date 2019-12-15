#!/bin/bash

# Testing server multiple clients open/close
len=2

clear
echo
for i in $( seq 0 $len )
do 
  echo Running client $i of $len
  ../client/client "user$i"
  echo
done
