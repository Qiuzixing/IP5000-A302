include $(AST_SDK_ROOT)/ast.cfg


app_dir = $(subst ",,$(AST_APP))

default: 	
	@for d in $(app_dir);   \
	do                              \
		$(MAKE) default -C $$d || exit 1;       \
	done 

clean: 
	@for d in $(app_dir);		\
	do					\
	  $(MAKE) --directory=$$d clean || exit 1;	\
	done


