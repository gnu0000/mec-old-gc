!IF "$(MODE)" == "D"
OBJ   = o\d
COPT  = /c /W3 /AL /Od /Zip /Gy /nologo /Fo$(OBJ)\$(@B).obj 
GCLIB = GCDR

!ENDIF

!IF "$(MODE)" == "M"
OBJ = o\m
COPT = /c /W3 /AL /Od /Zip /Gy /nologo /Fo$(OBJ)\$(@B).obj /DDEBUG /DMEM_DEBUG
GCLIB = GCMR
!ENDIF

!IF "$(MODE)" == "O"
OBJ = o\o
COPT = /c /W3 /AL /Ox /Grs /Gy /Zp /nologo /Fo$(OBJ)\$(@B).obj 
GCLIB = GCOR
!ENDIF


all : $(GCLIB).lib

#DISASSEM,
#GC,
#GCPLAY,
#CFUNC,

#
# Make The GnuLib library
#

$(OBJ)\COMPILE.obj : COMPILE.c gclib.h gc.h define.h symbol.h tokenize.h parse.h gen.h error.h 
   cl $(COPT) $(@B).c

$(OBJ)\ERROR.obj : ERROR.c gclib.h gc.h symbol.h tokenize.h parse.h check.h 
   cl $(COPT) $(@B).c

$(OBJ)\DEFINE.obj : DEFINE.c gclib.h gc.h symbol.h tokenize.h error.h define.h 
   cl $(COPT) $(@B).c

$(OBJ)\SYMBOL.obj : SYMBOL.c gclib.h gc.h symbol.h error.h type.h 
   cl $(COPT) $(@B).c

$(OBJ)\MEM.obj : MEM.c gclib.h gc.h error.h opcodes.h type.h mem.h 
   cl $(COPT) $(@B).c

$(OBJ)\NODE.obj : NODE.c gclib.h gc.h tokenize.h error.h opcodes.h type.h parse.h 
   cl $(COPT) $(@B).c

$(OBJ)\TOKENIZE.obj : TOKENIZE.c gclib.h gc.h symbol.h tokenize.h error.h define.h 
   cl $(COPT) $(@B).c

$(OBJ)\PARSE.obj : PARSE.c gclib.h gc.h symbol.h tokenize.h parse.h check.h error.h gen.h genfn.h genglob.h type.h node.h 
   cl $(COPT) $(@B).c

$(OBJ)\TYPE.obj : TYPE.c gclib.h gc.h symbol.h tokenize.h parse.h check.h error.h opt.h type.h node.h mem.h 
   cl $(COPT) $(@B).c

$(OBJ)\CHECK.obj : CHECK.c gclib.h gc.h symbol.h tokenize.h parse.h check.h error.h opt.h type.h mem.h opcodes.h node.h 
   cl $(COPT) $(@B).c

$(OBJ)\LABEL.obj : LABEL.c gclib.h gc.h label.h genop.h 
   cl $(COPT) $(@B).c

$(OBJ)\OPT.obj : OPT.c gclib.h gc.h symbol.h tokenize.h parse.h check.h error.h type.h opcodes.h 
   cl $(COPT) $(@B).c

$(OBJ)\GEN.obj : GEN.c gclib.h gclib.h gc.h label.h binfile.h error.h genop.h gen.h 
   cl $(COPT) $(@B).c

$(OBJ)\GENEXPR.obj : GENEXPR.c gclib.h gc.h symbol.h check.h genfn.h genglob.h error.h label.h genexpr.h genop.h opcodes.h type.h mem.h 
   cl $(COPT) $(@B).c

$(OBJ)\GENFN.obj : GENFN.c gclib.h gc.h symbol.h tokenize.h parse.h check.h label.h genfn.h genglob.h genexpr.h genop.h gen.h opcodes.h binfile.h type.h error.h mem.h node.h 
   cl $(COPT) $(@B).c

$(OBJ)\GENGLOB.obj : GENGLOB.c gclib.h gc.h symbol.h tokenize.h parse.h check.h label.h genglob.h gen.h genop.h opcodes.h binfile.h type.h error.h mem.h node.h 
   cl $(COPT) $(@B).c

$(OBJ)\GENOP.obj : GENOP.c gclib.h gc.h symbol.h tokenize.h parse.h check.h label.h genexpr.h opcodes.h binfile.h genfn.h genop.h error.h 
   cl $(COPT) $(@B).c

$(OBJ)\KEEP.obj : KEEP.c gclib.h gc.h label.h binfile.h codmaint.h genop.h gen.h 
   cl $(COPT) $(@B).c

$(OBJ)\CODMAINT.obj : CODMAINT.c gclib.h gc.h binfile.h codmaint.h 
   cl $(COPT) $(@B).c

$(OBJ)\LOADGX.obj : LOADGX.c gclib.h gc.h binfile.h codmaint.h 
   cl $(COPT) $(@B).c

$(OBJ)\EXTERN.obj : EXTERN.c gclib.h gc.h opcodes.h binfile.h internal.h codmaint.h type.h vm.h 
   cl $(COPT) $(@B).c

$(OBJ)\INTERNAL.obj : INTERNAL.c gclib.h gc.h opcodes.h codmaint.h vm.h extern.h 
   cl $(COPT) $(@B).c

$(OBJ)\VM.obj : VM.c gclib.h gc.h opcodes.h binfile.h extern.h codmaint.h vm.h 
   cl $(COPT) $(@B).c

$(GCLIB).lib : $(OBJ)\COMPILE.obj $(OBJ)\ERROR.obj $(OBJ)\DEFINE.obj $(OBJ)\SYMBOL.obj $(OBJ)\MEM.obj $(OBJ)\NODE.obj $(OBJ)\TOKENIZE.obj $(OBJ)\PARSE.obj $(OBJ)\TYPE.obj $(OBJ)\CHECK.obj $(OBJ)\LABEL.obj $(OBJ)\OPT.obj $(OBJ)\GEN.obj $(OBJ)\GENEXPR.obj $(OBJ)\GENFN.obj $(OBJ)\GENGLOB.obj $(OBJ)\GENOP.obj $(OBJ)\KEEP.obj $(OBJ)\CODMAINT.obj $(OBJ)\LOADGX.obj $(OBJ)\EXTERN.obj $(OBJ)\INTERNAL.obj $(OBJ)\VM.obj 
  lib /PAGE:32 $*,y;
  lib /PAGE:32 @<<lib.lbr
      $* /nol
      -+$(OBJ)\COMPILE.obj -+$(OBJ)\ERROR.obj -+$(OBJ)\DEFINE.obj -+$(OBJ)\SYMBOL.obj -+$(OBJ)\MEM.obj -+$(OBJ)\NODE.obj -+$(OBJ)\TOKENIZE.obj -+$(OBJ)\PARSE.obj -+$(OBJ)\TYPE.obj -+$(OBJ)\CHECK.obj -+$(OBJ)\LABEL.obj -+$(OBJ)\OPT.obj -+$(OBJ)\GEN.obj -+$(OBJ)\GENEXPR.obj -+$(OBJ)\GENFN.obj -+$(OBJ)\GENGLOB.obj -+$(OBJ)\GENOP.obj -+$(OBJ)\KEEP.obj -+$(OBJ)\CODMAINT.obj -+$(OBJ)\LOADGX.obj -+$(OBJ)\EXTERN.obj -+$(OBJ)\INTERNAL.obj -+$(OBJ)\VM.obj ;
<<KEEP

