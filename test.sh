#!/bin/bash

# Testing server with multiple clients
ip=127.0.0.1
port=5024
len=10

for i in $( seq 0 $len )
do 
  echo running client $i of $len
  ./client/client $ip "Hello server" $port
done
