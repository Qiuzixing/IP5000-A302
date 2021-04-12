# ASPEED Techonolgy Inc.
# AST Chip SDK 
# Author :  ryan_chen@aspeedtech.com

AST_VERSION=1
ifdef AST_VERSION
AST_VERSION=$(shell cat .version)
else
AST_VERSION=$(shell date +"%Y%m%d")
endif

# Ex: $(AST_VERSION) == "0.53"
# AST_MAJOR_VERSION will == 0
# AST_SUB_VERSION will == 53
# AST_TAG_VERSION will == AST_SUB_VERSION + 1
ast_ver = $(subst ., ,$(AST_VERSION))
AST_MAJOR_VERSION=$(word 1,$(ast_ver))
AST_SUB_VERSION= $(word 2,$(ast_ver)) 
AST_TAG_VERSION= $(shell expr $(AST_SUB_VERSION) + 1)

export AST_VERSION

export AST_SDK_ROOT = $(shell pwd)

# Ex: ast2400-default_cfg
# tmp will == "ast2400 default"
# target will == "ast2400"
# config will == "default"
tmp = $(subst -, ,$(subst _cfg,,$@))
target = $(filter ast%, $(tmp))
config = $(word 2, $(tmp))

# Construct setenv.sh and ast.cfg
%_cfg:
	@rm -f ast.cfg 2>/dev/null
	@rm -f ast-current.cfg 2>/dev/null
	@rm -f setenv.sh 2>/dev/null
	@ln -s configs/ast.cfg ast.cfg
	@echo "Target: [$(target)]"
	@if [ -z $(config) ]; then \
		if [ -f configs/$(target)/$(target)-default.cfg ] ; then \
			ln -s configs/$(target)/$(target)-default.cfg ast-current.cfg; \
			rm -f ast-default.cfg; \
			ln -s configs/$(target)/$(target)-default.cfg ast-default.cfg; \
			echo "Configuration : [default] .. OK"; \
			ln -s x86_bin/env/`grep -r ENV= ast-current.cfg | sed 's/ENV=//g'`.sh setenv.sh; \
			make post_config; \
		else \
			echo "Config not found!!"; \
		fi \
	else \
		if [ -f configs/$(target)/$(target)-$(config).cfg ] ; then \
			ln -s configs/$(target)/$(target)-$(config).cfg ast-current.cfg; \
			echo "Configuration : [$(config)] .. OK"; \
			ln -s x86_bin/env/`grep -r ENV= ast-current.cfg | sed 's/ENV=//g'`.sh setenv.sh; \
			make post_config; \
		else \
			echo "Config not found!!"; \
		fi \
	fi
	@exit


ifneq ($(wildcard ast.cfg),)
include $(AST_SDK_ROOT)/ast.cfg
endif


TARGET_FOLDER_NAME = $(TARGET)-$(CONFIG)
BOOTLOADER_DIR = $(AST_SDK_ROOT)/bootloader
KERNEL_DIR = $(AST_SDK_ROOT)/kernel
THIRD_MODULES_DIR = $(AST_SDK_ROOT)/modules/3rd_modules
AST_MODULES_DIR = $(AST_SDK_ROOT)/modules/ast_modules
AST_APP_DIR = $(AST_SDK_ROOT)/application/ast_app
GPL_APP_DIR = $(AST_SDK_ROOT)/application/gpl_app
X86_APP_DIR = $(AST_SDK_ROOT)/application/x86_app
FS_DIR = $(AST_SDK_ROOT)/filesystem
IMAGES_DIR = $(AST_SDK_ROOT)/images
BUILD_DIR = $(AST_SDK_ROOT)/build

ARM_BUILD_DIRS = $(BOOTLOADER_DIR) $(KERNEL_DIR) $(AST_MODULES_DIR) $(THIRD_MODULES_DIR) $(GPL_APP_DIR) $(AST_APP_DIR) $(FS_DIR)
ARM_CLEAN_DIRS = $(BOOTLOADER_DIR) $(AST_MODULES_DIR) $(THIRD_MODULES_DIR) $(KERNEL_DIR) $(FS_DIR) $(GPL_APP_DIR) $(AST_APP_DIR)
ARM_PACKED_DIRS = $(BOOTLOADER_DIR) $(KERNEL_DIR) $(GPL_APP_DIR)

