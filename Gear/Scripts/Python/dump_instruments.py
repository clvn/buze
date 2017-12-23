import armstrong

doc = mainframe.get_document()
player = doc.get_player()
plugit = player.get_plugin_iterator()
while plugit.valid():
    plugin = plugit.current()
    loader = plugin.get_pluginloader()
    print("id: %s, name: %s, type=%s, uri=%s" % (
      str(plugin.get_id()),
      plugin.get_name(),
      loader.get_name(), 
      loader.get_uri()
      ))
    dump = []
    for group_n in range(3):
      groups = []
      for track_n in range(plugin.get_track_count(group_n)):
        tracks = []
        for param_n in range(plugin.get_parameter_count(group_n, track_n)):
          value = plugin.get_parameter_value(group_n, track_n, param_n)
          tracks.append(value)
        groups.append(tracks)
      dump.append(groups)      
    print dump
    plugit.next()
