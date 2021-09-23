##
## Copyright (c) 2004-2012
## ASPEED Technology Inc. All Rights Reserved
## Proprietary and Confidential
##
## By using this code you acknowledge that you have signed and accepted
## the terms of the ASPEED SDK license agreement.
##
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

BUILD_VERSION=2.9

APP_FOLDER_NAME=wpa_supplicant-$(BUILD_VERSION)
MAKE_ROOT=$(shell pwd)

default:
	@[ -n $(BUILD_VERSION) ] || { echo "Error!! BUILD_VERSION not defined!!"; exit 1; }
	make ln_unpacked -C $(AST_SDK_ROOT)/gpl_src GPL_DIR=application/gpl_app/wpa_supplicant BUILD_VERSION=$(BUILD_VERSION) APP_FOLDER_NAME=$(APP_FOLDER_NAME)
	@make -C $(APP_FOLDER_NAME)-unpacked -f ast.mk

clean:
	@[ -h $(APP_FOLDER_NAME)-unpacked ] && make -C $(APP_FOLDER_NAME)-unpacked -f ast.mk clean || echo "Nothing to clean"

######################################################################################
unpack_old:
	@echo "Unpack $(APP_FOLDER_NAME)..."
	@( \
		rm -rf unpacked_src 2>/dev/null; \
		cd $(BUILD_VERSION) || exit 1; \
		tar zxf *.tar.gz -C ./ || exit 1; \
		cp -af patch/. ./$(APP_FOLDER_NAME)/; \
		mv -f ./$(APP_FOLDER_NAME) $(MAKE_ROOT)/unpacked_src || exit 1; \
		echo "$(APP_FOLDER_NAME) unpacked to $(MAKE_ROOT)/unpacked_src"; \
	) || exit 1;


clean_unpacked_old:
	@echo "Deleting $(MAKE_ROOT)/unpacked_src ..."
	@rm -rf unpacked_src 2>/dev/null