ARM_MODULES_DIRS = $(AST_MODULES_DIR) $(THIRD_MODULES_DIR)
ARM_APPS_DIRS = $(GPL_APP_DIR) $(AST_APP_DIR)

M68K_BUILD_DIRS = $(BOOTLOADER_DIR) uClinux
M68K_CLEAN_DIRS = $(BOOTLOADER_DIR) uClinux
M68K_PACKED_DIRS = ""

ifeq ($(ARCH),arm)
BUILD_DIRS=$(ARM_BUILD_DIRS)
CLEAN_DIRS=$(ARM_CLEAN_DIRS)
PACKED_DIRS=$(ARM_PACKED_DIRS)
else
BUILD_DIRS=$(M68K_BUILD_DIRS)
CLEAN_DIRS=$(M68K_CLEAN_DIRS)
PACKED_DIRS=$(M68K_PACKED_DIRS)
endif

default: env_chk
	@([ -n "$(TARGET)" ] && echo "$(TARGET) SDK version v$(AST_VERSION) Build..." || (echo "Please Config Target" ; exit 1))
	@mkdir -p $(BUILD_DIR)/$(TARGET_FOLDER_NAME)
	@mkdir -p $(BUILD_DIR)/$(TARGET_FOLDER_NAME)/rootfs_addon
	@mkdir -p $(IMAGES_DIR)/$(TARGET_FOLDER_NAME)
	@for i in $(BUILD_DIRS); do echo "$$i Build...";make default -C $$i || exit 1;done

bootldr: env_chk
	@for i in $(BOOTLOADER_DIR); do echo "$$i Build...";make default -C $$i || exit 1;done

kernel: env_chk
	@for i in $(KERNEL_DIR); do echo "$$i Build...";make default -C $$i || exit 1;done

kmods: env_chk
	@for i in $(ARM_MODULES_DIRS); do echo "$$i Build...";make default -C $$i || exit 1;done

apps: env_chk
	@for i in $(ARM_APPS_DIRS); do echo "$$i Build...";make default -C $$i || exit 1;done

rootfs: env_chk
	@for i in $(FS_DIR); do echo "$$i Build...";make rootfs -C $$i || exit 1;done

image: env_chk
	@for i in $(FS_DIR); do echo "$$i Build...";make default -C $$i || exit 1;done

clean:
	@([ -n "$(TARGET)" ] && echo "$(TARGET) SDK version v$(AST_VERSION) Clean..." || (echo "Please Config Target" ; exit 1))
	@for i in $(CLEAN_DIRS); do echo "$$i Clean...";make clean -C $$i || exit 1;done
	@rm -rf $(BUILD_DIR)/$(TARGET_FOLDER_NAME)
	@rm -rf $(IMAGES_DIR)/$(TARGET_FOLDER_NAME)

clean_unpacked:
	make -C gpl_src/ clean_unpacked_all

%:
	@# ToDo. Assumes using the same toolchain. Validate it.
	@# ToDo. Assumes all $'@' build the same gpl apps (unpacked_src).
	@( \
		echo "Validate config $@"; \
		__tmp="$(subst -, ,$@)"; \
		set -- $${__tmp}; \
		__target="$$1"; \
		__config="$$2"; \
		[ -n "$${__target}" ] || { echo "Unexpected target value"; exit 1; }; \
		[ -n "$${__config}" ] || __config=default; \
		[ -f "$(AST_SDK_ROOT)/configs/$${__target}/$${__target}-$${__config}.cfg" ] || { echo "Unknown config"; exit 1; }; \
		make USE_CFG=$${__target}-$${__config} default; \
	) || exit 1

distclean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(IMAGES_DIR)
	@rm -f *.cfg
	@rm -f setenv.sh

post_config:
	@#([ -n "$(BUILD_TYPE)" ] && (rm -f ast-$(BUILD_TYPE).cfg; ln -s configs/$(TARGET)/$(TARGET)-$(CONFIG).cfg ast-$(BUILD_TYPE).cfg;)) || (exit 0)

tag:
	@echo $(AST_TAG_VERSION) > .version

