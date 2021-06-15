/*
 *
 * genop.h
 * Wednesday, 3/5/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 */


extern UINT uOPT;


/*
 * init code buffer
 *
 */
void CodInitBuffer (void);

/*
 * Get the current code buffer
 */
PSZ CodBuffer (PUINT puLen);


/*
 * called by label module
 * this says a label has been dropped to the current IP
 * therefore an opcode peephole optimization cannot take place here
 */
void CodClearMerge (void);

/* 
 * Add opcodes to the current code buffer
 */
void AddOp (UINT uOp, UINT uType);
void AddOpL (UINT uOp, UINT uType, PLBL pl);
void AddOpA (UINT uOp, UINT uType, ULONG ul, USHORT uScope);
void AddOpW (UINT uOp, UINT uType, LONG w);
void AddOpS (UINT uOp, UINT uType, INT s);
void AddOpC (UINT uOp, UINT uType, CHAR c);
void AddOpF (UINT uOp, UINT uType, BIG bg);


/* 
 * Add operands to the current code buffer
 */
void AddChar (INT iWord);
void SetShort (UINT uAddr, SHORT iWord);
void AddShort (SHORT iWord);
void AddWord (LONG l);
void AddFloat (BIG bg);

