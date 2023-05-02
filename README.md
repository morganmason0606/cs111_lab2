NOT DONE DON'T USE JUST YET








## UID: 705747359
(IMPORTANT: Only replace the above numbers with your true UID, do not modify spacing and newlines, otherwise your tarfile might not be created correctly)

## Round Robin

This progam calculates the average wait time and response time from a provided list of processes and quantum lenght

## Building

this program is built with a make file: run the command ` make ` to build it; 

## Running

the program is called `./rr path_to_process.txt quant_lenght` where path_to_process.txt is a text file with the process in the form 

number of processes
pid, arrival time, burst length

and the processes are ordered on pid and arivial time (with the first process arriving at time 0)
and quant length is the desired quantum lenght expresed as an integer > 0 

the program will output two lines telling you what the average wating time and response time are

## Cleaning up

this program is cleaned up with a make file: run the command ` make clean ` to clean