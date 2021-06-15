/*
 *
 * GCLib.h
 * Wednesday, 4/2/1997.
 * Craig Fitzgerald
 *
 *
 ****************************************************************************
 *
 *  QUICK REF FUNCTION INDEX
 *  ========================
 *
 * Compiler Functions
 * ------------------
 * void Compile (PSZ pszIn, PSZ pszOut);
 * void SetCompileOption (UINT uOption, UINT uVal);
 * void GenSetOutputFn (PVOID pFn, UINT uFn);
 *
 *
 * Module Loader Functions
 * -----------------------
 * UINT LoadModule (PSZ pszFile);
 * LoadSetInputFn (PVOID pFn, UINT uFn);
 *
 *
 * Run Time Environment Functions
 * ------------------------------
 * void VMInit (UINT uStackSize, BOOL bDebug);
 * void VMTerm (void);
 * void InitInternalFunctions (void);
 * void AddFn (PVOID pFn, UINT uRetType, UINT uTag);
 * void OrderFn (void);
 * void VMExecMain (int argc, char *argv[]);
 * void VMExecFunction (PSZ pszFn);
 * long VMReturn (void);
 *
 ****************************************************************************
 *
 * This library provides compiler functions and a virtual machine
 * environment for the GC language.  GC is very much like C with
 * some restrictions and some extensions.
 * Differences from C:
 *
 * GC does not support:
 *    Struct initializers
 *    Anonymous nested structures or unions (who cares?)
 *    Function pointers of any kind (there is something similar)
 *    Linking (there is something similar)
 *    preprocessor macros (simple replacements are ok)
 *    you must use full param prototypes in function declarations
 *     and prototypes.
 *    Type checking may be incomplete
 *    switch statement does not support float type
 *    Multi dimensioned arrays:
 *       int array_2 [10][20]; // is not valid
 *
 *       typedef int Arr1[20];
 *       Arr1 array_2[10];     // is valid
 *
 *       array_2[1][1] = 20;   // is valid
 *
 *
 * GC does allow:
 *    Calling functions using the function name string.
 *    Calling any C that is part of the run time envirenment
 *    variable # of params (more or less than defd) for all calls
 *    startup and run time loading of code modules (kind of like DLLs)
 *
 *
 *
 * The basic idea is this:
 * 1> Make a compiler (or use GC.EXE)
 *    1.1> Create a program that calls:
 *           void Compile (PSZ pszIn, PSZ pszOut);
 *         and perhaps:
 *           void SetCompileOption (UINT uOption, UINT uVal);
 *
 * 2> Build a run time program (or use GX.EXE)
 *    2.1> Add a fn def in GNUSYS.GH for every C function you
 *         want accessable to your GC programs.
 *    1.1> Create a program that calls:
 *       VMInit (STACK_SIZE, bDebug); - init virtual machine
 *       InitInternalFunctions ();    - install GC internal support
 *       AddFn (malloc, DATATYPE_PTR, 101); - for each C fn in GNUSYS.GH
 *       OrderFn ();                  - after done AddFn calls
 *       LoadModule (szInFile);       - the file you compiled
 *       VMExecMain (argc, argv);     - this starts the execution
 *       uRet = (UINT)VMReturn ();    - the return from main
 *
 * 3> Create a sample GC program (for example test.gc)
 *    3.1> #include <GnuSys.gh>
 *    3.2> code must include a main fn
 *    3.3> Code like a C program.  Look in GnuSys.GH for available
 *         api calls. Use ParmCount, ParmType,ParmWord,etc... to
 *         access variable # of parm capabilities.  Use Call fn
 *         to access functions via string (replacement for fn ptrs).
 *         See examples of all this stuff.
 *
 * 4> Run the program
 *    4.1> Compile: GC test[.gc]
 *    4.2> exec:    GX test[.gx]
 *    or interpret  GI test[.gc]
 *
 ****************************************************************************
 *
 */


#define EXT_C   ".gc"
#define EXT_H   ".gh"
#define EXT_EXE ".gx"


/***************************************************************************/
/*                                                                         */
/*  Compiler functions                                                     */
/*                                                                         */
/***************************************************************************/

