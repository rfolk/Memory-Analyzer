#include <cstdint>
#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <map>


#include "page_table_element.h"

uint64_t get_VPN (uint64_t);
uint64_t get_next_index (uint64_t *, uint64_t, uint64_t);

// a few global variables...
uint64_t total_bytes_read = 0;
uint64_t total_bytes_write = 0;
uint64_t total_faults = 0;
uint64_t total_accessed = 0;
std::map<uint64_t, PTE> vpn_tracker;

int main (int argc, char ** argv)
{
	std::string filename;
	std::ifstream trace_file;

	uint64_t memory_size = 8*1024*1024; // default is 8B
	// Check if all arguments are given in command line
	if (argc != 3)
	{
		std::cout << "Please supply two arguements: <trace file> <physical memory size (B)>." << std::endl;
		return -1;
	}

	// Get the filename
	filename.assign(argv[1]);
	// Assign the memory size
	memory_size = std::stol(argv[2], nullptr, 10);

	// allocate array
	uint64_t array_size = memory_size / 4194304;
	uint64_t in_memory [array_size];
	uint64_t array_index = 0;

	std::cout << argv[2] << " " << memory_size << " " << array_size << std::endl;


	// Open the file
	trace_file.open(filename);
	char operation;
	uint64_t virtual_address;
	uint64_t this_key;
	int byte_size;
	std::string file_line;
	std::vector<std::string> line_input;
	std::string token;

	while (std::getline(trace_file, file_line))
	{
		// tokenize string
		std::istringstream ss(file_line);
		while(std::getline(ss, token, ' '))
		{
			line_input.push_back(token);
			//std::cout << token << std::endl;
		}

		if (line_input.size() != 3)
		{
			line_input.clear();
			continue;
		}
		
		operation = line_input[0].at(0);
		if (operation != 'R' && operation != 'W')
		{
			line_input.clear();
			continue;
		}

		total_accessed++;
		virtual_address = std::stol(line_input[1], nullptr, 16);
		this_key = get_VPN(virtual_address);
		byte_size = std::stoi(line_input[2]);

		if (operation == 'R')
			total_bytes_read += byte_size;
		else
			total_bytes_write += byte_size;

		auto search = vpn_tracker.find(this_key);
		if (search != vpn_tracker.end())
		{
			// check if not in memory
			if (!vpn_tracker[this_key].in_memory)
			{
				// find page to eject
				array_index = get_next_index(in_memory, array_size, array_index);
				vpn_tracker[in_memory[array_index]].in_memory = false;

				// insert new page
				in_memory[array_index] = this_key;
				vpn_tracker[this_key].clock_bit = false;
			}
			else
				vpn_tracker[this_key].clock_bit = true;
			vpn_tracker[this_key].in_memory = true;
			vpn_tracker[this_key].num_accessed += 1;
		}
		else
		{
			array_index = get_next_index(in_memory, array_size, array_index);
			vpn_tracker[in_memory[array_index]].in_memory = false;
			in_memory[array_index] = this_key;
			PTE tmp = {this_key, true, false, array_index, 1};
			vpn_tracker.insert(std::pair<uint64_t, PTE>(this_key, tmp));
		}
		// std::cout << "num tokens: " << line_input.size() << std::endl;
		// {
		// 	virtual_address 
		// }
		//std::cout << operation << " " << std::hex << virtual_address << " " << std::hex << get_VPN(virtual_address) << std::endl;

		line_input.clear();
	}


	uint64_t most_accessed_vpn = 0;
	uint64_t num_access_vpn = 0;
	for (auto& x: vpn_tracker)
	{
		if (x.second.num_accessed > num_access_vpn)
		{
			num_access_vpn = x.second.num_accessed;
			most_accessed_vpn = x.first;
		}
	}

	std::cout << "Number of pages access: " << vpn_tracker.size() << std::endl;
	std::cout << "Most accessed VPN: " << std::hex << most_accessed_vpn << std::endl;
	std::cout << "Number of bytes read: " << total_bytes_read << std::endl;
	std::cout << "Number of bytes written: " << total_bytes_write << std::endl;

	// uint64_t hex_value = 0;
	// std::cin >> std::hex >> hex_value;

	// std::cout << std::hex << get_VPN(hex_value) << std::endl;

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

/**
  * Find next viable index for fault
  */
uint64_t get_next_index (uint64_t * memory_array, uint64_t size, uint64_t last_index)
{
	uint64_t index = last_index;
	while (true)
	{
		uint64_t key = memory_array[index];
		if (key == 0)
		{
			return index;
		}
		// clock bit is 1, set to 0, move on
		if (vpn_tracker[key].clock_bit)
		{
			vpn_tracker[key].clock_bit = false;
			index = (index + 1) % size;
		}
		else
		{
			total_faults++;
			return index;
		}
	}
}
