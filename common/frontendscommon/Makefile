MAKE_LOCATION := ../../../core/make_system/
include $(MAKE_LOCATION)Makefile.platform.mk
include $(MAKE_LOCATION)Makefile.general.mk

LIBRARY_NAME := libfrontendscommon.a
LIBRARY_ROOT := ../
LIBRARY_PATH := $(LIBRARY_ROOT)library/
LIBRARY_INCLUDE := ./include/ \
				   ../../../core/include \
				   ../


LIBRARY_SRC := $(LIBRARY_ROOT)src/

include $(MAKE_LOCATION)Makefile.cxxcompilervar.mk
include $(MAKE_LOCATION)Makefile.library.mk
include $(MAKE_LOCATION)Makefile.cxxcompiler.mk

