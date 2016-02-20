/**
  * Russell Folk
  * C S 574
  *
  * Memory Element Structure
  */

 #ifndef MEMORY_ELEMENT
 #define MEMORY_ELEMENT

 #include <cstdint>

typedef struct meme
{
	uint64_t vpn;
	int level;
} MEME;

#endif