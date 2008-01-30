#ifndef _PS3RSX_IOCTL_H_
#define _PS3RSX_IOCTL_H_

#include <linux/ioctl.h>

/* ioctl */
#define PS3RSX_IOCTL_CONTEXT_INFO  _IOR('r', 0x100, int)
#define PS3RSX_IOCTL_KICK_FIFO     _IO('r', 0x101)
#define PS3RSX_IOCTL_EXCLUSIVE     _IO('r', 0x102)
#define PS3RSX_IOCTL_FLIP_SCRN 	   _IOR('r', 0x103, int)

struct ps3rsx_ioctl_context_info {
	__u32 hw_id;          /* hardware context number */
	__u32 vram_size;      /* size of available video memory */
	__u32 ioif_size;      /* size of system memory apperture */
	__u32 fifo_size;      /* size of command buffer */
	__u32 ctrl_size;      /* size of dma control registers */
	__u32 info_size;      /* size of driver info */
	__u32 reports_size;   /* size of reports */
	__u32 ioif_addr;      /* address of apperture in RSX memory space */
};

#endif /* _ASM_POWERPC_PS3RSX_H_ */
