#include <cstdint>
#include <iostream>
#include <vector>
#include <iomanip>
#include <string>

#include "page_table_element.h"

uint64_t get_VPN (uint64_t);

int main (int argc, char ** argv)
{
	std::string filename;
	int memory_size = 8; // default is 8B
	// Check if all arguments are given in command line
	if (argc != 3)
	{
		std::cout << "Please supply two arguements: <trace file> <physical memory size (B)>." << std::endl;
		return -1;
	}

	// Get the filename
	filename.assign(argv[1]);
	// Assign the memory size
	memory_size = std::stoi(argv[2]);


	uint64_t hex_value = 0;
	std::cin >> std::hex >> hex_value;

	std::cout << std::hex << get_VPN(hex_value) << std::endl;

	// Shift right 
	hex_value >>= 12;

	std::cout << std::hex << hex_value << std::endl;

	return 0;
}

/**
  * Converts a virtual address into the physical address
  * 48 bit virtual address / 12 bit page = 36 bit VPN
  */
uint64_t get_VPN (uint64_t virtual_address)
{
	return virtual_address >> 12;
}