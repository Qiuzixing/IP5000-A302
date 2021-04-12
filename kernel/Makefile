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

BUILD_VERSION=$(KERNEL_VERSION)

APP_FOLDER_NAME=linux-$(BUILD_VERSION)
MAKE_ROOT=$(shell pwd)

default:
	@echo "$(KERNEL) Build ..."
	@mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel
	@rm -f $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)/$(KERNEL_IMAGE)
	@[ -n "$(KERNEL_IMAGE)" ] || { echo "ERROR!!! ast.cfg KERNEL_IMAGE required"; exit 1; }
	make ln_unpacked -C $(AST_SDK_ROOT)/gpl_src GPL_DIR=kernel BUILD_VERSION=$(BUILD_VERSION) APP_FOLDER_NAME=$(APP_FOLDER_NAME)
#	@make clean -C ./$(APP_FOLDER_NAME)-unpacked
#	make distclean -C ./$(APP_FOLDER_NAME)-unpacked
	@make $(KERNEL_CONFIG) O=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel -C ./$(APP_FOLDER_NAME)-unpacked && \
	make oldconfig O=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel -C ./$(APP_FOLDER_NAME)-unpacked && \
	make mrproper -C ./$(APP_FOLDER_NAME)-unpacked && \
	make build

build:
	@# 'build' make script will not touch kernel .config file. 'default' make script will.
	@[ -n "$(KERNEL_IMAGE)" ] || { echo "ERROR!!! ast.cfg KERNEL_IMAGE required"; exit 1; }
	@rm -f $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)/$(KERNEL_IMAGE)
	make $(KERNEL_IMAGE) -C $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel
	@mv $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel/arch/arm/boot/$(KERNEL_IMAGE) $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)/
	@cp $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)/$(KERNEL_IMAGE) /tftpboot/

clean:
	@echo "$(KERNEL) Clean ..."
	@rm -f $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)/$(KERNEL_IMAGE);
	@rm -rf $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel;
	@if [ -h $(APP_FOLDER_NAME)-unpacked ]; then \
		make clean -C ./$(APP_FOLDER_NAME)-unpacked && make distclean -C ./$(APP_FOLDER_NAME)-unpacked || exit 1; \
	fi

%:
	@# ToDo. Assumes using the same toolchain. Validate it.
	@# ToDo. Assumes all $'@' build the same gpl apps (unpacked_src).
	@# I don't use the default handler in ast.cfg because I need make 'build' instead of build 'default'
	( \
		echo "Validate config $@"; \
		__tmp="$(subst -, ,$@)"; \
		set -- $${__tmp}; \
		__target="$$1"; \
		__config="$$2"; \
		[ -n "$${__target}" ] || { echo "Unexpected target value"; exit 1; }; \
		[ -n "$${__config}" ] || __config=default; \
		[ -f "$(AST_SDK_ROOT)/configs/$${__target}/$${__target}-$${__config}.cfg" ] || { echo "Unknown config"; exit 1; }; \
		make USE_CFG=$${__target}-$${__config} build; \
	) || exit 1


######################################################
default_old:
	@[ -d unpacked_src ] || make unpack
	@echo "$(KERNEL) Build ..."
	@mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel
	@rm -f $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)/$(KERNEL_IMAGE)
	@[ -n "$(KERNEL_IMAGE)" ] || { echo "ERROR!!! ast.cfg KERNEL_IMAGE required"; exit 1; }
#	@make clean -C ./unpacked_src
#	make distclean -C ./unpacked_src
	@make $(KERNEL_CONFIG) O=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel -C ./unpacked_src && \
	make oldconfig O=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel -C ./unpacked_src && \
	make mrproper -C ./unpacked_src && \
	make build

unpack_old:
	@echo "Unpack $(APP_FOLDER_NAME)..."
	@( \
		rm -rf unpacked_src 2>/dev/null; \
		cd $(BUILD_VERSION) || exit 1; \
		tar jxf *.tar.bz2 -C ./ || exit 1; \
		cp -af patch/. ./$(APP_FOLDER_NAME)/; \
		mv -f ./$(APP_FOLDER_NAME) $(MAKE_ROOT)/unpacked_src || exit 1;\
		echo "$(APP_FOLDER_NAME) unpacked to $(MAKE_ROOT)/unpacked_src"; \
	) || exit 1;


clean_old:
	@echo "$(KERNEL) Clean ..."
	@rm -rf $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel;
	@rm -f $(AST_SDK_ROOT)/images/$(TARGET_FOLDER_NAME)/$(KERNEL_IMAGE);
	@if [ -d unpacked_src ]; then \
		make clean -C ./unpacked_src && make distclean -C ./unpacked_src || exit 1; \
	fi

clean_unpacked_old:
	@echo "Deleting $(MAKE_ROOT)/unpacked_src ..."
	@rm -rf unpacked_src 2>/dev/null

