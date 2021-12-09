// File: cachesimulator.cpp 
// Author: Oliver Carver 
// Date: 12/07/2021
// Section: 312-506 
// E-mail: jansenc@tamu.edu 
// Description:
// runs/drives the program

#include "RAM.h"
#include "cache.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <bitset>

using namespace std;

// Prints the cache simulator menu to standard out
void printMenu() {
	cout << "*** Cache simulator menu *** \ntype one command: \n1. cache-read \n2. cache-write \n" 
		<< "3. cache-flush \n4. cache-view \n5. memory-view \n6. cache-dump \n7. memory-dump \n8. quit \n"
		<< "****************************" << endl;
}

int main(int argc, char** argv) {
	if (argc != 2) throw invalid_argument("Incorrect usage! ./cachesimulator <input file>");

	cout << "*** Welcome to the cache simulator ***" << endl;

	RAM mem;
	mem.memoryInit(argv[1]);
	
	Cache cache;
	cache.input(mem.size);
	string input;
	string token;
	while(true) {
		printMenu();
		getline(cin,input);
		stringstream inss(input);
		inss >> token;
		if(token == "cache-read") {
			unsigned long address;
			string temp;
			inss >> temp;
			//stoul converts the hex temp to a normal int
			address = stoul(temp, nullptr, 16);
			if (address < 0 || address > 256) cout << "Invalid address!" << endl;
			else cache.read(address,mem);
		} else if (token == "cache-write") {
			int value;
			unsigned long address;
			string temp;
			inss >> temp;
			address = stoul(temp, nullptr, 16);
			inss >> temp;
			value = stoul(temp, nullptr, 16);
			//cout << "the address is: " << address << endl;
			//cout << "the value is: " << value << endl;
			if (address < 0 || address > 256) cout << "Invalid address!" << endl;
			else cache.write(value,address,mem);
		} else if (token == "cache-flush") {
			cache.flush();
		} else if (token == "cache-view") {
			cache.view();
		} else if (token == "memory-view") {
			mem.memoryView();
		} else if (token == "cache-dump") {
			cache.dump();
		} else if (token == "memory-dump") {
			mem.memoryDump();
		} else if (token == "quit") break;
		else cout << "Invalid command!" << endl;
	}
}
