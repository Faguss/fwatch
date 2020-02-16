---------------------------------------------------------
                  ·:  Fwatch v1.0d  :·
---------------------------------------------------------

Fwatch adds additional scripting functionality to Operation
Flashpoint Resistance by hooking file access calls from
OFP. New commands include functions to handle strings, file
access, and keyboard/mouse input. 

To install Fwatch, extract this archive to your Operation
Flashpoint Resistance v1.96 final install directory. Then 
run OFP using fwatch.exe, any command line parameters for
fwatch.exe will be passed on to FlashpointResistance.exe.
You must always run OFP using fwatch.exe for fwatch to
work. You should use the '-nomap' commandline parameter 
with fwatch as with it Fwatch can use a much  faster
method for sending scripts for OFP.

See fwatch/doc.html for scripting documentation.
All files made by fwatch file access functions will be 
located at fwatch/mdb directory.

To use Fwatch on a Windows dedicated server, run fwatch.exe
with -nolaunch commandline parameter, and then run the
dedicated server normally. Linux dedicated server is not 
supported by this version.

Note that Fwatch searches for the following executable names:
flashpointresistance.exe (launched by running fwatch.exe)
flashpointbeta.exe
operationflashpoint.exe
ofpr_server.exe

If your client or server executable is named something 
different the Fwatch functions will not work.

---------------------------------------------------------
  credits
---------------------------------------------------------

Fwatch is made by Kegetys <kegetys@dnainternet.net> 
http://koti.mbnet.fi/kegetys/flashpoint

Fwatch includes code or parts from: 
- APIHijack DLL by Matt Pietrek/Wade Brainerd/Wei Junping
- GNU wget, http://www.gnu.org/software/wget/wget.html

---------------------------------------------------------
  license & disclaimer
---------------------------------------------------------

You are permitted to install and use this software for 
personal entertainment purposes only. Any commercial,
military or educational use is strictly forbidden without
permission from the author. 

You are free to distribute this software as you wish, as 
long as it is kept 100% free of charge, it is not modified 
in any way and this readme file is distributed with it.

The author takes no responsibility for any damages this 
program may cause, use at your own risk.

wget.exe is licensed under the GNU GPL, see 
http://www.gnu.org/software/wget/wget.html for details

---------------------------------------------------------