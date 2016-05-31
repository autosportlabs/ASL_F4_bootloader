_estack = 0x2000a000;

_Min_Heap_Size = 0x200;
_Min_Stack_Size = 0x200;

MEMORY
{
  FLASH         (rx)    : ORIGIN = 0x08000000, LENGTH = 16K
  HANDSHAKE     (rwx)   : ORIGIN = 0x20000000, LENGTH = 8
  RAM 		(rwx) 	: ORIGIN = 0x20000008, LENGTH = 40k - 8
  CCM           (rwx)   : ORIGIN = 0x10000000, LENGTH = 8K
}
