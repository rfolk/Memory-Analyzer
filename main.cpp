#include <cstdint>
#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <map>


#include "page_table_element.h"
#include "memory_element.h"

uint64_t get_VPN (uint64_t);
uint64_t get_next_index (MEME *, uint64_t, uint64_t);
void eject_from_memory (std::string, int);
void insert_into_memory (MEME *, uint64_t, std::string, int);
bool valid_address (std::string);
bool valid_size (std::string);

// a few global variables...
uint64_t total_bytes_read = 0;
uint64_t total_bytes_write = 0;
uint64_t total_faults = 0;
uint64_t total_accessed = 0;
std::map<std::string, PTE> vpn_tracker;

const int page_size = 4096;

int main (int argc, char ** argv)
{
	std::string filename;
	std::ifstream trace_file;

	uint64_t memory_size = 8*1024*1024; // default is 8GB
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
	uint64_t array_size = memory_size / page_size;
	array_size--; // assume 1st level page table ALWAYS in memory
	MEME * in_memory = new MEME[array_size];
//	in_memory [array_size];
	uint64_t array_index = 0;

	//std::cout << argv[2] << " " << memory_size << " " << array_size << std::endl;


	// Open the file
	trace_file.open(filename);
	char operation;
	std::string virtual_address;
	std::string this_key;
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

		line_input[1] = line_input[1].substr(0, line_input[1].size() - 3);
		if (!valid_address(line_input[1]))
		{
			line_input.clear();
			continue;
		}
		virtual_address = line_input[1];
		this_key = line_input[1];//virtual_address;//get_VPN(virtual_address);

		if (!valid_size(line_input[2]))
		{
			line_input.clear();
			continue;
		}
		byte_size = std::stoi(line_input[2]);

		if (operation == 'R')
			total_bytes_read += byte_size;
		else
			total_bytes_write += byte_size;

		auto search = vpn_tracker.find(this_key);
		if (search != vpn_tracker.end())
		{
			// check if 2nd level not in memory
			if (!vpn_tracker[this_key].lvl_2_mem)
			{
				// find page to eject
				array_index = get_next_index(in_memory, array_size, array_index);
				eject_from_memory (in_memory[array_index].vpn, in_memory[array_index].level);

				// insert new page
				insert_into_memory (in_memory, array_index, this_key, 2);
			}
			else if (vpn_tracker[this_key].lvl_2_mem)
			{
				total_accessed++;
				vpn_tracker[this_key].lvl_2_clock = 1;
			}

			// check if 3rd level not in memory
			if (!vpn_tracker[this_key].lvl_3_mem)
			{
				// find page to eject
				array_index = get_next_index(in_memory, array_size, array_index);
				eject_from_memory (in_memory[array_index].vpn, in_memory[array_index].level);

				// insert new page
				insert_into_memory (in_memory, array_index, this_key, 3);
			}
			else if (vpn_tracker[this_key].lvl_3_mem)
			{
				total_accessed++;
				vpn_tracker[this_key].lvl_3_clock = 1;
			}

			// check if 4th level not in memory
			if (!vpn_tracker[this_key].lvl_4_mem)
			{
				// find page to eject
				array_index = get_next_index(in_memory, array_size, array_index);
				eject_from_memory (in_memory[array_index].vpn, in_memory[array_index].level);

				// insert new page
				insert_into_memory (in_memory, array_index, this_key, 4);
			}
			else if (vpn_tracker[this_key].lvl_4_mem)
			{
				total_accessed++;
				vpn_tracker[this_key].lvl_4_clock = 1;
			}

			vpn_tracker[this_key].num_accessed += 1;
		}
		else
		{
			PTE new_elem = {this_key, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			vpn_tracker.insert(std::pair<std::string, PTE>(this_key, new_elem));
			for (int i = 2; i < 5; ++i)
			{
				array_index = get_next_index(in_memory, array_size, array_index);
				eject_from_memory (in_memory[array_index].vpn, in_memory[array_index].level);
				insert_into_memory (in_memory, array_index, this_key, i);
			}
		}
		// std::cout << "num tokens: " << line_input.size() << std::endl;
		// {
		// 	virtual_address 
		// }
		//std::cout << operation << " " << std::hex << virtual_address << " " << std::hex << get_VPN(virtual_address) << std::endl;

		line_input.clear();
	}

	std::string most_accessed_vpn = "";
	uint64_t num_access_vpn = 0;
	for (auto& x: vpn_tracker)
	{
		if (x.second.num_accessed > num_access_vpn)
		{
			num_access_vpn = x.second.num_accessed;
			most_accessed_vpn = x.first;
		}
	}

	long double page_fault_rate = (long double) total_faults / (long double) total_accessed;

	std::cout << "Number of pages accessed: " << vpn_tracker.size() << std::endl;
	//std::cout << "faults " << total_faults << " accessed " << total_accessed << std::endl;
	std::cout << "Page fault rate: " << page_fault_rate << std::endl;
	std::cout << "Most accessed VPN: " << most_accessed_vpn << std::endl;
	std::cout << "Number of bytes read: " << total_bytes_read << std::endl;
	std::cout << "Number of bytes written: " << total_bytes_write << std::endl;
	std::cout << "Memory footprint: " << (page_size * (1 + vpn_tracker.size())) << std::endl;

	delete [] in_memory;

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
uint64_t get_next_index (MEME * memory_array, uint64_t size, uint64_t last_index)
{
	for (uint64_t i = 0; i < size; ++i)
	{
		if (memory_array[i].vpn.empty())
			return i;
	}
	uint64_t index = last_index;
	while (true)
	{
		std::string key = memory_array[index].vpn;
		int level = memory_array[index].level;
		// clock bit is 1, set to 0, move on
		if (level == 2)
		{
			if (vpn_tracker[key].lvl_2_clock)
			{
				vpn_tracker[key].lvl_2_clock = 0;
				index = (index + 1) % size;
			}
			else
			{
				//std::cout << "lvl2: " << index << std::endl;
				return index;
			}
		}
		else if (level == 3)
		{
			if (vpn_tracker[key].lvl_3_clock)
			{
				vpn_tracker[key].lvl_3_clock = 0;
				index = (index + 1) % size;
			}
			else
			{
				//std::cout << "lvl3: " << index << std::endl;
				return index;
			}
		}
		else //if (level == 4)
		{
			if (vpn_tracker[key].lvl_4_clock)
			{
				vpn_tracker[key].lvl_4_clock = 0;
				index = (index + 1) % size;
			}
			else
			{
				//std::cout << "lvl4: " << index << std::endl;
				return index;
			}
		}
	}
}

/**
  * Eject an entry in memory
  */
void eject_from_memory (std::string vpn, int level)
{
	if (level == 2)
	{
		vpn_tracker[vpn].lvl_2_mem = false;
	}
	else if (level == 3)
	{
		vpn_tracker[vpn].lvl_3_mem = false;
	}
	else
	{
		vpn_tracker[vpn].lvl_4_mem = false;
	}
	total_faults++;
	total_accessed++;
}

/**
  * Insert an entry into memory
  */
void insert_into_memory (MEME * in_memory, uint64_t index, std::string vpn, int level)
{
	in_memory[index] = {vpn, level};
	if (level == 2)
	{
		vpn_tracker[vpn].lvl_2_clock = 0;
		vpn_tracker[vpn].lvl_2_mem = true;
	}
	else if (level == 3)
	{
		vpn_tracker[vpn].lvl_3_clock = 0;
		vpn_tracker[vpn].lvl_3_mem = true;
	}
	else
	{
		vpn_tracker[vpn].lvl_4_clock = 0;
		vpn_tracker[vpn].lvl_4_mem = true;
	}
}

/**
  * Check for valid address...
  */
bool valid_address (std::string addr)
{
	return addr.compare(0, 2, "0x") == 0
		&& addr.size() > 2
		&& addr.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string::npos;
}

bool valid_size (std::string size)
{
	return size.find_first_not_of("0123456789") == std::string::npos;
}
