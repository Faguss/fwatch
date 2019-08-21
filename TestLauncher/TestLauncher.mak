# Microsoft Developer Studio Generated NMAKE File, Based on TestLauncher.dsp
!IF "$(CFG)" == ""
CFG=TestLauncher - Win32 Debug
!MESSAGE No configuration specified. Defaulting to TestLauncher - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "TestLauncher - Win32 Release" && "$(CFG)" != "TestLauncher - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TestLauncher.mak" CFG="TestLauncher - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TestLauncher - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TestLauncher - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "TestLauncher - Win32 Release"

OUTDIR=.\..
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\..\ 
# End Custom Macros

ALL : "$(OUTDIR)\fwatchCWA.exe"


CLEAN :
	-@erase "$(INTDIR)\script1.res"
	-@erase "$(INTDIR)\TestLauncher.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\fwatchCWA.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\TestLauncher.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\script1.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\TestLauncher.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\fwatchCWA.pdb" /machine:I386 /out:"$(OUTDIR)\fwatchCWA.exe" 
LINK32_OBJS= \
	"$(INTDIR)\TestLauncher.obj" \
	"$(INTDIR)\script1.res" \
	"..\fwatch.lib"

"$(OUTDIR)\fwatchCWA.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "TestLauncher - Win32 Debug"

OUTDIR=.\..
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\..\ 
# End Custom Macros

ALL : "$(OUTDIR)\TestLauncher.exe"


CLEAN :
	-@erase "$(INTDIR)\script1.res"
	-@erase "$(INTDIR)\TestLauncher.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\TestLauncher.exe"
	-@erase "$(OUTDIR)\TestLauncher.ilk"
	-@erase "$(OUTDIR)\TestLauncher.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\TestLauncher.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\script1.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\TestLauncher.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\TestLauncher.pdb" /debug /machine:I386 /out:"$(OUTDIR)\TestLauncher.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\TestLauncher.obj" \
	"$(INTDIR)\script1.res" \
	"..\fwatch.lib"

"$(OUTDIR)\TestLauncher.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("TestLauncher.dep")
!INCLUDE "TestLauncher.dep"
!ELSE 
!MESSAGE Warning: cannot find "TestLauncher.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "TestLauncher - Win32 Release" || "$(CFG)" == "TestLauncher - Win32 Debug"
SOURCE=.\script1.rc

"$(INTDIR)\script1.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\TestLauncher.cpp

"$(INTDIR)\TestLauncher.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

