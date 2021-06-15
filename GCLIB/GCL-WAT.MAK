ODIR  = o\$(MODE)32
GCLIB = gc$(MODE)3


!IF "$(MODE)" == "D"
CFLAGS = -mf -bt=dos -fp5 -3s -s -w3 -ze -zp1 -zq -od -d2 -fo=$(ODIR)\$(@B).obj
!ENDIF

!IF "$(MODE)" == "O"
CFLAGS = -mf -bt=dos -fp5 -3s -s -w3 -ze -zp1 -zq -omaxnet -fo=$(ODIR)\$(@B).obj
!ENDIF

!IF "$(MODE)" == "M"
echo not supported!
!ENDIF

all : $(GCLIB).lib


#
# Make The GCLib library
#

$(ODIR)\COMPILE.obj : COMPILE.c gclib.h gc.h define.h symbol.h tokenize.h parse.h gen.h error.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\ERROR.obj : ERROR.c gclib.h gc.h symbol.h tokenize.h parse.h check.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\DEFINE.obj : DEFINE.c gclib.h gc.h symbol.h tokenize.h error.h define.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\SYMBOL.obj : SYMBOL.c gclib.h gc.h symbol.h error.h type.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\MEM.obj : MEM.c gclib.h gc.h error.h opcodes.h type.h mem.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\NODE.obj : NODE.c gclib.h gc.h tokenize.h error.h opcodes.h type.h parse.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\TOKENIZE.obj : TOKENIZE.c gclib.h gc.h symbol.h tokenize.h error.h define.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\PARSE.obj : PARSE.c gclib.h gc.h symbol.h tokenize.h parse.h check.h error.h gen.h genfn.h genglob.h type.h node.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\TYPE.obj : TYPE.c gclib.h gc.h symbol.h tokenize.h parse.h check.h error.h opt.h type.h node.h mem.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\CHECK.obj : CHECK.c gclib.h gc.h symbol.h tokenize.h parse.h check.h error.h opt.h type.h mem.h opcodes.h node.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\LABEL.obj : LABEL.c gclib.h gc.h label.h genop.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\OPT.obj : OPT.c gclib.h gc.h symbol.h tokenize.h parse.h check.h error.h type.h opcodes.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\GEN.obj : GEN.c gclib.h gclib.h gc.h label.h binfile.h error.h genop.h gen.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\GENEXPR.obj : GENEXPR.c gclib.h gc.h symbol.h check.h genfn.h genglob.h error.h label.h genexpr.h genop.h opcodes.h type.h mem.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\GENFN.obj : GENFN.c gclib.h gc.h symbol.h tokenize.h parse.h check.h label.h genfn.h genglob.h genexpr.h genop.h gen.h opcodes.h binfile.h type.h error.h mem.h node.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\GENGLOB.obj : GENGLOB.c gclib.h gc.h symbol.h tokenize.h parse.h check.h label.h genglob.h gen.h genop.h opcodes.h binfile.h type.h error.h mem.h node.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\GENOP.obj : GENOP.c gclib.h gc.h symbol.h tokenize.h parse.h check.h label.h genexpr.h opcodes.h binfile.h genfn.h genop.h error.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\KEEP.obj : KEEP.c gclib.h gc.h label.h binfile.h codmaint.h genop.h gen.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\CODMAINT.obj : CODMAINT.c gclib.h gc.h binfile.h codmaint.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\LOADGX.obj : LOADGX.c gclib.h gc.h binfile.h codmaint.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\EXTERN.obj : EXTERN.c gclib.h gc.h opcodes.h binfile.h internal.h codmaint.h type.h vm.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\INTERNAL.obj : INTERNAL.c gclib.h gc.h opcodes.h codmaint.h vm.h extern.h 
   wcc386 $(CFLAGS) $(@B).c

$(ODIR)\VM.obj : VM.c gclib.h gc.h opcodes.h binfile.h extern.h codmaint.h vm.h 
   wcc386 $(CFLAGS) $(@B).c

$(GCLIB).lib : $(ODIR)\COMPILE.obj $(ODIR)\ERROR.obj $(ODIR)\DEFINE.obj $(ODIR)\SYMBOL.obj $(ODIR)\MEM.obj $(ODIR)\NODE.obj $(ODIR)\TOKENIZE.obj $(ODIR)\PARSE.obj $(ODIR)\TYPE.obj $(ODIR)\CHECK.obj $(ODIR)\LABEL.obj $(ODIR)\OPT.obj $(ODIR)\GEN.obj $(ODIR)\GENEXPR.obj $(ODIR)\GENFN.obj $(ODIR)\GENGLOB.obj $(ODIR)\GENOP.obj $(ODIR)\KEEP.obj $(ODIR)\CODMAINT.obj $(ODIR)\LOADGX.obj $(ODIR)\EXTERN.obj $(ODIR)\INTERNAL.obj $(ODIR)\VM.obj 
   wlib -b $(GCLIB) @<<lib.lst
 -+$(ODIR)\COMPILE.obj -+$(ODIR)\ERROR.obj -+$(ODIR)\DEFINE.obj 
 -+$(ODIR)\SYMBOL.obj -+$(ODIR)\MEM.obj -+$(ODIR)\NODE.obj 
 -+$(ODIR)\TOKENIZE.obj -+$(ODIR)\PARSE.obj -+$(ODIR)\TYPE.obj 
 -+$(ODIR)\CHECK.obj -+$(ODIR)\LABEL.obj -+$(ODIR)\OPT.obj 
 -+$(ODIR)\GEN.obj -+$(ODIR)\GENEXPR.obj -+$(ODIR)\GENFN.obj 
 -+$(ODIR)\GENGLOB.obj -+$(ODIR)\GENOP.obj -+$(ODIR)\KEEP.obj 
 -+$(ODIR)\CODMAINT.obj -+$(ODIR)\LOADGX.obj -+$(ODIR)\EXTERN.obj 
 -+$(ODIR)\INTERNAL.obj -+$(ODIR)\VM.obj 
<<KEEP
