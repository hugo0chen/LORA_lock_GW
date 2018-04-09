#include "flash.h"

static u8 checkTargetAddr(u32 startAddr, u32 len);
static u8 computeForEraseCnt(u32 startAddr, u32 len, u8 size);
static u8 putFlashData(u32 startAddr, void *pBuf, u32 len, u8 size);
static u8 putFlashHalfWord(u32 startAddr, u16 *pBuf, u32 len);
static u8 putFlashWord(u32 startAddr, u32 *pBuf, u32 len);
/***************************************************************************************************
* ???:?flash??32????????
* ??:  ????,??,????
* ???:??????,0:??,1:??
* ?:	 1?????????USER_FLASH_START_ADDR,??USER_FLASH_END_ADDR,???????
*		 2?Flash??????????????????,????????????????
****************************************************************************************************/
u8 putWordData(u32 startAddr, u32 *pBuf, u32 len)				
{
	return putFlashData(startAddr, pBuf, len, 4);
}

/***************************************************************************************************
* ???:?flash??16????????
* ??:  ????,??,????
* ???:??????,0:??,1:??
* ?:	 1?????????USER_FLASH_START_ADDR,??USER_FLASH_END_ADDR,???????
*		 2?Flash??????????????????,????????????????
****************************************************************************************************/
u8 putHalfWordData(u32 startAddr, u16 *pBuf, u32 len)
{
	return putFlashData(startAddr, pBuf, len, 2);
}

/***************************************************************************************************
* ???:?flash????????32????
* ??:  ????,????,????
* ???:?
****************************************************************************************************/
void getWordData(u32 startAddr, u32* pBuf, u32 len)
{
	u32 i;
	for (i = 0; i < len; i++)
	{
		pBuf[i] = *((u32*)startAddr + i);
	}
}

/***************************************************************************************************
* ???:?flash????????16????
* ??:  ????,????,????
* ???:?
****************************************************************************************************/
void getHalfWordData(u32 startAddr, u16* pBuf, u32 len)
{
	u32 i;
	for (i = 0; i < len; i++)
	{
		pBuf[i] = *((u16*)startAddr + i);
	}
}

/***************************************************************************************************
* ???:?flash??????????
* ??:  ????,????,????,????
* ???:??????
****************************************************************************************************/
static u8 putFlashData(u32 startAddr, void *pBuf, u32 len, u8 size)
{
	u32 i;
	u8 eraseCnt;
	
	if (!checkTargetAddr(startAddr, len))
	{
		return 0;
	}
	eraseCnt = computeForEraseCnt(startAddr, len, size);
	FLASH_Unlock();

	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_BSY | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	for (i = 0; i < eraseCnt; i++)
	{
		if(FLASH_ErasePage(startAddr + FLASH_PAGE_SIZE * i) != FLASH_COMPLETE)	//????,?????????????????????
		{
			FLASH_Lock();
			return 0;
		}
	}

	switch(size)
	{
		case 2:
			if (!putFlashHalfWord(startAddr, pBuf, len))
			{
				return 0;
			}
			break;
		case 4:
			if (!putFlashWord(startAddr, pBuf, len))
			{
				return 0;
			}
			break;
		default:
			break;
	}

	FLASH_Lock();
	
	return 1;
}

static u8 putFlashHalfWord(u32 startAddr, u16 *pBuf, u32 len)
{
	int i;
	for (i = 0; i < len; i ++)
	{
		if (FLASH_ProgramHalfWord(startAddr + i * 2, pBuf[i]) != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return 0;
		}
	}
	return 1;
}

static u8 putFlashWord(u32 startAddr, u32 *pBuf, u32 len)
{
	int i;
	for (i = 0; i < len; i ++)
	{
		if (FLASH_ProgramWord(startAddr + i * 4, pBuf[i]) != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return 0;
		}
	}
	return 1;
}
/***************************************************************************************************
* ??:  ??????????????
* ??:  ????,????
* ???:????????,1:??,0:???
****************************************************************************************************/
static u8 checkTargetAddr(u32 startAddr, u32 len)
{
	if (len <= 0)
	{
		return 0;
	}
	if (startAddr < SYSTEM_FLASH_START_ADDR || startAddr >= USER_FLASH_END_ADDR)		//????????????
	{
		return 0;
	}
	
	if (startAddr + len >= USER_FLASH_END_ADDR)
	{
		return 0;
	}
	if (startAddr % 2 == 1)		//??????????
	{
		return 0;
	}
	return 1;
}

/***************************************************************************************************
* ??:  ????????
* ??:  ??????,????
* ???:?????????
****************************************************************************************************/
static u8 computeForEraseCnt(u32 startAddr, u32 len, u8 size)
{
	u32 end;
	u8 eraseCnt;
	end = startAddr + len * size;

	if ((end - SYSTEM_FLASH_START_ADDR) / FLASH_PAGE_SIZE != (startAddr - SYSTEM_FLASH_START_ADDR) / FLASH_PAGE_SIZE)
	{
		eraseCnt = (end - startAddr) / FLASH_PAGE_SIZE + 2;
	}
	else
	{
		eraseCnt = (end - startAddr) / FLASH_PAGE_SIZE + 1;
	}
	return eraseCnt;
}
