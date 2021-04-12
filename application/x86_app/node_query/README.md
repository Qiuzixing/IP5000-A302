Steps to Build node_query for Windows:
---
1.Install Cygwin
2.Copy node_query source code to Cygwin shell environment.
3.If Makefile.Win exists, rename it to Makefile.
4.make clean and make
5.node_query.exe will be compiled.

How to Run:
---
1.Note: Windows' firewall setting. Disable it to avoid problems.
2.Note: Use `--intf local_ip` option if your computer has multiple network interface.
3.Config Windows' IP setting to the same IP domain of AST15XX device.
  Default is autoip '169.254.xxx.xxx' with '255.255.0.0' netmask.
4.Run node_query.bat your_local_ip


2017.07.14. by Bruce