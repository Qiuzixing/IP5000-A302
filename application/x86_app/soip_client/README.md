Introduction
---
soip is an application which acts as the client mode of AST15XX serial over ip type 3.


Steps to Build soip for Windows:
---
1. Install Cygwin
2. Copy node_list source code to Cygwin shell environment.
3. If Makefile.Win exists, rename it to Makefile.
4. make clean and make
5. soip.exe will be compiled.

How to Run:
---
1. Note windows' firewall setting. Disable it to avoid problems.
2. Config Windows' IP setting to the same IP domain of AST15XX device.
  Default is autoip '169.254.xxx.xxx' with '255.255.0.0' netmask.
3. Run soip.exe by following descriptopn,

Usage:
  Client:  soip -c -d <HOST> -m [MGROUP] -i [MINTERFACE_IP]

    -d <HOST>          AST15XX host IP address
    -m [MGROUP]        multicast group when multicast is on
    -i [MINTERFACE_IP] specified which interface for binding of multicast receiver
                       be only needed in multiple network interfaces
For example:
	host: 169.254.10.133
	multicast group: 225.0.100.000
		soip.exe -c -d 169.254.10.133 -m 225.0.100.000

	If there are more than one NIC on your PC,
	and IP of the interface connect with AST15XX device is 169.254.10.200
		soip.exe -c -d 169.254.10.133 -m 225.0.100.000 -i 169.254.10.200
		