#unpack:
#	@([ -n "$(TARGET)" ] && echo "$(TARGET) SDK version v$(AST_VERSION) Clean..." || (echo "Please Config Target" ; exit 1))
#	@for i in $(PACKED_DIRS); do echo "$$i Create unpacked_src directories...";make unpack -C $$i || exit 1;done

	
help:
	@echo ""
	@echo "************* AST SDK version v$(AST_VERSION) *******************"
	@echo "Step 1 : Setup TARGET :            make ast1510-h_cfg"
	@echo "Step 2 : Setup build environment : source setenv.sh"
	@echo "Step 3 : Start build sdk :         make"
	@echo "******************************************************"
	@echo ""


#####################################################################################################################
################################ Following make script is for packaging SDK #########################################
#####################################################################################################################
#####################################################################################################################
#####################################################################################################################
#####################################################################################################################
#####################################################################################################################
#####################################################################################################################


distpack_basic:
	##########################################################################
	# Copy basic files
	mkdir -p $(TO)/sdk/application/ast_app
	mkdir -p $(TO)/sdk/application/gpl_app
	mkdir -p $(TO)/sdk/application/profile_app
	mkdir -p $(TO)/sdk/application/slt_app
	mkdir -p $(TO)/sdk/application/x86_app
	mkdir -p $(TO)/sdk/bootloader
	mkdir -p $(TO)/sdk/configs
	mkdir -p $(TO)/sdk/filesystem
	mkdir -p $(TO)/sdk/kernel
	mkdir -p $(TO)/sdk/modules/ast_modules
	mkdir -p $(TO)/sdk/modules/3rd_modules
	mkdir -p $(TO)/sdk/gpl_src/application/gpl_app
	mkdir -p $(TO)/sdk/gpl_src/kernel
	mkdir -p $(TO)/sdk/gpl_src/bootloader
	cp -fdp $(AST_SDK_ROOT)/gpl_src/* $(TO)/sdk/gpl_src || exit 0
	cp -fdp $(AST_SDK_ROOT)/* $(TO)/sdk || exit 0
	cp -fdp $(AST_SDK_ROOT)/.* $(TO)/sdk || exit 0
	rm $(TO)/sdk/*.cfg
	rm $(TO)/sdk/setenv.sh
	cp -af $(AST_SDK_ROOT)/x86_bin $(TO)/sdk/
	cp -fdp $(AST_SDK_ROOT)/configs/* $(TO)/sdk/configs/ || exit 0
	cp -fdp $(AST_SDK_ROOT)/configs/.* $(TO)/sdk/configs/ || exit 0
	tar c -C $(AST_SDK_ROOT)/application/x86_app/ . | tar x -C $(TO)/sdk/application/x86_app/

distpack_cfg:
	echo "$(USE_CFG)"
	##########################################################################
	# Copy $(USE_CFG) config files
	mkdir -p $(TO)/sdk/configs/$(TARGET)
	cp -f $(AST_SDK_ROOT)/configs/$(TARGET)/$(TARGET)-$(CONFIG).cfg $(TO)/sdk/configs/$(TARGET)/
	##########################################################################
	# Copy bootloader files
	@cp -fdp $(AST_SDK_ROOT)/bootloader/* $(TO)/sdk/bootloader/ 2>/dev/null || exit 0
	@cp -fdp $(AST_SDK_ROOT)/bootloader/.* $(TO)/sdk/bootloader/ 2>/dev/null  || exit 0
	#make cp_buildver -C $(AST_SDK_ROOT)/bootloader/ TO=$(TO)/sdk/bootloader
	make cp_buildver_bootldr -C $(AST_SDK_ROOT)/gpl_src/ TO=$(TO)/sdk
	##########################################################################
	# Copy kernel files
	@cp -fdp $(AST_SDK_ROOT)/kernel/* $(TO)/sdk/kernel/ 2>/dev/null || exit 0
	@cp -fdp $(AST_SDK_ROOT)/kernel/.* $(TO)/sdk/kernel/ 2>/dev/null  || exit 0
	#make cp_buildver -C $(AST_SDK_ROOT)/kernel/ TO=$(TO)/sdk/kernel
	make cp_buildver_kernel -C $(AST_SDK_ROOT)/gpl_src/ TO=$(TO)/sdk
	##########################################################################
	# Copy ast_modules files
	@cp -fdp $(AST_SDK_ROOT)/modules/ast_modules/* $(TO)/sdk/modules/ast_modules/ 2>/dev/null || exit 0
	@cp -fdp $(AST_SDK_ROOT)/modules/ast_modules/.* $(TO)/sdk/modules/ast_modules/ 2>/dev/null  || exit 0
	@for i in $(subst ",,$(AST_MODULES)); do echo "$$i Copy..." ; cp -afu $(AST_SDK_ROOT)/modules/ast_modules/$$i $(TO)/sdk/modules/ast_modules/ || exit 1; done
	##########################################################################
	# Copy 3rd_modules files
	@cp -fdp $(AST_SDK_ROOT)/modules/3rd_modules/* $(TO)/sdk/modules/3rd_modules/ 2>/dev/null || exit 0
	@cp -fdp $(AST_SDK_ROOT)/modules/3rd_modules/.* $(TO)/sdk/modules/3rd_modules/ 2>/dev/null  || exit 0
	@for i in $(subst ",,$(THIRD_MODULES)); do echo "$$i Copy..." ; cp -afu $(AST_SDK_ROOT)/modules/3rd_modules/$$i $(TO)/sdk/modules/3rd_modules/ || exit 1; done
	##########################################################################
	# Copy gpl_app files
	@cp -fdp $(AST_SDK_ROOT)/application/gpl_app/* $(TO)/sdk/application/gpl_app/ 2>/dev/null || exit 0
	@cp -fdp $(AST_SDK_ROOT)/application/gpl_app/.* $(TO)/sdk/application/gpl_app/ 2>/dev/null  || exit 0
	@for i in $(GPL_APP); do \
		echo "##########################################################################"; \
		echo "$$i Copy..."; \
		mkdir -p $(TO)/sdk/application/gpl_app/$$i; \
		cp -fdp $(AST_SDK_ROOT)/application/gpl_app/$$i/* $(TO)/sdk/application/gpl_app/$$i/ 2>/dev/null; \
		cp -fdp $(AST_SDK_ROOT)/application/gpl_app/$$i/.* $(TO)/sdk/application/gpl_app/$$i/ 2>/dev/null; \
		#make cp_buildver -C $(AST_SDK_ROOT)/application/gpl_app/$$i TO=$(TO)/sdk/application/gpl_app/$$i; \
		# following echo is important because above 'cp -fdp' returns failed \
		echo "Done"; \
	done
	make cp_buildver_apps -C $(AST_SDK_ROOT)/gpl_src/ TO=$(TO)/sdk
	##########################################################################
	# Copy ast_app files
	@cp -fdp $(AST_SDK_ROOT)/application/ast_app/* $(TO)/sdk/application/ast_app/ 2>/dev/null || exit 0
	@cp -fdp $(AST_SDK_ROOT)/application/ast_app/.* $(TO)/sdk/application/ast_app/ 2>/dev/null  || exit 0
	@for i in $(subst ",,$(AST_APP)); do \
		echo "##########################################################################"; \
		echo "$$i Copy..."; \
		tar c -C $(AST_SDK_ROOT)/application/ast_app/ $$i/ | tar x -C $(TO)/sdk/application/ast_app/ ;\
	done
	##########################################################################
	# Copy filesystem files
	@cp -fdp $(AST_SDK_ROOT)/filesystem/* $(TO)/sdk/filesystem/ 2>/dev/null || exit 0
	@cp -fdp $(AST_SDK_ROOT)/filesystem/.* $(TO)/sdk/filesystem/ 2>/dev/null  || exit 0
	@mkdir -p $(TO)/sdk/filesystem/$(TARGET)/$(TARGET)-$(CONFIG)
	@tar c -C $(AST_SDK_ROOT)/filesystem/ $(FS_TEMPLATE)/ | tar x -C $(TO)/sdk/filesystem/
	@tar c -C $(AST_SDK_ROOT)/filesystem/ $(TARGET)/$(TARGET)-$(CONFIG)/ | tar x -C $(TO)/sdk/filesystem/


export _TO=$(abspath $(TO))
distpack: env_chk
	@[ -n "$(CFG_LIST)" ] || { echo "Need CFG_LIST. Ex: CFG_LIST=\"ast1510-h ast1510-c\""; exit 1;}
	@[ -n "$(TO)" ] || { echo "Need TO. Where to put distribution package."; exit 1;}
	@[ -n "$(_TO)" ] || { echo "The Makefile could be broken due to $abspath function"; exit 1;}
	@echo "Create distribution for '$(CFG_LIST)' and store the package to $(TO) [$(abspath $(TO))]"
	##########################################################################
	# Create package folder
	@cd $(_TO) || { echo "cd error!!"; exit 1; }
	rm -rf $(_TO)/sdk
	mkdir -p $(_TO)/sdk

	##########################################################################
	# Clean SDK folder before copy
	@for i in $(CFG_LIST); do \
		make clean TO=$(_TO) USE_CFG=$$i || exit 1; \
	done
	## Create patch and clean unpacked source code
	@for i in $(CFG_LIST); do \
		make pack -C gpl_src USE_CFG=$$i || exit 1; \
	done
	
	##########################################################################
	# Start copy...
	@make distpack_basic CFG_LIST="$(CFG_LIST)" TO=$(_TO)
	@for i in $(CFG_LIST); do \
		echo "Create config ($$i) directories..."; \
		make distpack_cfg TO=$(_TO) USE_CFG=$$i || exit 1; \
	done
	##########################################################################
	# Some clean up...
	rm -f `find $(_TO)/sdk/ -name '*-unpacked' -type l`
	##########################################################################
	# Tar sdk
	tar jcvf $(_TO)/sdk-$(shell date +%y%m%d).tar.bz2 -C $(_TO)/ sdk/
	##########################################################################
	# $(_TO)/sdk-$(shell date +%y%m%d).tar.bz2 Done
	##########################################################################

distpack_all: env_chk
	@[ -n "$(TO)" ] || { echo "Need TO. Where to put distribution package."; exit 1;}
	@[ -n "$(_TO)" ] || { echo "The Makefile could be broken due to $abspath function"; exit 1;}
	# ToDo. following make clen will depends on ast-current.cfg which is not actually 'all'.
	#make clean && make clean_unpacked && make distclean 
	make clean && make distclean 
	#rm -rf `find $(AST_SDK_ROOT) -name 'unpacked_src' -type d`
	##########################################################################
	# Create package folder
	@cd $(_TO) || { echo "cd error!!"; exit 1; }
	rm -rf $(_TO)/sdk
	mkdir -p $(_TO)/sdk

	##########################################################################
	# Start copy...
	@tar c -C $(AST_SDK_ROOT) . --exclude-vcs | tar vx -C $(TO)/sdk/

	##########################################################################
	# Some clean up...
	rm -f `find $(_TO)/sdk/ -name '*-unpacked' -type l`
	rm -rf `find $(_TO)/sdk/gpl_src/ -name '*-ast-tgz' -type d`
	rm -rf `find $(_TO)/sdk/gpl_src/ -name '*-ast-src' -type d`

	##########################################################################
	# Tar sdk
	tar jcvf $(_TO)/sdk-$(shell date +%y%m%d).tar.bz2 -C $(_TO)/ sdk/
	##########################################################################
	# $(_TO)/sdk-$(shell date +%y%m%d).tar.bz2 Done
	##########################################################################


distinstall:
	@[ -n "$(TO)" ] || { echo "Need 'TO'. Where to install distribution package."; exit 1;}
	@[ -n "$(_TO)" ] || { echo "The Makefile could be broken due to $abspath function"; exit 1;}
	if ! [ -d "$(_TO)" ]; then \
		mkdir -p $(_TO); \
		make distinstall TO=$(TO); \
	else \
		tar c -C $(shell pwd)/ . | tar vx -C $(_TO)/; \
		rm -f `find $(_TO)/ -name '*-unpacked' -type l`; \
		rm -rf `find $(_TO)/gpl_src/ -name '*-ast-tgz' -type d`; \
		#rm -rf `find $(_TO)/gpl_src/ -name '*-ast-src' -type d`; \
	fi

