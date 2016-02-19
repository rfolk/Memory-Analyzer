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
 } PTE;

 #endif