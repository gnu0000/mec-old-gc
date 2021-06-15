/*
 *
 * opcodes.h
 * Tuesday, 3/4/1997.
 * Craig Fitzgerald
 *
 * opcodes for the virtual GNU stack machine
 * 
 * 
 * OP_ADDRG 
 * 
 */

#define M_BYTE  0x02
#define M_SHORT 0x03
#define M_WORD  0x00
#define M_FLOAT 0x01

#define MEMSIZE_BYTE   1
#define MEMSIZE_SHORT  2
#define MEMSIZE_WORD   4
#define MEMSIZE_FLOAT  8

#define STACK_ELEMENT_SIZE 4


//          size code comment
//--------------------------------------------------
//c - char   1   10
//s - short  2   11   typically an offset
//w - word   4   00   stack sized word - int, or address
//f - float  8   01   float value
//n - any

//       name      val       bin        oper    stack     types  comments
//------------------------------------------------------------------------------------------------
#define  OP_NOP    0x00   // 0000 0000                          no op
#define  OP_CALL   0x01   // 0000 0001  str                     call user fn
#define  OP_CALLI  0x02   // 0000 0010  short                   call internal fn #w1
#define  OP_JMP    0x03   // 0000 0011  offset                  jump. w1 is +- offset
#define  OP_JZ     0x04   // 0000 0100  offset  w->        w    jump if zero
#define  OP_JNZ    0x05   // 0000 0101  offset  w->        w    jump if not zero
#define  OP_JZK    0x06   // 0000 0110  offset  w->w       w    jump if zero (keep stack)
#define  OP_JNZK   0x07   // 0000 0111  offset  w->w       w    jump if not zero (keep stack)

#define  OP_PUSHSF 0x08   // 0000 1000          ->sp            push stack frame 
#define  OP_PUSHSP 0x09   // 0000 1001          ->sp            push stack ptr
#define  OP_POPSF  0x0A   // 0000 1010          sf->            pop stack frame
#define  OP_PUSH0  0x0B   // 0000 1011          ->0        w    push a 0 onto stack
#define  OP_PUSH1  0x0C   // 0000 1100          ->1        w    push a 1 onto stack
#define  OP_DUPS   0x0D   // 0000 1101          n1->n1,n1  w    duplicate top word
#define  OP_SWAPS  0x0E   // 0000 1110        n1,n2->n2,n1 w    swap top 2 stack words
                     
#define  OP_POPL   0x10   // 0001 00xx  offset  n1->       cwf  pop to local var
#define  OP_POPLS  0x14   // 0001 01xx          n1,w->     cwf  pop to local var
#define  OP_POPG   0x18   // 0001 10xx  addr    n1->       cwf  pop to global var
#define  OP_POPGS  0x1C   // 0001 11xx          n1,w->     cwf  pop to global var

#define  OP_PUSHL  0x20   // 0010 00xx  offset  ->n        cwf  push from local var
#define  OP_PUSHLS 0x24   // 0010 01xx          w1->n      w    push from local var
#define  OP_PUSHG  0x28   // 0010 10xx  addr    ->n        cwf  push from global var
#define  OP_PUSHGS 0x2C   // 0010 11xx          w1->n      cwf  push from global var
#define  OP_PUSHI  0x30   // 0011 00xx  n       ->n        cswf push from literal

#define  OP_INCL   0x40   // 0100 00xx  offset             cwf  increment locl variable directly
#define  OP_INCG   0x44   // 0100 01xx  addr               cwf  increment glbl variable directly
#define  OP_DECL   0x48   // 0100 10xx  offset             cwf  decrement locl variable directly
#define  OP_DECG   0x4C   // 0100 11xx  addr               cwf  decrement glbl variable directly
                          //
#define  OP_ISP    0x50   // 0101 0000  s       ???             inc SP by s words
#define  OP_DSP    0x51   // 0101 0001  s       ???             dec SP by s words

#define  OP_FTOW   0x52   // 0101 0010          f->w       f    cvt f -> w
#define  OP_WTOF   0x53   // 0101 0011          w->f       w    cvt w -> f

#define  OP_ADDRL  0x54   // 0101 0100          w1->w2     w    cvt local addr to global ptr
#define  OP_ADDRG  0x55   // 0101 0101          w1->w2     w    cvt global addr to global ptr