/*
 * This function compiles a gc script (.gc) file into a code file (.gx).
 * In its basic form, pszIn is the script file and pszOut is the gx
 * output file.
 *
 * Options can be set using the SetCompileOption fn below
 *
 * The generators output fns are implemented as function pointers so that
 * they may be redirected using GenSetOutputFn below
 *  (possible reasons are to leave code in mem for an interpreter or to
 *   direct to compression or library writing functions)
 *
 * Compiler use:
 *  call SetCompileOption to set options
 *  optionally call GenSetOutputFn if redirecting output
 *  call Compile to compile the script
 */
void Compile (PSZ pszIn, PSZ pszOut);



#define CO_OPTIMIZE    1
#define CO_STRUCTPACK  2
#define CO_LOGVAL      3

/* option          value
 * -----------------------
 * CO_OPTIMIZE     0 - no peephole optimizations   \
 *                 1 - Stack management (ISP,DSP)  | may be combined
 *                 2 - Short push                  | use + or |
 *                 4 - Lvalue assignment shortcuts /
 *
 * CO_STRUCTPACK   # - struct packing boundry: when interfacing with C
 *                     this must be the same as the c code
 *                     (I pack ... structures only!)
 *
 * CO_LOGVAL       4 - dump parse trees
 *                 8 - dump token stream
 *
 */
void SetCompileOption (UINT uOption, UINT uVal);

/***************************************************************************/

#define FTYP_WRITE_FILE_TAGS    1
#define FTYP_WRITE_FN           2
#define FTYP_WRITE_GLOBAL_START 3
#define FTYP_WRITE_GLOBAL_DATA  4
#define FTYP_WRITE_GLOBAL_END   5

/*
 * Use this to replace the default output functions
 * The default produces a legal GX module file.
 * The _keep function definitions defined immediately
 * below are a possible set of replacements that
 * redirect the output to the load module to support
 * an interpreter.  If you want to replace these fn's
 * yourself, I suggest looking at the code! (gen.c)
 *
 */
void GenSetOutputFn (PVOID pFn, UINT uFn);

/*keep.h*/
void _cdecl _keepTag       (UINT uTag);
void _cdecl _keepFunction  (PSZ pszName);
void _cdecl _keepGlobStart (UINT uLen);
void _cdecl _keepGlobData  (PVOID p, UINT uLen);
void _cdecl _keepGlobEnd   (void);


/**************************************************************************/
/*                                                                        */
/* Loader Functions                                                       */
/*                                                                        */
/**************************************************************************/

/*
 * This fn loads a GX module file into the virtual machine's memory
 *
 *
 */
UINT LoadGXModule (PSZ pszFile);

#define FTYP_LOAD_FILE_TAGS     1
#define FTYP_LOAD_FN            2
#define FTYP_LOAD_GLOBAL_START  3

/*
 * Use this to replace the default input functions The defaults read
 * a legal GX file.  You would only need to replace these functions
 * if you wanted to get the module from a source other than a simple file.
 * For example if you replaced the output compiler functions to write
 * using the CMP compression api from GnuLib, you would then need to
 * replace these functions and use the CMP api to read the file.
 */
void LoadSetInputFn (PVOID pFn, UINT uFn);

/**************************************************************************/
/*                                                                        */
/* Virtual machine functions                                              */
/*                                                                        */
/**************************************************************************/

/*
 * This is the startup fn for the VM (Virtual Machine).  This fn
 * malloc's a stack and sets the debug mode.  This fn could
 * conceivably be called multiple times to change the size of the
 * stack or to change the debug mode.
 */
void VMInit (UINT uStackSize, BOOL bDebug);

/*
 * VM shutdown function
 * This frees the stack
 */
void VMTerm (void);

/*
 * This installs the following internal functions:
 *   GNU_DumpStack, GNU_DumpMem, GNU_ParmCount, GNU_ParmType,
 *   GNU_ParmWord,  GNU_ParmFloat, GNU_CallByName, GNU_SetPtr,
 */
void InitInternalFunctions (void);



#define DATATYPE_VOID         0
#define DATATYPE_CHAR         1
#define DATATYPE_SHORT        2
#define DATATYPE_LONG         3
#define DATATYPE_INT          4
#define DATATYPE_FLOAT        5
#define DATATYPE_PTR          6

