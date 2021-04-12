
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

default: build
	@echo "Done"

build:
	if [ "$(BUILD_TYPE)" = 'client' ]; then \
		mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/; \
		( \
			cd src; \
			./mk.sh || exit 1; \
			cp -f GUI $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/ || exit 1; \
			cp -f run_list.sh $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/ || exit 1; \
		) || exit 1; \
	fi

clean:
	( \
		cd src; \
		./mk_clean.sh || exit 1; \
	) || exit 1;

