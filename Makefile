# global Makefile that is calling sub directories ones

gotoall: all

# Clean previous builds sequels
clean:
	rm -f *.so
	rm -f Modules/*/*.o

# Build everything
all:
	$(MAKE) -C Modules/mod_Lua
	$(MAKE) -C Modules/mod_every
	$(MAKE) -C Modules/mod_ups
	$(MAKE) -C Modules/mod_outfile
	$(MAKE) -C Modules/mod_OnOff
	$(MAKE) -C Modules/mod_dpd
	$(MAKE) -C Modules/mod_sht31
	$(MAKE) -C Modules/mod_1wire
	$(MAKE) -C Modules/mod_alert
	$(MAKE) -C Modules/mod_inotify
	$(MAKE) -C Modules/mod_OpenWeatherMap
	$(MAKE) -C Modules/mod_freeboxV5
	$(MAKE) -C Modules/mod_RFXtrx
	$(MAKE) -C Modules/mod_dummy
	$(MAKE) -C Modules/Marcel
