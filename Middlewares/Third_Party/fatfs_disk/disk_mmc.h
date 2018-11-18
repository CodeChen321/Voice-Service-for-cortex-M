#ifndef _DISK_MMC_H_
#define _DISK_MMC_H_
#include "diskio.h"		/* FatFs lower layer API */









DSTATUS MMC_disk_status(void);
DSTATUS MMC_disk_initialize(void);
DRESULT MMC_disk_read(
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */);
DRESULT MMC_disk_write(
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */);
DRESULT MMC_disk_ioctl(BYTE cmd, void *buff);
#endif
