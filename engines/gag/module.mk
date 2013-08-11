MODULE := engines/gag

MODULE_OBJS := \
	detection.o \
	gag.o


# This module can be built as a plugin
ifeq ($(ENABLE_GAG), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
