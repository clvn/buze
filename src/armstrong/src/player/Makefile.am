AUTOMAKE_OPTIONS = foreign

noinst_LTLIBRARIES = libplayer.la

libplayer_la_SOURCES = driver.cpp driver_native.cpp dummy.cpp player.cpp pluginmanager.cpp

libplayer_la_SOURCES += audiodriver/audioapi.cpp audiodriver/audiodriver.cpp audiodriver/convertsample.cpp

if JACK
libplayer_la_SOURCES += audiodriver/jack/jackdriver.cpp
endif

if PORTMIDI
libplayer_la_SOURCES += mididriver/portmidi/portmididriver.cpp
endif
if JACK
libplayer_la_SOURCES += mididriver/jack/jackmididriver.cpp
endif

libplayer_la_CPPFLAGS = -DPOSIX -I../../include -I.. -fpermissive

if JACK
libplayer_la_LIBADD = -ljack
endif