#define  OP_BYTE   0x56   // 0101 0110          w->b       w    top of stk truncate to byte
#define  OP_SHORT  0x57   // 0101 0111          w->s       w    top of stk truncate to short

#define  OP_SAVL   0x60   // 0110 00xx  offset  n1->       cwf  top of stk to local var
#define  OP_SAVLS  0x64   // 0110 01xx          n1,w->     cwf  top of stk to local var
#define  OP_SAVG   0x68   // 0110 10xx  addr    n1->       cwf  top of stk to global var
#define  OP_SAVGS  0x6C   // 0110 11xx          n1,w->     cwf  top of stk to global var

#define  OP_SWITCH 0x70   // 0111 0000  tabl    w1->       w    tabl is size, bDefault, (val,jmp)@ list

#define  OP_PUSHNL 0x72   // 0111 0010  offset  ct->dat    c    push n bytes
#define  OP_PUSHNG 0x73   // 0111 0011  addr    ct->dat    c    push n bytes
#define  OP_POPNL  0x74   // 0111 0100  offset  ct,dat->   c    pop n bytes
#define  OP_POPNG  0x75   // 0111 0101  addr    ct,dat->   c    pop n bytes
#define  OP_POPNLS 0x76   // 0111 0110  offset  ct,off,dat-> c  pop n bytes
#define  OP_SAVNL  0x77   // 0111 0111  offset  ct,dat->dat  c  pop n bytes
#define  OP_SAVNG  0x78   // 0111 1000  addr    ct,dat->dat  c  pop n bytes


#define  OP_GT     0x80   // 1000 000x          n1,n2->w   wf   comparison: >
#define  OP_LT     0x82   // 1000 001x          n1,n2->w   wf   comparison: <
#define  OP_GE     0x84   // 1000 010x          n1,n2->w   wf   comparison: >=
#define  OP_LE     0x86   // 1000 011x          n1,n2->w   wf   comparison: <=
#define  OP_EQ     0x88   // 1000 100x          n1,n2->w   wf   comparison: equal
#define  OP_NE     0x8A   // 1000 101x          n1,n2->w   wf   comparison: not equal
#define  OP_ZE     0x8C   // 1000 110x          n->w       wf   comparison: is zero
#define  OP_NZ     0x8E   // 1000 111x          n->w       wf   comparison: is non zero
                          //
#define  OP_ADD    0x90   // 1001 000x          n1,n2->n3  wf   arithmetic +
#define  OP_SUB    0x92   // 1001 001x          n1,n2->n3  wf   arithmetic -
#define  OP_MUL    0x94   // 1001 010x          n1,n2->n3  wf   arithmetic *
#define  OP_DIV    0x96   // 1001 011x          n1,n2->n3  wf   arithmetic /
#define  OP_MOD    0x98   // 1001 100x          n1,n2->n3  wf   arithmetic %
#define  OP_NEG    0x9A   // 1001 101x          n1->n2     wf   arithmetic -
                                                                
#define  OP_SHR    0xA0   // 1010 0000          w1,w2->w3  w    arithmetic >>
#define  OP_SHL    0xA2   // 1010 0010          w1,w2->w3  w    arithmetic <<
#define  OP_AND    0xA4   // 1010 0100          w1,w2->w3  w    arithmetic &
#define  OP_XOR    0xA6   // 1010 0110          w1,w2->w3  w    arithmetic ^
#define  OP_OR     0xA8   // 1010 1000          w1,w2->w3  w    arithmetic |
#define  OP_NOT    0xAA   // 1010 1010          w1->w2     w    arithmetic ~
                          //  
#define  OP_LAND   0xB0   // 1011 000x          n1,n2->w   wf   boolean and
#define  OP_LOR    0xB2   // 1011 001x          n1,n2->w   wf   boolean or
#define  OP_LNOT   0xB4   // 1011 010x          n->w       wf   boolean not

#define  OP_SHL1   0xB8   // 1011 1000          w->w       w    shift left 1
#define  OP_SHL2   0xB9   // 1011 1001          w->w       w    shift left 2
#define  OP_SHL3   0xBA   // 1011 1010          w->w       w    shift left 3