/*
 * This function adds external C functions to the GC VM.
 * To add a C function that is callable from a GC program do the following:
 *
 * 1> add a definition to the gnusys.gh include file:
 *          int myfunction (int i1, int i2): 666;
 *    Notice that this looks just like a c function prototype except for
 *    the : # part at the end.  This is a function tag.
 *
 * 2> Call this function (thats AddFn) like this:
 *          AddFn (myfunction, DATATYPE_INT, 666);
 *    Where uSize is the number of bytes returned from the function and
 *    uTag is the value specified as the Function Tag in step 1
 *
 *
 * By the way, if uTag is < 32000, the function is assumed to be a C
 * function, therefore call data on the GC stack is converted to call
 * data on the C (real) stack and the function is called ...
 * If the Tag is >= 32000, the function is assumed to be a GC internal
 * function and no stack buildup or tear down takes place so no params
 * or return values can be accessed without directly manipulating the
 * GC stack.  Basically dont use 32000 or larger (internal.c has examples)
 */
void AddFn (PVOID pFn, UINT uReturnType, UINT uTag);


/*
 * Call this after you are done with the AddFn calls.  This is necessary
 * due to the inherent laziness of the developer (me).  If your interested,
 * I add functions to a random location in a binary tree and then this fn
 * orders them.  I need to do this because AddFn calls are likely to add
 * functions in increasing Tag order causing my nice tree to look like a
 * wasteful linked list.  I could implement the tree as a Red-Black tree
 * but I don't like Georgia.  I could use standard Tree Balancing or B
 * trees or Hash tables, etc.. But They involve additional memory and
 * code overhead and besides, I'm lazy - Which may cause you to ask "why
 * then did he write this entire paragraph?"  Which I would then promptly
 * replay with "because I'm on my daily early morning (11:45) caffeine
 * buzz.
 */
void OrderFn (void);

/*
 * calls the main function
 * After loading the GX module via LoadModule and initializing the VM
 * via VMInit, and adding external functions via InitInternalFunctions
 * and AddFn, you can then execute the module calling this fn.  Note
 * that the fn wants your argc and argv.  This fn will strip off the first
 * param (ie the run time program) before passing them on to gc's main
 */
void VMExecMain (int argc, char *argv[]);

/*
 * This is like VMExecMain except:
 * You can call whatever function you want.
 * It does not setup the stack for you.  Therefore you should either
 * be calling a function defined like this  "void myfunction (void)"
 * or you must setup the stack by reading about how the stack is setup for
 * calling in gclib.txt and then putting stuff on the stack using VMSPush
 *
 * For example, this is what VMExecMain puts on the stack to call main:
 *
 *    VMSPush (0);                // place for return val;
 *    VMSPush ((ULONG)(argv + 1));// parm #2
 *    VMSPush (argc-1);           // parm #1
 *    VMSPush (0x00000020);       // status #2 (parm types info)
 *    VMSPush (0x00020202);       // status #1 (parm count info)
 *    VMSPush (-7);               // offset to return area
 */
void VMExecFunction (PSZ pszFn);


/*
 * after calling VMExecMain or VMExecFunction, you can get
 * the return value by calling this function
 * Although Return from VMExecFunction may be broke...
 */
long VMReturn (void);


/*
 * VM Stack manipulation
 * These are internal and should not be needed unless you plan on
 * doing something that will most likely get you in trouble
 *
 */
void VMSPush (LONG l);
void VMSPushF (BIG f);
LONG VMSPeek (void);
LONG VMSPeekIn (LONG l);
LONG VMSPop (void);
BIG  VMSPopF (void);
BIG  VMSPeekF (void);


/*
 * VM Global memory access
 * These are internal and should not be needed unless you plan on
 * doing something that will most likely get you in trouble
 */
void  VMGStoreByte (UCHAR c, LONG lAddr);
void  VMGStoreShort (SHORT s, LONG lAddr);
void  VMGStoreWord  (LONG l, LONG lAddr);
void  VMGStoreFloat (BIG f, LONG lAddr);
LONG  VMGGetByte  (LONG lAddr);
LONG  VMGGetShort (LONG lAddr);
LONG  VMGGetWord  (LONG lAddr);
BIG   VMGGetFloat (LONG lAddr);
PVOID VMGGetPtr (LONG lAddr);


/*
 * VM op stream functions
 * These are internal and should not be needed unless you plan on
 * creating a general uprising and squashing the establishment
 */
INT  VMCNextByte (void);
INT  VMCNextShort (void);
LONG VMCNextWord (void);


/*
 * When dealing directly with GC pointers a value <64K represents a
 * byte offset into the global memory area.  This fn converts that
 * value into a true memory pointer.  Soo you say.
 */
PSZ VMFixPtr (ULONG ul);

