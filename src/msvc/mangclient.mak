# Microsoft Developer Studio Generated NMAKE File, Based on mangclient.dsp
!IF "$(CFG)" == ""
CFG=mangclient - Win32 Debug
!MESSAGE No configuration specified. Defaulting to mangclient - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "mangclient - Win32 Release" && "$(CFG)" != "mangclient - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mangclient.mak" CFG="mangclient - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mangclient - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "mangclient - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "mangclient - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\mangclient.exe"


CLEAN :
	-@erase "$(INTDIR)\Angband.res"
	-@erase "$(INTDIR)\c-birth.obj"
	-@erase "$(INTDIR)\c-cmd.obj"
	-@erase "$(INTDIR)\c-files.obj"
	-@erase "$(INTDIR)\c-init.obj"
	-@erase "$(INTDIR)\c-inven.obj"
	-@erase "$(INTDIR)\c-spell.obj"
	-@erase "$(INTDIR)\c-store.obj"
	-@erase "$(INTDIR)\c-tables.obj"
	-@erase "$(INTDIR)\c-util.obj"
	-@erase "$(INTDIR)\c-xtra1.obj"
	-@erase "$(INTDIR)\c-xtra2.obj"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\main-win.obj"
	-@erase "$(INTDIR)\net-win.obj"
	-@erase "$(INTDIR)\netclient.obj"
	-@erase "$(INTDIR)\Readdib.obj"
	-@erase "$(INTDIR)\sockbuf.obj"
	-@erase "$(INTDIR)\variable.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\z-form.obj"
	-@erase "$(INTDIR)\z-rand.obj"
	-@erase "$(INTDIR)\z-term.obj"
	-@erase "$(INTDIR)\z-util.obj"
	-@erase "$(INTDIR)\z-virt.obj"
	-@erase "..\..\mangclient.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USE_GRAPHICS" /D "__MSVC__" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\Angband.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mangclient.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib /nologo /version:0.7 /subsystem:windows /pdb:none /machine:I386 /out:"../../mangclient.exe" 
LINK32_OBJS= \
	"$(INTDIR)\c-birth.obj" \
	"$(INTDIR)\c-cmd.obj" \
	"$(INTDIR)\c-files.obj" \
	"$(INTDIR)\c-init.obj" \
	"$(INTDIR)\c-inven.obj" \
	"$(INTDIR)\c-spell.obj" \
	"$(INTDIR)\c-store.obj" \
	"$(INTDIR)\c-tables.obj" \
	"$(INTDIR)\c-util.obj" \
	"$(INTDIR)\c-xtra1.obj" \
	"$(INTDIR)\c-xtra2.obj" \
	"$(INTDIR)\client.obj" \
	"$(INTDIR)\main-win.obj" \
	"$(INTDIR)\netclient.obj" \
	"$(INTDIR)\Readdib.obj" \
	"$(INTDIR)\variable.obj" \
	"$(INTDIR)\z-term.obj" \
	"$(INTDIR)\net-win.obj" \
	"$(INTDIR)\sockbuf.obj" \
	"$(INTDIR)\z-form.obj" \
	"$(INTDIR)\z-rand.obj" \
	"$(INTDIR)\z-util.obj" \
	"$(INTDIR)\z-virt.obj" \
	"$(INTDIR)\Angband.res"

"..\..\mangclient.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\mangclient.exe" "$(OUTDIR)\mangclient.bsc"


