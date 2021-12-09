// File: RAM.h 
// Author: Oliver Carver 
// Date: 12/07/2021
// Section: 312-506 
// E-mail: jansenc@tamu.edu 
// Description:
// Header file for the RAM class 

#ifndef RAM_H 
#define RAM_H 
#include <iostream>
#include <vector>

//! Struct that represents the memory
/*!
 * This struct contains the passed in user data from
 * the input.txt file. This is essentially a wrapper struct 
 * that has some assoociated functions and variables to go along
 * with the vector that actually holds the data.
 * @author Oliver Carver
 */
struct RAM {
	std::vector<int> memory; /*!< Vector that holds data from passed in file*/
	int size = 0; /*!< Int that contains the size of the memory*/
	//! This function initializes the RAM according to user input
	/*!
	 * The function prompts the user for the initialization addresses
	 * and then modifies the RAM object according to the user specifications
	 * \param filename C-String that contains the user passed in filename
	 */
	void memoryInit(char* filename);
	//! Function that displays the memory to standard out
	const void memoryView();
	//! Function that dumps the memory to ram.txt
	const void memoryDump();
};


#endif




