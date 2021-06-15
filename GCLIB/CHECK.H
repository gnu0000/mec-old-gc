/*
 *
 * decorate.h
 * Thursday, 2/27/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 *
 * part of the compiler
 */

extern UINT uSTRUCT_PACK_SIZE;


void CheckDeclaration (PNODE pnode);

void CheckTypedef (PNODE pnode);

/*
 * make sure structure isn't already defined
 * make sure var names are unique in the structure
 * calc element offsets
 * calc structure size
 *
 */
void CheckStructDef (PNODE pStruct);


PSYM FindStructType (PNODE pnode, BOOL bDie);
