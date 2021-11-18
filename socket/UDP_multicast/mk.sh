#!/bin/sh

gcc -o client-1 udp_multicast_client-1.c 
gcc -o client-2 udp_multicast_client-2.c 

gcc -o server udp_multicast_server.c 
