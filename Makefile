# global Makefile that is calling sub directories ones

gotoall: all

# Clean previous builds sequels
clean:
	rm *.so

# Build everything
all:
	$(MAKE) -C src/mod_Lua
	$(MAKE) -C src/mod_every
	$(MAKE) -C src/mod_ups
	$(MAKE) -C src/mod_dummy
	$(MAKE) -C src