CLEAN :
	-@erase "$(INTDIR)\Angband.res"
	-@erase "$(INTDIR)\c-birth.obj"
	-@erase "$(INTDIR)\c-birth.sbr"
	-@erase "$(INTDIR)\c-cmd.obj"
	-@erase "$(INTDIR)\c-cmd.sbr"
	-@erase "$(INTDIR)\c-files.obj"
	-@erase "$(INTDIR)\c-files.sbr"
	-@erase "$(INTDIR)\c-init.obj"
	-@erase "$(INTDIR)\c-init.sbr"
	-@erase "$(INTDIR)\c-inven.obj"
	-@erase "$(INTDIR)\c-inven.sbr"
	-@erase "$(INTDIR)\c-spell.obj"
	-@erase "$(INTDIR)\c-spell.sbr"
	-@erase "$(INTDIR)\c-store.obj"
	-@erase "$(INTDIR)\c-store.sbr"
	-@erase "$(INTDIR)\c-tables.obj"
	-@erase "$(INTDIR)\c-tables.sbr"
	-@erase "$(INTDIR)\c-util.obj"
	-@erase "$(INTDIR)\c-util.sbr"
	-@erase "$(INTDIR)\c-xtra1.obj"
	-@erase "$(INTDIR)\c-xtra1.sbr"
	-@erase "$(INTDIR)\c-xtra2.obj"
	-@erase "$(INTDIR)\c-xtra2.sbr"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.sbr"
	-@erase "$(INTDIR)\main-win.obj"
	-@erase "$(INTDIR)\main-win.sbr"
	-@erase "$(INTDIR)\net-win.obj"
	-@erase "$(INTDIR)\net-win.sbr"
	-@erase "$(INTDIR)\netclient.obj"
	-@erase "$(INTDIR)\netclient.sbr"
	-@erase "$(INTDIR)\Readdib.obj"
	-@erase "$(INTDIR)\Readdib.sbr"
	-@erase "$(INTDIR)\sockbuf.obj"
	-@erase "$(INTDIR)\sockbuf.sbr"
	-@erase "$(INTDIR)\variable.obj"
	-@erase "$(INTDIR)\variable.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\z-form.obj"
	-@erase "$(INTDIR)\z-form.sbr"
	-@erase "$(INTDIR)\z-rand.obj"
	-@erase "$(INTDIR)\z-rand.sbr"
	-@erase "$(INTDIR)\z-term.obj"
	-@erase "$(INTDIR)\z-term.sbr"
	-@erase "$(INTDIR)\z-util.obj"
	-@erase "$(INTDIR)\z-util.sbr"
	-@erase "$(INTDIR)\z-virt.obj"
	-@erase "$(INTDIR)\z-virt.sbr"
	-@erase "$(OUTDIR)\mangclient.bsc"
	-@erase "$(OUTDIR)\mangclient.pdb"
	-@erase "..\..\mangclient.exe"
	-@erase "..\..\mangclient.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "__MSVC__" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\mangclient.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\Angband.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mangclient.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\c-birth.sbr" \
	"$(INTDIR)\c-cmd.sbr" \
	"$(INTDIR)\c-files.sbr" \
	"$(INTDIR)\c-init.sbr" \
	"$(INTDIR)\c-inven.sbr" \
	"$(INTDIR)\c-spell.sbr" \
	"$(INTDIR)\c-store.sbr" \
	"$(INTDIR)\c-tables.sbr" \
	"$(INTDIR)\c-util.sbr" \
	"$(INTDIR)\c-xtra1.sbr" \
	"$(INTDIR)\c-xtra2.sbr" \
	"$(INTDIR)\client.sbr" \
	"$(INTDIR)\main-win.sbr" \
	"$(INTDIR)\netclient.sbr" \
	"$(INTDIR)\Readdib.sbr" \
	"$(INTDIR)\variable.sbr" \
	"$(INTDIR)\z-term.sbr" \
	"$(INTDIR)\net-win.sbr" \
	"$(INTDIR)\sockbuf.sbr" \
	"$(INTDIR)\z-form.sbr" \
	"$(INTDIR)\z-rand.sbr" \
	"$(INTDIR)\z-util.sbr" \
	"$(INTDIR)\z-virt.sbr"

