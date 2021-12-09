// File: cache.cpp
// Author: Oliver Carver 
// Date: 12/07/2021
// Section: 312-506 
// E-mail: jansenc@tamu.edu 
// Description:
// defines+implements cache functions 

#include "cache.h"
#include "RAM.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <bitset>
#include <stdlib.h>
#include <algorithm>

void Cache::input(int RAMSize) {
	int cacheSize,blockSize,assoc,replace,whit,wmiss;
	std::cout << "configure the cache:" << std::endl;
	std::cout << "cache size: ";
	std::cin >> cacheSize;
	std::cout << "data block size: ";
	std::cin >> blockSize;
	std::cout << "associativity: ";
	std::cin >> assoc;
	std::cout << "replacement policy: ";
	std::cin >> replace;
	std::cout << "write hit policy: ";
	std::cin >> whit;
	std::cout << "write miss policy: ";
	std::cin >> wmiss;
	//i want memBits in int but log2 only returns float/double
	memBits = static_cast<int>(std::log2(RAMSize));
	init(cacheSize, blockSize, assoc, replace, whit, wmiss);
	std::cout << "cache successfully configured!";
	std::cout << std::endl;
	//if this isnt included the program will prompt too fast
	std::cin.clear();
	std::cin.ignore(10000, '\n');
}

void Cache::init(int size, int blockSize, int assoc, int replace, int whit, int wmiss) {
	this->size = size;	
	if (size < 8 || size > 256) throw std::invalid_argument("Invalid cache size!");
	this->blockSize = blockSize;
	this->assoc = assoc;
	if (assoc < 0 || assoc > 4 || assoc == 3) throw std::invalid_argument("Invalid associativity!"); 
	this->replace = replace;
	this->whit = whit;
	this->wmiss = wmiss;

	//do calculations to get corresponding data for the cache
	numOfSets = size / (assoc * blockSize);
	if (numOfSets < 1) numOfSets = 1;
	//have to static cast to int since log2 only does doubles or floats
	idxBits = static_cast<int>(std::log2(numOfSets));
	blockBits = static_cast<int>(std::log2(blockSize));
	tagBits = memBits - (idxBits + blockBits); 

	//initalizing the vectors
	std::vector<Reg> set;
	Reg reg;
	evictionSets.resize(numOfSets);
	for (int i = 0; i < blockSize; i++) reg.data.push_back(0);
	for (int i = 0; i < assoc; i++) set.push_back(reg);
	for (int i = 0; i < numOfSets; i++) sets.push_back(set); 
	for (auto iter = sets.begin(); iter != sets.end(); iter++) {
		for (auto it = (*iter).begin(); it != (*iter).end(); it++) {
			(*it).index = std::distance((*iter).begin(),it); 
		}
	}
}

void Cache::readWriteInit(std::bitset<8> addressBits, int& offset, int& idx, int& tag) {
	//convert the bits to a string to parse
	std::string addressString = addressBits.to_string();
	tag = std::stoi(addressString.substr(0,tagBits), 0, 2);
	if (idxBits > 0) {
		idx = std::stoi(addressString.substr(tagBits,idxBits),0,2);
		offset = std::stoi(addressString.substr(tagBits+idxBits,blockBits),0,2);
	}
	else {
		idx = 0;
		offset = std::stoi(addressString.substr(tagBits + idxBits,blockBits),0,2);
	}
	/*
	std::cout << "bits:" << addressString << std::endl;
	std::cout << "blockbits:" << addressString.substr(tagBits + idxBits,blockBits) << std::endl;
	std::cout << "idxbits:" << addressString.substr(tagBits,idxBits) << std::endl;
	std::cout << "tagbits:" << addressString.substr(0,tagBits) << std::endl;
	*/
}

