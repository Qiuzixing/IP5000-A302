##
## Copyright (c) 2004-2012
## ASPEED Technology Inc. All Rights Reserved
## Proprietary and Confidential
##
## By using this code you acknowledge that you have signed and accepted
## the terms of the ASPEED SDK license agreement.
##

## Note:
##   - Assume source code is make cleaned.
##
##

include $(AST_SDK_ROOT)/ast.cfg
include $(AST_SDK_ROOT)/gpl_src/version_list.cfg

MAKE_ROOT=$(shell pwd)

default:
	@echo "make pack: create all GPL source code patches and save them to gpl_src/"
	@echo "make unpack: create all 'patched' GPL source code based on patch files in gpl_src/"

unpack:
	make unpack_bootldr
	make unpack_kernel
	make unpack_apps

unpack_all:
	for d in $(UBOOT_VER_LIST);   \
	do \
		make unpack_patch GPL_DIR=bootloader BUILD_VERSION=$$d APP_FOLDER_NAME=u-boot-$$d || exit 1; \
	done
	for d in $(KERNEL_VER_LIST);   \
	do \
		make unpack_patch GPL_DIR=kernel BUILD_VERSION=$$d APP_FOLDER_NAME=linux-$$d || exit 1; \
	done
	for d in $(GPL_APP_LIST_ALL);   \
	do \
		# Construct the BUILD_VER_LIST upper-case string. For example: avahi -> AVAHI_VER_LIST. \
		build_ver_list_str=`echo $$d | tr '[a-z]' '[A-Z]'`_VER_LIST; \
		make unpack_patch_all_app GPL_DIR=application/gpl_app/$$d  BUILD_VERSION_LIST_STR=$$build_ver_list_str APP_NAME=$$d || exit 1; \
	done

pack:
	make pack_bootldr
	make pack_kernel
	make pack_apps

pack_all:
	for d in $(UBOOT_VER_LIST);   \
	do \
		make pack_patch GPL_DIR=bootloader BUILD_VERSION=$$d APP_FOLDER_NAME=u-boot-$$d || exit 1; \
	done
	for d in $(KERNEL_VER_LIST);   \
	do \
		make pack_patch GPL_DIR=kernel BUILD_VERSION=$$d APP_FOLDER_NAME=linux-$$d || exit 1; \
	done
	for d in $(GPL_APP_LIST_ALL);   \
	do \
		# Construct the BUILD_VER_LIST upper-case string. For example: avahi -> AVAHI_VER_LIST. \
		build_ver_list_str=`echo $$d | tr '[a-z]' '[A-Z]'`_VER_LIST; \
		make pack_patch_all_app GPL_DIR=application/gpl_app/$$d  BUILD_VERSION_LIST_STR=$$build_ver_list_str APP_NAME=$$d || exit 1; \
	done

unpack_apps:
	for d in $(GPL_APP);   \
	do \
		# Construct the BUILD_VER upper-case string. For example: avahi -> AVAHI_VER. \
		build_ver_str=`echo $$d | tr '[a-z]' '[A-Z]'`_VER; \
		make unpack_patch_app GPL_DIR=application/gpl_app/$$d BUILD_VERSION_STR=$$build_ver_str APP_NAME=$$d || exit 1; \
	done

unpack_kernel:
	# Warning! Will delete SDK_ROOT/kernel/linux-xxxx source code
	@make unpack_patch GPL_DIR=kernel BUILD_VERSION=$(KERNEL_VERSION) APP_FOLDER_NAME=linux-$(KERNEL_VERSION)

unpack_bootldr:
	# Warning! Will delete SDK_ROOT/bootloader/u-boot-xxxx source code
	@make unpack_patch GPL_DIR=bootloader BUILD_VERSION=$(UBOOT_VER) APP_FOLDER_NAME=u-boot-$(UBOOT_VER)

pack_bootldr:
	@make pack_patch GPL_DIR=bootloader BUILD_VERSION=$(UBOOT_VER) APP_FOLDER_NAME=u-boot-$(UBOOT_VER)

pack_kernel:
	@make pack_patch GPL_DIR=kernel BUILD_VERSION=$(KERNEL_VERSION) APP_FOLDER_NAME=linux-$(KERNEL_VERSION)

