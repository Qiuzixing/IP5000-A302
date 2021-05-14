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

BUILD_VERSION=$(UBOOT_VER)

APP_FOLDER_NAME=u-boot-$(BUILD_VERSION)
MAKE_ROOT=$(shell pwd)

default:
	@echo "u-boot $(BUILD_VERSION) Build ..."
	@[ -n "$(UBOOT_IMAGE)" ] || { echo "ERROR!!! ast.cfg UBOOT_IMAGE required"; exit 1; }
	@rm -f $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/$(UBOOT_IMAGE)
	@rm -rf $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/u-boot_bin
	@mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/u-boot_bin
	@mkdir -p $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)
	make ln_unpacked -C $(AST_SDK_ROOT)/gpl_src GPL_DIR=bootloader BUILD_VERSION=$(BUILD_VERSION) APP_FOLDER_NAME=$(APP_FOLDER_NAME)
	@[ -h $(APP_FOLDER_NAME)-unpacked ] || { echo "$(APP_FOLDER_NAME)-unpacked not exists?!"; exit 1; }
	( \
		make clean -C ./$(APP_FOLDER_NAME)-unpacked && make distclean -C ./$(APP_FOLDER_NAME)-unpacked && make $(UBOOT_CONFIG) -C ./$(APP_FOLDER_NAME)-unpacked &&  make -C ./$(APP_FOLDER_NAME)-unpacked > /dev/null || exit 1; \
		#make env -C ./$(APP_FOLDER_NAME)-unpacked || exit 1; \
		cp -f ./$(APP_FOLDER_NAME)-unpacked/u-boot.bin $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)/$(UBOOT_IMAGE); \
		#cp -f ./$(APP_FOLDER_NAME)-unpacked/tools/env/fw_printenv $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/u-boot_bin/; \
		#cp -f ./$(APP_FOLDER_NAME)-unpacked/tools/env/fw_setenv $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/u-boot_bin/; \
		cp -f ./$(APP_FOLDER_NAME)-unpacked/tools/mkimage $(AST_SDK_ROOT)/x86_bin/; \
		#cp -f $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)/$(UBOOT_IMAGE) /tftpboot/; \
	) || exit 1;

clean:
	@echo "u-boot $(BUILD_VERSION) Clean ..."
	@rm -f $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)/$(UBOOT_IMAGE);
	@rm -rf $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/u-boot_bin;
	@if [ -h $(APP_FOLDER_NAME)-unpacked ]; then \
		make clean -C ./$(APP_FOLDER_NAME)-unpacked && make distclean -C ./$(APP_FOLDER_NAME)-unpacked || exit 1; \
	fi

######################################################
default_old:
	@echo "u-boot $(BUILD_VERSION) Build ..."
	@[ -n "$(UBOOT_IMAGE)" ] || { echo "ERROR!!! ast.cfg UBOOT_IMAGE required"; exit 1; }
	@rm -f $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/$(UBOOT_IMAGE)
	@rm -rf $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/u-boot_bin
	@mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/u-boot_bin
	@mkdir -p $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)
	@[ -d unpacked_src ] || make unpack
	( \
		make clean -C ./unpacked_src && make distclean -C ./unpacked_src && make $(UBOOT_CONFIG) -C ./unpacked_src &&  make -C ./unpacked_src > /dev/null || exit 1; \
		#make env -C ./unpacked_src || exit 1; \
		cp -f ./unpacked_src/u-boot.bin $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)/$(UBOOT_IMAGE); \
		#cp -f ./unpacked_src/tools/env/fw_printenv $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/u-boot_bin/; \
		#cp -f ./unpacked_src/tools/env/fw_setenv $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/u-boot_bin/; \
		cp -f ./unpacked_src/tools/mkimage $(AST_SDK_ROOT)/x86_bin/; \
		#cp -f $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)/$(UBOOT_IMAGE) /tftpboot/; \
	) || exit 1;

unpack_old:
	@echo "Unpack $(APP_FOLDER_NAME)..."
	@( \
		rm -rf unpacked_src 2>/dev/null; \
		cd $(BUILD_VERSION) || exit 1; \
		tar jxf *.tar.bz2 -C ./ || exit 1; \
		cp -af patch/. ./$(APP_FOLDER_NAME)/; \
		mv -f ./$(APP_FOLDER_NAME) $(MAKE_ROOT)/unpacked_src || exit 1; \
		echo "$(APP_FOLDER_NAME) unpacked to $(MAKE_ROOT)/unpacked_src"; \
	) || exit 1;

clean_unpacked_old:
	@echo "Deleting $(MAKE_ROOT)/unpacked_src ..."
	@rm -rf unpacked_src 2>/dev/null

