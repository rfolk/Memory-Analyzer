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
 	bool clock_bit;
 	uint64_t num_accessed;

 	bool in_memory;
 	uint64_t index;
 } PTE;

 #endif