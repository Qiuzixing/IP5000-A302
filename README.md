One minute SDK usage guide
===============================================================================

Install SDK
-------------------------------------------------------------------------------

+ Prepare a Linux system with:  
	- genext2fs
	- sudo
	- toolchain from who send you this SDK

+ Get the SDK distribution package. Ex: `ast_sdk_130117.tar.bz2`

+ Un-tar `ast_sdk_130117.tar.bz2` to a temp folder like `/tmp`.

		$ tar jxvf ast_sdk_130117.tar.bz2

+ Files will be un-tared to `/tmp/sdk`. Now, select a folder and install `TO`. Ex:

		$ cd /tmp/sdk
		$ make distinstall TO=~/sdk

+ SDK will be installed to the folder specified. Ex: `~/sdk`


Update SDK
-----------------------------------------------------------

+ Use the same procedure as "Install SDK".

+ Note! All 'unpacked_src' folders will be deleted after update. Backup! and Backup!!


Compile Firmware
-------------------------------------------------------------------------------

+ Go to SDK folder. Ex: `~/sdk`

		$ cd ~/sdk

+ Choose which platform to build. Ex: To build ast1510 host platform:

		$ make ast1510-h_cfg

+ Source environment. Note! Do it only once per login session.

		$ source setenv.sh

+ Build

		$ make

+ Most of built object/binary files will be placed in `~/sdk/build/` folder.

+ The final built image will be placed in `~/sdk/images/` folder.


Advanced Compile
-------------------------------------------------------------------------------
+ Most of target/config dependent compile options are defined in `sdk_root/ast.cfg`.  
  ast.cfg is dynamically created on doing `make target-config_cfg`.
  By default, `ast.cfg` will include `ast-current.cfg` which is a symbolic link to
  the current 'target-config'.

+ Build individully:

		$ make kernel			# To build kernel only
		$ make bootldr			# To build boot loader only
		$ make kmods			# To build kernel modules only
		$ make apps				# To build apps only
		$ make rootfs			# To build rootfs and misc files only
		$ make images			# To build images only

+ To build AST1510 host and client platform concurrently:

		$ make ast1510-c_cfg    # Do it once per login session
		$ source setenv.sh      # Do it once per login session
		$ make ast1510-h ast1510-c

+ You can also compile in most of source code subfolders. Ex:

		$ cd application/gpl_app/busybox
		$ make					# 'make ast1510-h ast1510-c' trick also works


Distribute SDK
-------------------------------------------------------------------------------
+ Make distpack with `CFG_LIST` and where to package `TO`. Ex:

		$ make distpack CFG_LIST="ast1510-h ast1510-c" TO=~/dist

+ Above example will package "ast1510-h" and "ast1510-c" platform to `~/dist/sdk`

+ `~/dist/sdk-yymmdd.tar.bz2` will be created.  

+ Hint: To pack full sdk to `~/dist`, do:

		$ make distpack_all TO=~/dist





