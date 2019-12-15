#!/bin/bash

# Testing server with multiple clients
ip=127.0.0.1
port=5024
len=2

clear
echo
for i in $( seq 0 $len )
do 
  echo Running client $i of $len
  ./client/client "user$i"
  echo
done
