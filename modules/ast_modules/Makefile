include $(AST_SDK_ROOT)/ast.cfg
#$(shell . ../../ast.cfg)

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

mod_dir = $(subst ",,$(AST_MODULES))

default: subdirs

subdirs:
	@for i in $(mod_dir); do echo "$$i Build..." ; make -C $$i default || exit 1; done

clean:
	@for i in $(mod_dir); do echo "$$i Clean..." ; make -C $$i clean >> /dev/null || exit 1; done

%:
	@for i in $(mod_dir); do echo "$$i Build..." ; make -C $$i $@ || exit 1; done