pack_apps:
	for d in $(GPL_APP);   \
	do \
		# Construct the BUILD_VER upper-case string. For example: avahi -> AVAHI_VER. \
		build_ver_str=`echo $$d | tr '[a-z]' '[A-Z]'`_VER; \
		make pack_patch_app GPL_DIR=application/gpl_app/$$d BUILD_VERSION_STR=$$build_ver_str APP_NAME=$$d || exit 1; \
	done

cp_buildver_bootldr:
	make cp_buildver GPL_DIR=bootloader BUILD_VERSION=UBOOT_VER TO=$(TO)

cp_buildver_kernel:
	make cp_buildver GPL_DIR=kernel BUILD_VERSION=KERNEL_VERSION TO=$(TO)

cp_buildver_apps:
	for d in $(GPL_APP);   \
	do \
		# Construct the BUILD_VER upper-case string. For example: avahi -> AVAHI_VER. \
		build_ver_str=`echo $$d | tr '[a-z]' '[A-Z]'`_VER; \
		make cp_buildver GPL_DIR=application/gpl_app/$$d BUILD_VERSION=$$build_ver_str TO=$(TO) || exit 1; \
	done

clean_unpacked_all:
	for d in $(UBOOT_VER_LIST);   \
	do \
		make clean_unpacked GPL_DIR=bootloader BUILD_VERSION=$$d APP_FOLDER_NAME=u-boot-$$d || exit 1; \
	done
	for d in $(KERNEL_VER_LIST);   \
	do \
		make clean_unpacked GPL_DIR=kernel BUILD_VERSION=$$d APP_FOLDER_NAME=linux-$$d || exit 1; \
	done
	for d in $(GPL_APP_LIST_ALL);   \
	do \
		# Construct the BUILD_VER_LIST upper-case string. For example: avahi -> AVAHI_VER_LIST. \
		build_ver_list_str=`echo $$d | tr '[a-z]' '[A-Z]'`_VER_LIST; \
		make clean_unpacked_all_app GPL_DIR=application/gpl_app/$$d BUILD_VERSION_LIST_STR=$$build_ver_list_str APP_NAME=$$d || exit 1; \
	done

