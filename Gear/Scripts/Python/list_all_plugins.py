"""
List all plugins
"""
import armstrong

doc = mainframe.get_document()
player = doc.get_player()
loader_n = player.get_pluginloader_count()
print("%i plugins" % loader_n)
for n in range(loader_n):
    loader = player.get_pluginloader(n)
    collection_name = loader.get_plugincollection().get_name()
    print("%s: %-30s %s" % (
      collection_name,
      loader.get_name(), 
      loader.get_uri()
    ))
