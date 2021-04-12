Steps to Build node_list for Windows:
---
1.Install Cygwin
2.Copy node_list source code to Cygwin shell environment.
3.If Makefile.Win exists, rename it to Makefile.
4.make clean and make
5.node_list.exe will be compiled.

How to Run:
---
1.Note windows' firewall setting. Disable it to avoid problems.
2.Config Windows' IP setting to the same IP domain of AST15XX device.
  Default is autoip '169.254.xxx.xxx' with '255.255.0.0' netmask.
3.Run node_list.exe


2015.09.04. by Bruce