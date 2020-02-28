# Fwatch 1.16 early release

Fwatch was created by [Kegetys](http://www.kegetys.fi/). New functionality added by [Faguss](https://ofp-faguss.com).

Code to report listen server to the master server was taken from OFPReportListenServer by Pulverizer and was improved with the help of [Poweruser](https://github.com/Poweruser).

Uses [natsort](https://github.com/sourcefrog/natsort) by Martin Pool.

Binary download: https://ofp-faguss.com/fwatch/116test

## Compiling

Project is compiled using Microsoft Visual C++ 6.0 on WinXP SP3

```
Open file TestDLL.dsw or TestLauncher.dsw
Build >> Set Active Configuration >> Win32 Release
Press F7
```

## Program Description

Fwatch adds additional scripting functionality to Operation Flashpoint Resistance by hooking file access calls from OFP. New commands include functions to handle strings, file access, and keyboard/mouse input. 

## Original Readme

Fwatch v1.0d source code.

by Kegetys <kegetys@dnainternet.net>m

Based on apihijack example code from CodeGuru.com by Matt Pietrek and Wade Brainerd. 



TestDLL directory includes the source for fwatch.dll which does all the interesting things.

TestLauncher directory contains source for fwatch.exe which installs the hook and runs OFP.



If you make modified versions of this program, change the script call syntax so that it uses a different menthod for detecting script commands to avoid conflicts with the "official" Fwatch version. 



You may use this source for personal entertainment purposes only. Any commercial-, education- or military use is strictly forbidden without permission from the author. Any programs compiled from this source and the source code itself should be made available free of charge. Any modified versions must have the source code available for free upon request.



