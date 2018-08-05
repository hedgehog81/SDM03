<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: Tester - Win32 (WCE x86) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP50.tmp" with contents
[
/nologo /W3 /I "..\..\zlib-1.2.3" /D _WIN32_WCE=500 /D "WCE_PLATFORM_STANDARDSDK_500" /D "_i386_" /D UNDER_CE=500 /D "i_386_" /D "UNICODE" /D "_UNICODE" /D "_X86_" /D "x86" /D "NDEBUG" /Fp"X86Rel/Tester.pch" /YX /Fo"X86Rel/" /Gs8192 /GF /O2 /c 
"E:\Projects\CPP\sdm03_driver\Tester\main.cpp"
"E:\Projects\CPP\sdm03_driver\Tester\SDM03Reader.cpp"
]
Creating command line "cl.exe @C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP50.tmp" 
Creating temporary file "C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP51.tmp" with contents
[
commctrl.lib coredll.lib corelibc.lib  zlib.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"mainWCRTStartup" /incremental:no /pdb:"X86Rel/Tester.pdb" /nodefaultlib:"OLDNAMES.lib" /nodefaultlib:libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib /out:"X86Rel/Tester.exe" /libpath:"..\..\zlib-1.2.3\projects\evc4\zlib\X86Release_Static" /subsystem:windowsce,5.00 /MACHINE:IX86 
.\X86Rel\main.obj
.\X86Rel\SDM03Reader.obj
]
Creating command line "link.exe @C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP51.tmp"
<h3>Output Window</h3>
Compiling...
main.cpp
SDM03Reader.cpp
E:\Projects\CPP\sdm03_driver\Tester\SDM03Reader.cpp(38) : warning C4018: '<' : signed/unsigned mismatch
Linking...




<h3>Results</h3>
Tester.exe - 0 error(s), 1 warning(s)
</pre>
</body>
</html>
