
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

default: build
	@echo "Done"

build:
	if [ "$(BUILD_TYPE)" = 'host' ]; then \
		mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/; \
		( \
			cd autoswitch; \
			qmake || exit 1; \
			make || exit 1; \
			cp -f mainswitch/mainswitch $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/ || exit 1; \
			cp -f sconfig/sconfig $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/ || exit 1; \
			cp -f audioswitch/audioswitch $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/ || exit 1; \
		) || exit 1; \
	fi

clean:
	( \
		cd autoswitch; \
		make distclean || exit 1; \
	) || exit 1;

