/*
 *
 * gen.h
 * Thursday, 2/27/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 *
 * part of the interface is in gclib.h
 */

void GenInit (PSZ pszOutFile);
void GenTerm (void);

/*
 * void GenDumpTag       (UINT uTag);
 * void GenDumpFunction  (PSZ pszName);
 * void GenDumpGlobStart (UINT uLen);
 * void GenDumpGlobData  (PVOID p, UINT uLen);
 * void GenDumpGlobEnd   (void);
 */
extern void (_cdecl *GenDumpTag)       (UINT uTag)         ; 
extern void (_cdecl *GenDumpFunction)  (PSZ pszName)       ; 
extern void (_cdecl *GenDumpGlobStart) (UINT uLen)         ; 
extern void (_cdecl *GenDumpGlobData)  (PVOID p, UINT uLen); 
extern void (_cdecl *GenDumpGlobEnd)   (void)              ; 
