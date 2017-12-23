AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libarmstrong.la

armstrongheadersdir = $(includedir)/zzub
armstrongheaders_HEADERS = ../../include/zzub/zzub.h ../../include/zzub/types.h

#nobase_include_HEADERS = zzub/zzub.h zzub/types.h

libarmstrong_la_SOURCES = armstrong_driver.cpp armstrong_info.cpp armstrong_io.cpp armstrong_pattern.cpp armstrong_player.cpp armstrong_plugin.cpp armstrong_wave.cpp bmxreader.cpp decompress.cpp modimport.cpp waveimport.cpp

libarmstrong_la_SOURCES += ../minizip/zip.c ../minizip/unzip.c ../minizip/ioapi.c

libarmstrong_la_SOURCES += ../modfile/itsex.c ../modfile/module.cpp ../modfile/module_it.cpp ../modfile/module_mod.cpp ../modfile/module_s3m.cpp ../modfile/module_xm.cpp

libarmstrong_la_CPPFLAGS = -DPOSIX -DUSE_SNDFILE -I../../include -I.. -I../player -fpermissive

libarmstrong_la_LIBADD = ../player/libplayer.la ../mixing/libmixing.la ../storage/libstorage.la ../plugins/core/libcore.la ../plugins/midi/libmidi.la ../plugins/ld_fungus/libfungus.la ../plugins/lunar/liblunar.la -lsqlite3 -lsndfile -lz

if PORTMIDI
libarmstrong_la_LIBADD += -lportmidi
endif

libarmstrong_la_LDFLAGS = -no-undefined

