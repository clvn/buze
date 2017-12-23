"""
Callback demonstration

Run once! Move the python window into a subscreen, then do simple operations. Loading a song will be too heavy, usually.
"""
import time
import armstrong
from ctypes import *
def foo_cb(player, plugin, event, x):
    print plugin, event, x # These are not fully qualified
    return 0
  
doc = mainframe.get_document()
player = doc.get_player()

cbref = armstrong.zzub_callback_t(foo_cb)
player.add_callback(cbref, 0)
print "Callback installed"
#player.remove_callback(cbref, 0) # How to remove it
