/**
  * Russell Folk
  * C S 574
  *
  * Page Table Element Structure
  */

 #ifndef PAGE_TABLE_ELEMENT
 #define PAGE_TABLE_ELEMENT

 #include <cstdint>

typedef struct pte
{
	uint64_t VPN;
	uint64_t num_accessed;

	// page table level check
	bool lvl_2_mem;
	bool lvl_2_clock;
	uint64_t lvl_2_index;
	bool lvl_3_mem;
	bool lvl_3_clock;
	uint64_t lvl_3_index;
	bool lvl_4_mem;
	bool lvl_4_clock;
	uint64_t lvl_4_index;
} PTE;

#endif