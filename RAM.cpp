// File: RAM.cpp 
// Author: Oliver Carver 
// Date: 12/07/2021
// Section: 312-506 
// E-mail: jansenc@tamu.edu 
// Description:
// defines/implements functions for RAM class

#include "RAM.h"
#include "cache.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <bitset>
#include <algorithm>


void RAM::memoryInit(char* filename) {
	//make ram always 256
	memory.resize(256,0);
	std::string input;
	std::string line;
	std::istringstream inss;
	int start = 0;
	int end = 0;
	bool valid = 0;
	while (!valid) {
		std::cout << "initialize the ram:" << std::endl;
		std::getline(std::cin,input);
		inss.str(input);
		inss >> line;
		if (line != "init-ram") {
			std::cout << "Wrong command! Expecting \"init-ram start end\"" << std::endl;
			inss.str("");
			inss.clear();
			continue;
		}
		inss >> line;
		start = std::stoul(line, nullptr, 16);
		inss >> line;
		end = std::stoul(line, nullptr, 16);
		if (start < 0 || start > 256 || end < 0 || end > 256) { 
			std::cout << "Invalid Start! Must be 0" << std::endl;
			inss.str("");
			inss.clear();
			continue;
		}
		valid = 1;
	}

	std::ifstream infs(filename);
	int i = 0;
	//fill ram
	if (infs.is_open()) {
		while(getline(infs,input)) {
			memory[i++] = std::stoul(input, nullptr, 16);
		}
	}
	//fill the the user didnt want to load in with 0s
	std::fill(memory.begin() + end + 1, memory.end(), 0);
	size = end + 1;
	std::cin.clear();
}

const void RAM::memoryView() {
	//output values;
	std::cout << "memory_size:" << std::dec << size << std::endl
		<< "memory_content:" << std::endl
		<< "address:data" << std::endl;
	int i = 0;
	//output the memory to standard out
	for (const auto& it : memory) {
		if (i % 8 == 0) { 
			std::cout << "0x";
			outputHex(std::cout, i,false);
			std::cout << ':'; 
		}
		outputHex(std::cout, it, true);
		if ((i++ + 1) % 8 == 0) std::cout << std::endl;
		if (i == size) break; 
	}
}

const void RAM::memoryDump() {
	//dump the memory to ram.txt
	std::ofstream outfs("ram.txt");
	for (const auto& it : memory) {
		outputHex(outfs,it,false);
		outfs << std::endl;
	}
}



