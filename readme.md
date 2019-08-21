Fwatch 1.16 early release

Fwatch was created by Kegetys. New functionality added by Faguss (ofp-faguss.com)



Code to report listen server to the master server was taken from OFPReportListenServer by Pulverizer and was improved with the help of Poweruser

Uses natsort by Martin Pool



Compiled using Microsoft Visual C++ 6.0 on WinXP SP3



To open project:

   TestDLL \ TestDLL.dsw

   TestLauncher \ TestLauncher.dsw



To compile select:

   Build >> Set Active Configuration >> Win32 Release

and then press F7



==================================================================



Fwatch adds additional scripting functionality to Operation

Flashpoint Resistance by hooking file access calls from

OFP. New commands include functions to handle strings, file

access, and keyboard/mouse input. 



Fwatch v1.0d source code.

by Kegetys <kegetys@dnainternet.net>

Based on apihijack example code from CodeGuru.com by Matt Pietrek and Wade Brainerd. 



TestDLL directory includes the source for fwatch.dll which does all the interesting things.

TestLauncher directory contains source for fwatch.exe which installs the hook and runs OFP.



If you make modified versions of this program, change the script call syntax so that it uses a different menthod for detecting script commands to avoid conflicts with the "official" Fwatch version. 



You may use this source for personal entertainment purposes only. Any commercial-, education- or military use is strictly forbidden without permission from the author. Any programs compiled from this source and the source code itself should be made available free of charge. Any modified versions must have the source code available for free upon request.



