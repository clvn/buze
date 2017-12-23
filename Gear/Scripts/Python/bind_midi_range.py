import armstrong
import buze
import sys

CHANNEL = 2
MIDI_RANGE = range(60, 70)
words = ["cut", "res", "vol", "dec"]

if __name__ in ["__builtin__", "__main__"]:
  doc = mainframe.get_document()
  player = doc.get_player()
  plugin = player.get_midi_plugin()
  assert plugin, "Select the plugin you want to autobind"

  print "'%s' autobind" % plugin.get_name()
  for group_n in [1,2]:
    for track_n in range(plugin.get_track_count(group_n)):
      for param_n in range(plugin.get_parameter_count(group_n, track_n)):
        if MIDI_RANGE:
          parameter = plugin.get_parameter(group_n, track_n, param_n)
          name = parameter.get_name()
          if [True for word in words if name.lower().find(word) > -1]:
            cc = MIDI_RANGE.pop(0)
            player.add_midimapping(plugin, group_n, track_n, param_n, CHANNEL, cc)
            print "CC#%03i Chn:%02i :: %s" % (cc, CHANNEL, name)
  print


  

