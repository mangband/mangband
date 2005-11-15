# Microsoft Developer Studio Project File - Name="mangclient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=mangclient - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mangclient.mak".
!MESSAGE 
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

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mangclient - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USE_GRAPHICS" /D "__MSVC__" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib /nologo /version:0.7 /subsystem:windows /pdb:none /machine:I386 /out:"../../mangclient.exe"

!ELSEIF  "$(CFG)" == "mangclient - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "__MSVC__" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../../mangclient.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "mangclient - Win32 Release"
# Name "mangclient - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "client"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\client\c-birth.c"
# End Source File
# Begin Source File

SOURCE="..\client\c-cmd.c"
# End Source File
# Begin Source File

SOURCE="..\client\c-files.c"
# End Source File
# Begin Source File

SOURCE="..\client\c-init.c"
# End Source File
# Begin Source File

SOURCE="..\client\c-inven.c"
# End Source File
# Begin Source File

SOURCE="..\client\c-spell.c"
# End Source File
# Begin Source File

SOURCE="..\client\c-store.c"
# End Source File
# Begin Source File

SOURCE="..\client\c-tables.c"
# End Source File
# Begin Source File

SOURCE="..\client\c-util.c"
# End Source File
# Begin Source File

SOURCE="..\client\c-xtra1.c"
# End Source File
# Begin Source File

SOURCE="..\client\c-xtra2.c"
# End Source File
# Begin Source File

SOURCE="..\client\client.c"
# End Source File
# Begin Source File

SOURCE="..\client\main-win.c"
# End Source File
# Begin Source File

SOURCE="..\client\netclient.c"
# End Source File
# Begin Source File

SOURCE="..\client\Readdib.c"
# End Source File
# Begin Source File

SOURCE="..\client\variable.c"
# End Source File
# Begin Source File

SOURCE="..\client\z-term.c"
# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\common\net-win.c"
# End Source File
# Begin Source File

SOURCE="..\common\sockbuf.c"
# End Source File
# Begin Source File

SOURCE="..\common\z-form.c"
# End Source File
# Begin Source File

SOURCE="..\common\z-rand.c"
# End Source File
# Begin Source File

SOURCE="..\common\z-util.c"
# End Source File
# Begin Source File

SOURCE="..\common\z-virt.c"
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "client-h"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\client\angband.h"
# End Source File
# Begin Source File

SOURCE="..\client\externs.h"
# End Source File
# Begin Source File

SOURCE="..\client\netclient.h"
# End Source File
# Begin Source File

SOURCE="..\client\Readdib.h"
# End Source File
# Begin Source File

SOURCE="..\client\resource.h"
# End Source File
# Begin Source File

SOURCE="..\client\z-term.h"
# End Source File
# End Group
# Begin Group "common-h"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\common\angband.h"
# End Source File
# Begin Source File

SOURCE="..\common\bit.h"
# End Source File
# Begin Source File

SOURCE="..\common\const.h"
# End Source File
# Begin Source File

SOURCE="..\common\defines.h"
# End Source File
# Begin Source File

SOURCE="..\common\externs.h"
# End Source File
# Begin Source File

SOURCE="..\common\h-basic.h"
# End Source File
# Begin Source File

SOURCE="..\common\h-define.h"
# End Source File
# Begin Source File

SOURCE="..\common\h-net.h"
# End Source File
# Begin Source File

SOURCE="..\common\h-system.h"
# End Source File
# Begin Source File

SOURCE="..\common\h-type.h"
# End Source File
# Begin Source File

SOURCE="..\common\net-win.h"
# End Source File
# Begin Source File

SOURCE="..\common\pack.h"
# End Source File
# Begin Source File

SOURCE="..\common\sockbuf.h"
# End Source File
# Begin Source File

SOURCE="..\common\types.h"
# End Source File
# Begin Source File

SOURCE="..\common\version.h"
# End Source File
# Begin Source File

SOURCE="..\common\z-form.h"
# End Source File
# Begin Source File

SOURCE="..\common\z-rand.h"
# End Source File
# Begin Source File

SOURCE="..\common\z-term.h"
# End Source File
# Begin Source File

SOURCE="..\common\z-util.h"
# End Source File
# Begin Source File

SOURCE="..\common\z-virt.h"
# End Source File
# End Group
# Begin Source File

SOURCE="..\config.h"
# End Source File
# Begin Source File

SOURCE="..\h-config.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Angband.ico
# End Source File
# Begin Source File

SOURCE=.\Angband.rc
# End Source File
# End Group
# End Target
# End Project
