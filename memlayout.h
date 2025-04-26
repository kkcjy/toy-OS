// -----Memory layout-----

// CPU has only 20 address lines in real mode
// extended memory refer to the part that exceeds the conventional 1MB memory limit
// ```start of extended memory```
#define EXTMEM  0x100000

// highest address that can be used for physical memory in system
// any attempt to access physical memory above this address is illegal     
// ```top of physical memory```
#define PHYSTOP 0xE000000

// devices in the system are usually mapped to the high-address space
// convenient to identify the memory area occupied by the devices
// ```start of device address```
#define DEVSPACE 0xFE000000         


// -----Key addresses for address space layout(vm.c)-----

// ```start of kernel virtual address```
#define KERNBASE 0x80000000        

// function calls and data access instructions within the kernel generate relative offsets based on link address
// ```address where kernel is linked```
#define KERNLINK (KERNBASE+EXTMEM)  

// ```convert a virtual address to a physical address```
#define V2P(a) (((uint) (a)) - KERNBASE)

// ```convert a physical address to a virtual address(without casts)```
#define P2V(a) ((void *)(((char *) (a)) + KERNBASE))

// ```convert a virtual address to a physical address```
#define V2P_WO(x) ((x) - KERNBASE)    

// ```convert a physical address to a virtual address(without casts)```
#define P2V_WO(x) ((x) + KERNBASE)   