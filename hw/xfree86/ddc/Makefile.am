sdk_HEADERS = edid.h xf86DDC.h xf86i2c.h i2c_def.h

noinst_LTLIBRARIES = libddc.la

libddc_la_SOURCES = ddc.c interpret_edid.c print_edid.c ddcProperty.c xf86i2c.c

AM_CPPFLAGS = $(XORG_INCS)

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)

EXTRA_DIST = DDC.HOWTO
