
OBJ  = o\$(MODE)
EXE  = exe\$(MODE)
LIB1 = Gnu16$(MODE)
LIB2 = gclib\GC$(MODE)R
LIBS = $(LIB1) $(LIB2)

!IF "$(MODE)" == "D"
COPT = /c /W3 /AL /Od /Zip /nologo /Fo$(OBJ)\$(@B).obj 
LOPT = /CO/NOLOGO/NOE/St:18000
!ENDIF

!IF "$(MODE)" == "M"
COPT = /c /W3 /AL /Od /Zip /nologo /Fo$(OBJ)\$(@B).obj /DDEBUG /DMEM_DEBUG
LOPT = /CO/NOLOGO/NOE/St:18000
!ENDIF

!IF "$(MODE)" == "O"
COPT = /c /W3 /AL /Ox /Grs /Zp /nologo /Fo$(OBJ)\$(@B).obj 
LOPT = /E/F/PACKC/NOLOGO/NOE/St:18000
!ENDIF

all: gc gx gi dis

gc:  $(EXE)\gc.exe 

gx:  $(EXE)\gx.exe 

gi:  $(EXE)\gi.exe 

dis: $(EXE)\dis.exe



$(OBJ)\gc.obj : gc.c gclib\gclib.h 
   cl $(COPT) $(@B).c

$(OBJ)\gx.obj : gx.c gclib\gclib.h fn_c.h fn_gnu.h 
   cl $(COPT) $(@B).c

$(OBJ)\gi.obj : gi.c gclib\gclib.h fn_c.h fn_gnu.h 
   cl $(COPT) $(@B).c

$(OBJ)\dis.obj : dis.c gclib\gclib.h gclib\opcodes.h gclib\binfile.h 
   cl $(COPT) $(@B).c

$(OBJ)\fn_c.obj : fn_c.c gclib\gclib.h 
   cl $(COPT) $(@B).c

$(OBJ)\fn_gnu.obj : fn_gnu.c gclib\gclib.h 
   cl $(COPT) $(@B).c

$(EXE)\gc.exe : $(OBJ)\gc.obj $(LIB2).lib
  link $(LOPT) $(OBJ)\$(@B),$(EXE)\$(@B),NUL,$(LIBS);

$(EXE)\gx.exe : $(OBJ)\gx.obj $(OBJ)\fn_c.obj $(OBJ)\fn_gnu.obj  $(LIB2).lib
  link $(LOPT) $(OBJ)\$(@B) $(OBJ)\fn_c $(OBJ)\fn_gnu,$(EXE)\$(@B),NUL,$(LIBS);

$(EXE)\gi.exe : $(OBJ)\gi.obj $(OBJ)\fn_c.obj $(LIB2).lib
  link $(LOPT) $(OBJ)\$(@B) $(OBJ)\fn_c $(OBJ)\fn_gnu,$(EXE)\$(@B),NUL,$(LIBS);

$(EXE)\dis.exe : $(OBJ)\dis.obj $(LIB2).lib
  link $(LOPT) $(OBJ)\$(@B),$(EXE)\$(@B),NUL,$(LIBS);

