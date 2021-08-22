
#ifndef extmem_h
#define extmem_h

enum EextmemType {EEMT_Unknown,EEMT_M3,EEMT_SC,EEMT_EZ4,EEMT_DSBM};

extern void extmem_Init(void);
extern void extmem_Free(void);
extern bool extmem_ExistMemory(void);

extern EextmemType extmem_GetMemType(void);
extern u32 extmem_GetMemSize(void);

extern void extmem_SetCount(u32 Count);
extern bool extmem_Exists(u32 SlotIndex);
extern bool extmem_Alloc(u32 SlotIndex,u32 Size);
extern bool extmem_Write(u32 SlotIndex,void *pData,u32 DataSize);
extern bool extmem_Read(u32 SlotIndex,void *pData,u32 DataSize);

//extern bool extmem_WriteByte(u32 SlotIndex,void *pData,u32 Pos,u32 Size);
//extern bool extmem_ReadByte(u32 SlotIndex,void *pData,u32 Pos,u32 Size);

#endif

