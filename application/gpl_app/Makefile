include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)


app_dir = $(subst ",,$(GPL_APP))

default:
	@for d in $(app_dir);   \
	do \
		$(MAKE) -C $$d default || exit 1;\
	done


clean: 
	@for d in $(app_dir);		\
	do	\
	  $(MAKE) -C $$d clean || exit 1;	\
	done

unpack:
	@for d in $(app_dir);		\
	do	\
	  $(MAKE) -C $$d unpack || exit 1;	\
	done

clean_unpacked: 
	@for d in $(app_dir);		\
	do	\
	  $(MAKE) -C $$d clean_unpacked || exit 1;	\
	done

