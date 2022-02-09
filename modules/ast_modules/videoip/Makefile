
build_dir = driver app

default: subdirs

subdirs:
	@for i in $(build_dir); do echo "$$i Build..." ; make -C $$i default || exit 1; done

clean:
	@for i in $(build_dir); do echo "$$i Clean..." ; make -C $$i clean >> /dev/null || exit 1; done

%:
	@for i in $(build_dir); do echo "$$i Build..." ; make -C $$i $@ || exit 1; done
