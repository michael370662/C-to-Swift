MAKE_LOCATION := ../../../core/make_system/
include $(MAKE_LOCATION)Makefile.platform.mk
include $(MAKE_LOCATION)Makefile.general.mk


LIBRARY_NAME := libfrontendsbridge.a
WITH_PLATFORM_SORUCE := yes

STRONSWAN_PATH := ../../../../strongswan-master/
include ../strongswan_flags.mk


LIBRARY_PATH := ../library/
LIBRARY_INCLUDE := ./ \
	../../../core/include/ \
	$(STRONSWAN_PATH)src/	\
	$(STRONSWAN_PATH)src/libstrongswan/ \
	$(STRONSWAN_PATH)src/libcharon/ \
	$(STRONSWAN_PATH)src/libipsec/

CCFLAGS += $(strongswan_CFLAGS)

include $(MAKE_LOCATION)Makefile.cccompilervar.mk
include $(MAKE_LOCATION)Makefile.library.mk
include $(MAKE_LOCATION)Makefile.cccompiler.mk