###########################################################################################################
cp_buildver:
	@[ -n "$(TO)" ] || { echo "Error!! TO not defined!!"; exit 1; }
	@[ -n "$(GPL_DIR)" ] || { echo "Error!! GPL_DIR not defined!!"; exit 1; }
	@[ -n "$(BUILD_VERSION)" ] || { echo "Error!! BUILD_VERSION not defined!!"; exit 1; }
	mkdir -p $(TO)/gpl_src/$(GPL_DIR)/$($(BUILD_VERSION))
	cp -afu $(MAKE_ROOT)/$(GPL_DIR)/$($(BUILD_VERSION))/patch $(TO)/gpl_src/$(GPL_DIR)/$($(BUILD_VERSION))/
	cp -afu $(MAKE_ROOT)/$(GPL_DIR)/$($(BUILD_VERSION))/*.tar.* $(TO)/gpl_src/$(GPL_DIR)/$($(BUILD_VERSION))/
	@#tar c $(MAKE_ROOT)/$(GPL_DIR)/$($(BUILD_VERSION))/ | tar x -C $(TO)/gpl_src/$(GPL_DIR)/ #This method is much slower because of file overwritten


create_patch:
	@echo "Creating patch for $(APP_FOLDER_NAME)..."
	@[ -n "$(APP_FOLDER_NAME)" ] || { echo "Error!! APP_FOLDER_NAME not defined!!"; exit 1; }
	@[ -n "$(BUILD_VERSION)" ] || { echo "Error!! BUILD_VERSION not defined!!"; exit 1; }
	@[ -n "$(GPL_ROOT)" ] || { echo "Error!! GPL_ROOT not defined!!"; exit 1; }
	@if [ -f $(GPL_ROOT)/$(BUILD_VERSION)/*.tar.gz ]; then \
		tar zxf $(GPL_ROOT)/$(BUILD_VERSION)/*.tar.gz -C $(GPL_ROOT)/$(BUILD_VERSION); \
	elif [ -f $(GPL_ROOT)/$(BUILD_VERSION)/*.tar.bz2 ]; then \
		tar jxf $(GPL_ROOT)/$(BUILD_VERSION)/*.tar.bz2 -C $(GPL_ROOT)/$(BUILD_VERSION); \
	else \
		echo "GPL source ($(GPL_ROOT)/$(APP_FOLDER_NAME).tar.X) tarball not exists?!"; \
		exit 1; \
	fi
	if [ -d $(MAKE_ROOT)/$(GPL_DIR)/$(BUILD_VERSION)/$(APP_FOLDER_NAME)-ast-src ]; then \
		( \
			cd $(GPL_ROOT)/$(BUILD_VERSION); \
			create_patch.sh $(APP_FOLDER_NAME) $(APP_FOLDER_NAME)-ast-src patch 1 || exit 1; \
		) || exit 1; \
	elif [ -d $(MAKE_ROOT)/$(GPL_DIR)/$(BUILD_VERSION)/$(APP_FOLDER_NAME)-ast-tgz ]; then \
		( \
			cd $(GPL_ROOT)/$(BUILD_VERSION); \
			create_patch.sh $(APP_FOLDER_NAME) $(APP_FOLDER_NAME)-ast-tgz patch 1 || exit 1; \
		) || exit 1; \
	else \
		echo "Modified files doesn't exist?! Assume no changed."; \
	fi
	rm -rf $(GPL_ROOT)/$(BUILD_VERSION)/$(APP_FOLDER_NAME)

pack_patch:
	# input example:
	#  GPL_DIR=bootloader
	#  BUILD_VERSION=$(UBOOT_VER)
	#  APP_FOLDER_NAME=u-boot-$(UBOOT_VER)
	@[ -n "$(GPL_DIR)" ] || { echo "Error!! GPL_DIR not defined!!"; exit 1; }
	@[ -n "$(BUILD_VERSION)" ] || { echo "Error!! BUILD_VERSION not defined!!"; exit 1; }
	@[ -n "$(APP_FOLDER_NAME)" ] || { echo "Error!! APP_FOLDER_NAME not defined!!"; exit 1; }
	make create_patch GPL_ROOT=$(MAKE_ROOT)/$(GPL_DIR) BUILD_VERSION=$(BUILD_VERSION) APP_FOLDER_NAME=$(APP_FOLDER_NAME)

pack_patch_app:
	# This is a make wrap to convert BUILD_VERSION_STR to BUILD_VERSION in ast.cfg
	# input example:
	#  GPL_DIR=applicaton/gpl_app/avahi
	#  BUILD_VERSION_STR=AVAHI_VER
	#  APP_NAME=avahi
	@[ -n "$(GPL_DIR)" ] || { echo "Error!! GPL_DIR not defined!!"; exit 1; }
	@[ -n "$(BUILD_VERSION_STR)" ] || { echo "Error!! BUILD_VERSION_STR not defined!!"; exit 1; }
	@[ -n "$(APP_NAME)" ] || { echo "Error!! APP_NAME not defined!!"; exit 1; }
	make pack_patch GPL_DIR=$(GPL_DIR) BUILD_VERSION=$($(BUILD_VERSION_STR)) APP_FOLDER_NAME=$(APP_NAME)-$($(BUILD_VERSION_STR))

pack_patch_all_app:
	for v in $($(BUILD_VERSION_LIST_STR));   \
	do \
		# Construct the BUILD_VER upper-case string. For example: avahi -> AVAHI_VER. \
		#build_ver_str=`echo $$v | tr '[a-z]' '[A-Z]'`_VER; \
		make pack_patch GPL_DIR=$(GPL_DIR) BUILD_VERSION=$$v APP_FOLDER_NAME=$(APP_NAME)-$$v || exit 1; \
	done
	
clean_unpacked:
	# input example:
	#  GPL_DIR=bootloader
	#  BUILD_VERSION=$(UBOOT_VER)
	#  APP_FOLDER_NAME=u-boot-$(UBOOT_VER)
	@[ -n "$(GPL_DIR)" ] || { echo "Error!! GPL_DIR not defined!!"; exit 1; }
	@[ -n "$(BUILD_VERSION)" ] || { echo "Error!! BUILD_VERSION not defined!!"; exit 1; }
	@[ -n "$(APP_FOLDER_NAME)" ] || { echo "Error!! APP_FOLDER_NAME not defined!!"; exit 1; }
	@echo "Deleting $(AST_SDK_ROOT)/$(GPL_DIR)/$(APP_FOLDER_NAME) ..."
	# Make sure we have patch folder exists. Otherwise delete source code may have no way back.
	if [ -d $(AST_SDK_ROOT)/gpl_src/$(GPL_DIR)/$(BUILD_VERSION)/patch ]; then \
		# Test whether the symbolic link file exists; \
		#if [ -h $(AST_SDK_ROOT)/$(GPL_DIR)/$(APP_FOLDER_NAME)-unpacked ]; then \
		#	rm -f $(AST_SDK_ROOT)/$(GPL_DIR)/$(APP_FOLDER_NAME)-unpacked; \
		#fi; \
		if [ -d $(AST_SDK_ROOT)/gpl_src/$(GPL_DIR)/$(BUILD_VERSION)/$(APP_FOLDER_NAME)-ast-tgz ]; then \
			rm -rf $(AST_SDK_ROOT)/gpl_src/$(GPL_DIR)/$(BUILD_VERSION)/$(APP_FOLDER_NAME)-ast-tgz; \
		fi; \
	fi

clean_unpacked_all_app:
	for v in $($(BUILD_VERSION_LIST_STR));   \
	do \
		# Construct the BUILD_VER upper-case string. For example: avahi -> AVAHI_VER. \
		make clean_unpacked GPL_DIR=$(GPL_DIR) BUILD_VERSION=$$v APP_FOLDER_NAME=$(APP_NAME)-$$v || exit 1; \
	done

unpack_patch:
	@[ -n "$(GPL_DIR)" ] || { echo "Error!! GPL_DIR not defined!!"; exit 1; }
	@[ -n "$(BUILD_VERSION)" ] || { echo "Error!! BUILD_VERSION not defined!!"; exit 1; }
	@[ -n "$(APP_FOLDER_NAME)" ] || { echo "Error!! APP_FOLDER_NAME not defined!!"; exit 1; }
	if [ -d $(GPL_DIR)/$(BUILD_VERSION)/$(APP_FOLDER_NAME)-ast-src ]; then \
		echo "Do nothing"; \
	else \
		make _unpack_patch GPL_DIR=$(GPL_DIR) BUILD_VERSION=$(BUILD_VERSION) APP_FOLDER_NAME=$(APP_FOLDER_NAME); \
	fi

_unpack_patch:
	# input example:
	#  GPL_DIR=bootloader
	#  BUILD_VERSION=$(UBOOT_VER)
	#  APP_FOLDER_NAME=u-boot-$(UBOOT_VER)
	@[ -n "$(GPL_DIR)" ] || { echo "Error!! GPL_DIR not defined!!"; exit 1; }
	@[ -n "$(BUILD_VERSION)" ] || { echo "Error!! BUILD_VERSION not defined!!"; exit 1; }
	@[ -n "$(APP_FOLDER_NAME)" ] || { echo "Error!! APP_FOLDER_NAME not defined!!"; exit 1; }
	@echo "Unpack $(GPL_DIR)/$(BUILD_VERSION)/$(APP_FOLDER_NAME)..."
	if [ -f $(GPL_DIR)/$(BUILD_VERSION)/*.tar.gz ]; then \
		tar zxf $(GPL_DIR)/$(BUILD_VERSION)/*.tar.gz -C $(GPL_DIR)/$(BUILD_VERSION); \
	elif [ -f $(GPL_DIR)/$(BUILD_VERSION)/*.tar.bz2 ]; then \
		tar jxf $(GPL_DIR)/$(BUILD_VERSION)/*.tar.bz2 -C $(GPL_DIR)/$(BUILD_VERSION); \
	else \
		echo "GPL source ($(GPL_DIR)/$(APP_FOLDER_NAME).tar.X) tarball not exists?!"; \
		exit 1; \
	fi
	# Applying patch...
	tar c -C $(GPL_DIR)/$(BUILD_VERSION)/patch/ . | tar x -C $(GPL_DIR)/$(BUILD_VERSION)/$(APP_FOLDER_NAME)/
	# Delete old source code
	make clean_unpacked GPL_DIR=$(GPL_DIR) BUILD_VERSION=$(BUILD_VERSION) APP_FOLDER_NAME=$(APP_FOLDER_NAME)
	# Copy new source code
	mv -f $(GPL_DIR)/$(BUILD_VERSION)/$(APP_FOLDER_NAME) $(GPL_DIR)/$(BUILD_VERSION)/$(APP_FOLDER_NAME)-ast-tgz
	@echo "$(APP_FOLDER_NAME) unpacked to $(MAKE_ROOT)/$(GPL_DIR)/$(APP_FOLDER_NAME)-ast-tgz"

unpack_patch_app:
	# This is a make wrap to convert BUILD_VERSION_STR to BUILD_VERSION in ast.cfg
	# input example:
	#  GPL_DIR=applicaton/gpl_app/avahi
	#  BUILD_VERSION_STR=AVAHI_VER
	#  APP_NAME=avahi
	@[ -n "$(GPL_DIR)" ] || { echo "Error!! GPL_DIR not defined!!"; exit 1; }
	@[ -n "$(BUILD_VERSION_STR)" ] || { echo "Error!! BUILD_VERSION not defined!!"; exit 1; }
	@[ -n "$(APP_NAME)" ] || { echo "Error!! APP_NAME not defined!!"; exit 1; }
	make unpack_patch GPL_DIR=$(GPL_DIR) BUILD_VERSION=$($(BUILD_VERSION_STR)) APP_FOLDER_NAME=$(APP_NAME)-$($(BUILD_VERSION_STR))

unpack_patch_all_app:
	for v in $($(BUILD_VERSION_LIST_STR));   \
	do \
		# Construct the BUILD_VER upper-case string. For example: avahi -> AVAHI_VER. \
		make unpack_patch GPL_DIR=$(GPL_DIR) BUILD_VERSION=$$v APP_FOLDER_NAME=$(APP_NAME)-$$v || exit 1; \
	done

ln_unpacked:
	# input example:
	#  GPL_DIR=bootloader
	#  BUILD_VERSION=$(UBOOT_VER)
	#  APP_FOLDER_NAME=u-boot-$(UBOOT_VER)
	@[ -n "$(GPL_DIR)" ] || { echo "Error!! GPL_DIR not defined!!"; exit 1; }
	@[ -n "$(BUILD_VERSION)" ] || { echo "Error!! BUILD_VERSION not defined!!"; exit 1; }
	@[ -n "$(APP_FOLDER_NAME)" ] || { echo "Error!! APP_FOLDER_NAME not defined!!"; exit 1; }

	if [ -h $(AST_SDK_ROOT)/$(GPL_DIR)/$(APP_FOLDER_NAME)-unpacked ]; then rm -f $(AST_SDK_ROOT)/$(GPL_DIR)/$(APP_FOLDER_NAME)-unpacked; fi
	if [ -d $(GPL_DIR)/$(BUILD_VERSION)/$(APP_FOLDER_NAME)-ast-src ]; then \
		ln -s $(AST_SDK_ROOT)/gpl_src/$(GPL_DIR)/$(BUILD_VERSION)/$(APP_FOLDER_NAME)-ast-src $(AST_SDK_ROOT)/$(GPL_DIR)/$(APP_FOLDER_NAME)-unpacked; \
	elif [ -d $(GPL_DIR)/$(BUILD_VERSION)/$(APP_FOLDER_NAME)-ast-tgz ]; then \
		ln -s $(AST_SDK_ROOT)/gpl_src/$(GPL_DIR)/$(BUILD_VERSION)/$(APP_FOLDER_NAME)-ast-tgz $(AST_SDK_ROOT)/$(GPL_DIR)/$(APP_FOLDER_NAME)-unpacked; \
	else \
		make unpack_patch GPL_DIR=$(GPL_DIR) BUILD_VERSION=$(BUILD_VERSION) APP_FOLDER_NAME=$(APP_FOLDER_NAME) || exit 1; \
		make ln_unpacked GPL_DIR=$(GPL_DIR) BUILD_VERSION=$(BUILD_VERSION) APP_FOLDER_NAME=$(APP_FOLDER_NAME) || exit 1; \
	fi
	
	
	
	