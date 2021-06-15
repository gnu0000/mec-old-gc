/*
 *
 * mem.h
 * Monday, 3/17/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 */


/*
 * a variable of a given type will take up how many bytes
 * of memory you ask?
 */
UINT MemSize (UINT uTypeVal, PSYM psStruct);


/*
 *
 */
UINT MemSymSize (PSYM ps);

/*
 * a variable of a given type will take up how much room
 * on the stack you ask?
 */
UINT MemStkSize (UINT uTypeVal, PSYM psStruct);

