To compile the program run the following compilation command
g++ -std=c++11 *.cpp -o cachesimulator

To run the program run the outputted execution file, 
either cachesimulator.exe or cachesimulator.out, from
the command line and include the input file name as an
argument.

When prompted enter the address 0x00 and the address
you want to initialize the ram up to.

When prompted enter the cache settings you want.

The cache size should be between 8 and 256 bytes

Only 1, 2, and 4 way associativity is supported

When prompted for replacement policy enter the corresponding integer
for the desired policy:
Random Replacement Policy = 1
Least Recently Used = 2

When prompted for write hit policy enter the corresponding integer
for the desired policy:
write-through = 1
write-back = 2

When prompted for write miss policy enter the corresponding integer
for the desired policy:
write-allocate = 1
no write-allocate = 2