void Cache::read(unsigned long address, RAM& mem) {
	bool hit = false;
	bool foundEmptySet = false;
	int evictionLine = -1;
	int dirty = 0;
	std::bitset<8> addressBits{address};
	int offset, idx, tag;
	readWriteInit(addressBits, offset, idx, tag);
	int data;
	
	//labmda so i can reuse read throughout the function
	auto readAction = [&](Reg& readReg) {
		bool found = false;
		for (auto it = evictionSets.at(idx).begin(); it != evictionSets.at(idx).end(); it++) {
			if (*it == tag) {
				//if we find the reg tag in eviction sets, swap it one place up
				//place within set doesn't matter, only that it isnt at 0
				found = true;
				if(it != evictionSets.at(idx).end() - 1) std::iter_swap(it,std::next(it));
				break;
			}
		}
		//tempaddress to get to the start of data block
		int tempAddress = address;
		tempAddress -= tempAddress % blockSize;
		for (auto& iter : readReg.data) {
			//fill reg with memory from ram
			if (tempAddress < mem.size) iter = mem.memory.at(tempAddress++);
			else iter = 0;
		}
		//if didnt find in eviction set add it
		if (!found) evictionSets.at(idx).push_back(tag);
		data = readReg.data.at(offset);
	};

	//read data if hit
	for (auto& it : sets.at(idx)) {
		if (it.tag == tag) {
			if (it.valid == 1) {
				hit = true;
				numOfHits++;
				data = it.data.at(offset);
				break;
			}
		}
	}

	//if not hit write to empty line
	if (!hit) {
		numOfMisses++;
		for (auto it = sets.at(idx).begin(); it != sets.at(idx).end(); it++) {
			if ((*it).valid == 0) {
				//std::cout << "writing!" << std::endl;
				(*it).tag = tag;
				(*it).valid = 1;
				evictionLine = std::distance(sets.at(idx).begin(),it);
				foundEmptySet = true;
				(*it).RAMAddress = address;
				readAction((*it));
				break;
			}
		}
	}

	//eviction lambda that will write reg values to memory
	auto replaceEviction = [&] (Reg& evictLine) {
		int tempAddress = evictLine.RAMAddress;
		tempAddress -= tempAddress % blockSize;
		for (auto& iter : evictLine.data) {
			if(tempAddress < mem.size) mem.memory.at(tempAddress++) = iter;
			else break;
		}
	};

	if (!hit && !foundEmptySet) {
		if (replace == 1) {
			//get random cache line
			evictionLine = std::rand() % assoc; 
			if (sets.at(idx).at(evictionLine).dirty == 1) 
				replaceEviction(sets.at(idx).at(evictionLine));
		}
		if (replace == 2) {
			//this converts the tag value held in the evictionset to the cache line index within the set vector
			evictionLine = evictionSets.at(idx).at(0);
			for (auto it = sets.at(idx).begin(); it != sets.at(idx).end(); it++) {
				if ((*it).tag == evictionLine) {
					//swap eviction place up one
					std::iter_swap(evictionSets.at(idx).begin(),evictionSets.at(idx).begin()+1);
					evictionLine = std::distance(sets.at(idx).begin(),it);
					break;
				}
			}
			if(sets.at(idx).at(evictionLine).dirty == 1)
				replaceEviction(sets.at(idx).at(evictionLine));
		}
		//now do our normal read stuff
		readAction(sets.at(idx).at(evictionLine));
		sets.at(idx).at(evictionLine).tag = tag;
		sets.at(idx).at(evictionLine).dirty = 0;
		sets.at(idx).at(evictionLine).valid = 1;
		sets.at(idx).at(evictionLine).RAMAddress = address;
	}

	//output to standard out
	std::cout << "set:" << std::dec << idx << std::endl;
	std::cout << "tag:";
	outputHex(std::cout,tag,false);
	std::cout << std::endl;
	if(hit) std::cout << "hit:yes" << std::endl;
	else std::cout << "hit:no" << std::endl;
	std::cout << "eviction_line:" << std::dec << evictionLine << std::endl;
	std::cout << "ram_address:";
	if (!hit) { 
		std::cout<<"0x";
		outputHex(std::cout,address,false);
	}
	else std::cout << "-1";
	std::cout << std::endl;
	std::cout << "data:0x";
	outputHex(std::cout,data,false);
	std::cout << std::endl;
}

