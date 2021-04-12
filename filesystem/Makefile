##
## Copyright (c) 2004-2012
## ASPEED Technology Inc. All Rights Reserved
## Proprietary and Confidential
##
## By using this code you acknowledge that you have signed and accepted
## the terms of the ASPEED SDK license agreement.
##
include $(AST_SDK_ROOT)/ast.cfg


default:
	make -f $(FS_MAKEFILE) default

# Pass unknown make rules to sub-Makefile. The sub-Makefile decides how to deal with it.
%:
	make -f $(FS_MAKEFILE) $@