"$(OUTDIR)\mangclient.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\mangclient.pdb" /debug /machine:I386 /out:"../../mangclient.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\c-birth.obj" \
	"$(INTDIR)\c-cmd.obj" \
	"$(INTDIR)\c-files.obj" \
	"$(INTDIR)\c-init.obj" \
	"$(INTDIR)\c-inven.obj" \
	"$(INTDIR)\c-spell.obj" \
	"$(INTDIR)\c-store.obj" \
	"$(INTDIR)\c-tables.obj" \
	"$(INTDIR)\c-util.obj" \
	"$(INTDIR)\c-xtra1.obj" \
	"$(INTDIR)\c-xtra2.obj" \
	"$(INTDIR)\client.obj" \
	"$(INTDIR)\main-win.obj" \
	"$(INTDIR)\netclient.obj" \
	"$(INTDIR)\Readdib.obj" \
	"$(INTDIR)\variable.obj" \
	"$(INTDIR)\z-term.obj" \
	"$(INTDIR)\net-win.obj" \
	"$(INTDIR)\sockbuf.obj" \
	"$(INTDIR)\z-form.obj" \
	"$(INTDIR)\z-rand.obj" \
	"$(INTDIR)\z-util.obj" \
	"$(INTDIR)\z-virt.obj" \
	"$(INTDIR)\Angband.res"

"..\..\mangclient.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("mangclient.dep")
!INCLUDE "mangclient.dep"
!ELSE 
!MESSAGE Warning: cannot find "mangclient.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "mangclient - Win32 Release" || "$(CFG)" == "mangclient - Win32 Debug"
SOURCE="..\client\c-birth.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\c-birth.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\c-birth.obj"	"$(INTDIR)\c-birth.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\c-cmd.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\c-cmd.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\c-cmd.obj"	"$(INTDIR)\c-cmd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\c-files.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\c-files.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\c-files.obj"	"$(INTDIR)\c-files.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\c-init.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\c-init.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\c-init.obj"	"$(INTDIR)\c-init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\c-inven.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\c-inven.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\c-inven.obj"	"$(INTDIR)\c-inven.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\c-spell.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\c-spell.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\c-spell.obj"	"$(INTDIR)\c-spell.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\c-store.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\c-store.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\c-store.obj"	"$(INTDIR)\c-store.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\c-tables.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\c-tables.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\c-tables.obj"	"$(INTDIR)\c-tables.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\c-util.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\c-util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\c-util.obj"	"$(INTDIR)\c-util.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\c-xtra1.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\c-xtra1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\c-xtra1.obj"	"$(INTDIR)\c-xtra1.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\c-xtra2.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\c-xtra2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\c-xtra2.obj"	"$(INTDIR)\c-xtra2.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\client.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\client.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\client.obj"	"$(INTDIR)\client.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\main-win.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\main-win.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\main-win.obj"	"$(INTDIR)\main-win.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\netclient.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\netclient.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\netclient.obj"	"$(INTDIR)\netclient.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\Readdib.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\Readdib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\Readdib.obj"	"$(INTDIR)\Readdib.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\variable.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\variable.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\variable.obj"	"$(INTDIR)\variable.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\client\z-term.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\z-term.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\z-term.obj"	"$(INTDIR)\z-term.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\common\net-win.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\net-win.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\net-win.obj"	"$(INTDIR)\net-win.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\common\sockbuf.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\sockbuf.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\sockbuf.obj"	"$(INTDIR)\sockbuf.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\common\z-form.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\z-form.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\z-form.obj"	"$(INTDIR)\z-form.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\common\z-rand.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\z-rand.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\z-rand.obj"	"$(INTDIR)\z-rand.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\common\z-util.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\z-util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\z-util.obj"	"$(INTDIR)\z-util.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\common\z-virt.c"

!IF  "$(CFG)" == "mangclient - Win32 Release"


"$(INTDIR)\z-virt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"


"$(INTDIR)\z-virt.obj"	"$(INTDIR)\z-virt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Angband.rc

"$(INTDIR)\Angband.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

