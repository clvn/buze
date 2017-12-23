"""
Callback demonstration

Run once! Move the python window into a subscreen, then do simple operations. Loading a song will be too heavy, usually.
"""
import time
import buze
import armstrong
from ctypes import *

def foo_cb(view, hint, data, tag):
    print view, hint, data # These are not fully qualified
    return 0
  
doc = mainframe.get_document()

cbref = buze.buze_callback_t(foo_cb)
doc.add_callback(cbref, 0)
print "Callback installed"
#player.remove_callback(cbref, 0) # How to remove it
