HNPS:=busybox.hnp \
	bash.hnp \
	coremark.hnp \
	openssl.hnp \
	make.hnp

all: copy

output/%.hnp:
	mkdir -p output
	make -C $(patsubst output/%.hnp,%,$@)
	cp $(patsubst output/%.hnp,%,$@)/$(patsubst output/%.hnp,%,$@).hnp $@

copy: $(patsubst %,output/%, $(HNPS))
	rm -f ../entry/hnp/arm64-v8a/*.hnp
	cp $^ ../entry/hnp/arm64-v8a