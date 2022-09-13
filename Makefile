# global Makefile that is calling sub directories ones

gotoall: all

clean:
	rm *.so

# Build everything
all:
	$(MAKE) -C src/mod_test
	$(MAKE) -C src
