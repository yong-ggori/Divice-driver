#ifndef __IOCTL_H__
#define __IOCTL_H__

#define IOCTLBLOCK_MAGIC 't'
typedef struct
{
	unsigned long intval;
} __attribute__((packed)) ioctl_block_info;

#define IOCTLBLOCK_INTVAL		_IOR(IOCTLBLOCK_MAGIC, 0, ioctl_block_info)	
#define IOCTLBLOCK_MAXNR		1
#endif