void Cache::write(int value, unsigned long address, RAM& mem) {
	bool hit = false;
	bool foundEmptySet = false;
	int evictionLine = -1;
	int dirty = 0;
	std::bitset<8> addressBits{address};
	int offset, idx, tag;
	readWriteInit(addressBits, offset, idx, tag);
	//std::cout << "offset: " << offset << " idx: " << idx << " tag: " << tag << std::endl;
	
	//lambda that does write functionality
	auto writeAction = [&](Reg& writeReg) {
		bool found = false;
		//handles eviction priority
		for (auto it = evictionSets.at(idx).begin(); it != evictionSets.at(idx).end(); it++) {
			if (*it == tag) {
				found = true;
				if(it != evictionSets.at(idx).end() - 1) std::iter_swap(it,std::next(it));
				break;
			}
		}
		int tempAddress = address;
		tempAddress -= tempAddress % blockSize;
		if (!hit && wmiss == 1) {
			for (auto& iter : writeReg.data) {
				if (tempAddress < mem.size) iter = mem.memory.at(tempAddress++);
				else iter = 0;
			}
		}
		if (!found) evictionSets.at(idx).push_back(tag);
		writeReg.data.at(offset) = value;
		//handle particularities of whit policies
		if (whit == 1) mem.memory.at(address) = value;
		else if (whit == 2) {
			dirty = writeReg.dirty = 1;
		}
	};

	//write if we have a hit
	for (auto& it : sets.at(idx)) {
		if (it.tag == tag) {
			if (it.valid == 1) {
				hit = true;
				numOfHits++;
				writeAction(it);
				break;
			}
		}
	}

	//write if no hit but empty line
	if (!hit) {
		numOfMisses++;
		if (wmiss == 2) {
			mem.memory.at(address) = value;
			foundEmptySet = true;
		}
		else if (wmiss == 1) {
			for (auto it = sets.at(idx).begin(); it != sets.at(idx).end(); it++) {
				if ((*it).valid == 0) {
					//std::cout << "writing!" << std::endl;
					(*it).tag = tag;
					(*it).valid = 1;
					evictionLine = std::distance(sets.at(idx).begin(),it);
					foundEmptySet = true;
					(*it).RAMAddress = address;
					writeAction((*it));
					break;
				}
			}
		}
	}
	//lambda that writes from memory to the cache in case of eviction
	auto replaceEviction = [&] (Reg& evictLine) {
		int tempAddress = evictLine.RAMAddress;
		tempAddress -= tempAddress % blockSize;
		for (auto& iter : evictLine.data) {
			if(tempAddress < mem.size) mem.memory.at(tempAddress++) = iter;
			else break;
		}
	};

	//code for if eviction
	if (!hit && !foundEmptySet) {
		if (replace == 1) {
			evictionLine = std::rand() % assoc; 
			if (sets.at(idx).at(evictionLine).dirty == 1) 
				replaceEviction(sets.at(idx).at(evictionLine));
		}
		if (replace == 2) {
			//convert tag from eviction set to an actual cache line
			evictionLine = evictionSets.at(idx).at(0);
			for (auto it = sets.at(idx).begin(); it != sets.at(idx).end(); it++) {
				if ((*it).tag == evictionLine) {
					std::iter_swap(evictionSets.at(idx).begin(),evictionSets.at(idx).begin()+1);
					evictionLine = std::distance(sets.at(idx).begin(),it);
					break;
				}
			}
			if(sets.at(idx).at(evictionLine).dirty == 1)
				replaceEviction(sets.at(idx).at(evictionLine));
		}
		sets.at(idx).at(evictionLine).tag = tag;
		sets.at(idx).at(evictionLine).dirty = 0;
		sets.at(idx).at(evictionLine).valid = 1;
		//zero out the cache line to prep for writing
		std::fill(sets.at(idx).at(evictionLine).data.begin(),sets.at(idx).at(evictionLine).data.end(),0);
		sets.at(idx).at(evictionLine).RAMAddress = address;
		writeAction(sets.at(idx).at(evictionLine));
	}

	
	//output to standard out
	std::cout << "set:" << std::dec << idx << std::endl;
	std::cout << "tag:";
	outputHex(std::cout,tag,false);
	std::cout << std::endl;
	std::cout << "write_hit:";
	if(hit) std::cout<< "yes" << std::endl;
	else std::cout << "no" << std::endl;
	std::cout << "eviction_line:" << std::dec << evictionLine << std::endl;
	std::cout << "ram_address:";
	if (!hit) { 
		std::cout<<"0x";
		outputHex(std::cout,address,false);
	}
	else std::cout << "-1";
	std::cout << std::endl;
	std::cout << "data:0x";
	outputHex(std::cout,value,false);
	std::cout << std::endl;
	std::cout << "dirty_bit:" << std::dec << dirty << std::endl; 
}

const void Cache::view() {
	//output to standard out
	std::cout << "cache_size:" << std::dec << size << std::endl;
	std::cout << "data_block_size:" << std::dec << blockSize << std::endl;
	std::cout << "associativity:" << std::dec << assoc << std::endl;

	std::cout << "replacement_policy:";
	if (replace == 1) std::cout << "random_replacement"; 
	else std::cout << "least_recently_used";
	std::cout << std::endl;

	std::cout << "write_hit_policy:";
	if (whit == 1) std::cout << "write_through"; 
	else std::cout << "write_back";
	std::cout << std::endl;

	std::cout << "write_miss_policy:";
	if (wmiss == 1)  std::cout << "write_allocate"; 
	else std::cout << "no_write_allocate";
	std::cout << std::endl;

	std::cout << "number_of_cache_hits:" << std::dec << numOfHits << std::endl;
	std::cout << "number_of_cache_misses:" << std::dec << numOfMisses << std::endl;
	std::cout << "cache_content:" << std::endl;
	//std::cout << "idxBits: " << idxBits << " blockBits: " << blockBits << " tagBits: " << tagBits
	//	<< " membits: " << memBits << std::endl;

	//iterate through the sets and print out contents of the registers
	for (const auto& set : sets) {
		for (const auto& line : set) {
			std::cout << line.valid << ' ' << line.dirty << ' ';
			outputHex(std::cout, line.tag,true);
			for (const auto& block : line.data)  
				outputHex(std::cout, block,true);
			std::cout << std::endl;
		}
	}
}

const void Cache::dump() {
	//dump to cache.txt
	std::ofstream outfs("cache.txt");
	for (const auto& set : sets) {
		for (const auto& line: set) {
			for (const auto& block : line.data)
				outputHex(outfs,block,true);
			outfs << std::endl;
		}
	}
}

void Cache::flush() {
	//reset every register and eviction set
	for (auto& set : sets) {
		for (auto& line: set) {
				line.clear();
		}
	}
	for (auto& set : evictionSets) {
		set.clear();
	}
	std::cout << "cache_cleared" << std::endl;
}






