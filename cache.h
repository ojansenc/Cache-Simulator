// File: cache.h 
// Author: Oliver Carver 
// Date: 12/07/2021
// Section: 312-506 
// E-mail: jansenc@tamu.edu 
// Description:
// Header file for the cache class that includes function for
// hex printing, and the register (individual cache line) struct

#ifndef CACHE_H 
#define CACHE_H 
#include "RAM.h"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <bitset>

/*! \mainpage Cache Project for CSCE312
 * Cache Project written using C++
 * @author Oliver Carver
 *
 * \section Cache
 * Implementation of the CPU cache
 * Cache
 *
 * \section Reg
 * Implementation of the cache line
 * Reg
 *
 * \section RAM
 * Implementation of main memory
 * RAM
 *
 * \section Usage
 *
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
*/

//! A struct implementing the cache line
/*!
 * The Reg struct implements the functionality for the cache line.
 * Each reg is meant to represent and function like a cache line should,
 * holding the various attributes of the cache line (whether its valid, dirty
 * its tag...) and also holds the data of the cache line
 * @author Oliver Carver
 */

struct Reg {
	bool dirty = 0; /*< Bool representing whether the line is dirty*/
	bool valid = 0; /*< Bool representing whether the line is valid*/
	int tag = 0; /*< Int to represent the cache line's tag */
	int RAMAddress = -1; /*< RAM address for the data stored here. Allows for easy writing*/
	int index = 0; /*< Int torepresent the cache lines index*/
	std::vector<int> data; /*< Vector holding the cache line's data*/
	//! A clear function
	/*!
	 * The function resets all the properties of the cache line
	 * and fills the line with 0s
	 */
	void clear() { 
		dirty = valid = tag = index = 0;
		std::fill(data.begin(),data.end(),0);
	}
};

//! A class implementing the Cache
/*!
 * This is a rather monolithic class that aims to implement the majority
 * of the functionality of the cache. It contains all the sets and cache lines
 * and also contains the corresponding properties of the cache. There is no
 * intermediate data structure between the cache and the cache line, sets are
 * represented as a vector of regs (cache lines), that are then a part of a vector of
 * sets that is implemented by the Cache class
 * @author Oliver Carver
 */
class Cache {
	private:
		int size; /*< Int representing the size of the cache*/
		int numOfSets; /*< Int that documents the number of sets within the cache*/
		int blockSize; /*<Int that tracks the size of the blocks within the cache*/
		int assoc; /*<Int that represents the associativity of the cache*/
		int replace; /*< Int that holds the replacement policy of the cache*/
		int whit; /*< Holds the write hit policy*/
		int wmiss;/*< Holds the write miss policy*/
		int numOfHits = 0; /*< Keeps track of the number of Hits in the cache*/
		int numOfMisses = 0;/*< Keeps track of the number of misses in the cache*/
		int idxBits; /*< Holds the number of index bits for addresses*/
		int tagBits; /*< Holds the number of tag bits for addresses*/
		int blockBits; /*< Holds the number of block bits for addresses*/
		int memBits; /*< Holds the number of memory bits */
		std::vector<std::vector<Reg> > sets; /*< Represents the sets within the cache line, a vector of vectors of regs*/
		std::vector<std::vector<int> > evictionSets; /*< Vector that holds the eviction priority for each set, the tag
		for each set is put into the vector, and then when evicted the tag is swapped from the 0 index which is the bottom
		of the priority.*/

	public:
		//! Initializes the cache, using values passed in by the user during initialization
		/*!
		 * The function initializes all the values of the cache, and creates the corresponding
		 * vectors of sets, and eviction sets, according to user specification. Each register
		 * is zeroed out on creation. The number of bits for the addresses are also calculated
		 * The function takes in values that were provided by the user and initializes all 
		 * variables of the class.
		 * \param size Integer size of the cache
		 * \param blockSize Integer size of the blocks
		 * \param assoc Integer representation of associativity
		 * \param replace Integer represetation of replacement policy
		 * \param whit Integer representation of write hit policy
		 * \param wmiss Integer representation of write miss policy
		 */
		void init(int size, int blockSize, int assoc, int replace, int whit, int wmiss);
		//! Obtains input values for the cache from the user
		/*!
		 * Displays questions to standard out and reads in user inputs, which are then 
		 * passed to the init function which will handle the initialization of the cache.
		 * The function takes in the size of the RAM to calculate the number of memory bits.
		 * \param RAMSize Integer representation of the size of the RAM
		 */
		void input(int RAMSize);
		//! Writes data to the cache and handles evictions/misses
		/*!
		 * A rather monolithic function that will write to the cache and memory depending on user
		 * input. The function takes in a value and an address, supplied by the user, and the 
		 * memory passed by reference. It will then write to the cache and memory, and display
		 * information to standard out as per specification. In addition, evictions and misses for writes
		 * are handled in this function as well.
		 * \param value User provided integer value
		 * \param address User provided unsigned long address
		 * \param mem Passed by reference main memory
		 */
		void write(int value, unsigned long address, RAM& mem);
		//! Reads data from the cache and handles evictions/misses
		/*!
		 * A rather monolithic function that will read from the cache and memory depending
		 * on user input. The function takes in an address supplied by the user, and the
		 * memory passed by reference. It will then read the memory in the cache at that addresses
		 * and display information to standard out as per specification. In addition, it also handles
		 * evictions and misses for reads.
		 * \param address User provided unsigned long address
		 * \param mem Passed by reference main memory
		 */
		void read(unsigned long address, RAM& mem);
		//! Initializes data needed for the operation of the read and write functions
		/*! 
		 * The read and write functions convert the user provided address to binary and this
		 * function parses the binary and sets the passed by reference variables equal to
		 * the values obtained from the binary representation of the address. These values
		 * are then used for the cache read/write operations.
		 * \param addressBits Bitset containing the binary value of user provided address
		 * \param offset Passed by reference int holding the address offset
		 * \param idx Passed by reference int holding the address index
		 * \param tag Passed by reference int holding the address tag
		 * Helper function that takes as input variables needed by the read and write functions.
		 */
		void readWriteInit(std::bitset<8> addressBits, int& offset, int& idx, int& tag);
		//! Dumps the cache values to a file
		/*!
		 * The function dumps the entirety of the cache to the cache.txt file
		 * according to specification
		 */
		const void dump();
		//! Displays the contents of the cache to standard out
		/*
		 * The function displays the entirety of the cache to standard out
		 * and also displays the current cache settings, according to specification
		 */
		const void view();
		//! Flushes and resets all the data in the cache
		/*
		 * The function resets all registers in the cache and re-initializes them
		 * to 0 while preserving the current user provided settings for the cache.
		 */
		void flush();
};

//! Helper function to easily allow for hexadecimal output
/*!
 * Helper function that takes in an output stream, and integer, and a bool
 * which determines whether a space should be output to the ostream. Necessary
 * due to the rather long winded nature of the code required for outputting
 * correctly formatted hexadecimal.
 * \param o Ostream object that displays the output
 * \param num Integer that holds the number meant to be outputted
 * \param space Control bool that controls whether a space is outputted
 */
const inline void outputHex(std::ostream& o, int num, bool space) {
	o << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << num;
	if (space) o << ' ';
}

#endif
