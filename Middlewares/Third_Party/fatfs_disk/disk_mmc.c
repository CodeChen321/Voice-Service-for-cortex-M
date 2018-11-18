#include "disk_mmc.h"
#include "stm32f769i_discovery_sd.h"
static volatile DSTATUS Stat = STA_NOINIT;
static uint8_t SDCARD_IsWriteEnabled(void) {
//	HAL_GPIO_ReadPin(,);
	
	/* Card is not write protected */
	return 1;
}
DSTATUS MMC_disk_status()
{
	Stat = STA_NOINIT;

	/* Check SDCARD status */
	if (BSP_SD_GetCardState() == MSD_OK) {
		Stat &= ~STA_NOINIT;
	} else {
		Stat |= STA_NOINIT;
	}

	/* Check if write enabled */
	if (SDCARD_IsWriteEnabled()) {
		Stat &= ~STA_PROTECT;
	} else {
		Stat |= STA_PROTECT;
	}

	return Stat;
}
DSTATUS MMC_disk_initialize()
{
	Stat = STA_NOINIT;

	/* Configure the SDCARD device */
	if (BSP_SD_Init() == MSD_OK) {
		Stat &= ~STA_NOINIT;
	} else {
		Stat |= STA_NOINIT;
	}

	return Stat;
}
DRESULT MMC_disk_read(
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */)
{
	uint32_t timeout = 100000;
	if (BSP_SD_ReadBlocks((uint32_t *)buff, sector, count, 1000) != MSD_OK) {
		return RES_ERROR;
	}
	while (BSP_SD_GetCardState() != MSD_OK) {
		if (timeout-- == 0) {
				return RES_ERROR;
		}
	}
	return RES_OK;
}
DRESULT MMC_disk_write(
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */)
{
  uint32_t timeout = 100000;
	if (BSP_SD_WriteBlocks((uint32_t *)buff, sector, count, 1000) != MSD_OK) {
		return RES_ERROR;
	}
	while (BSP_SD_GetCardState() != MSD_OK) {
		if (timeout-- == 0) {
				return RES_ERROR;
		}
	}
	return RES_OK;
}
DRESULT MMC_disk_ioctl(BYTE cmd, void *buff) {
	DRESULT res = RES_ERROR;
	HAL_SD_CardInfoTypeDef CardInfo;
  
	/* Check if init OK */
	if (Stat & STA_NOINIT) {
		return RES_NOTRDY;
	}
  
	switch (cmd) {
		/* Make sure that no pending write process */
		case CTRL_SYNC :
			res = RES_OK;
			break;

		/* Size in bytes for single sector */
		case GET_SECTOR_SIZE:
			*(WORD *)buff = BLOCKSIZE;
			res = RES_OK;
			break;

		/* Get number of sectors on the disk (DWORD) */
		case GET_SECTOR_COUNT :
			BSP_SD_GetCardInfo(&CardInfo);
			*(DWORD *)buff = CardInfo.LogBlockNbr;
			res = RES_OK;
			break;

		/* Get erase block size in unit of sector (DWORD) */
		case GET_BLOCK_SIZE :
			*(DWORD*)buff = CardInfo.LogBlockSize;
			break;

		default:
			res = RES_PARERR;
	}
  
	return res;
}
