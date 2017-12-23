#pragma once

void load_binary(std::string filename, char** image, int* size);
void save_binary(std::string filename, const char* image, int size);
void zzub_audio_connection_set_amp(zzub_plugin_t* connplug, int amp, bool with_undo);

int zzub_audio_connection_get_amp(zzub_plugin_t* connplug);
zzub_plugin_t* zzub_plugin_group_get_input_plugin(zzub_plugin_group_t* layer);
zzub_plugin_t* zzub_plugin_group_get_output_plugin(zzub_plugin_group_t* layer);
zzub_plugin_group_t* zzub_player_create_group_with_io(zzub_player_t* player, zzub_plugin_group_t* currentlayer, float x, float y);
zzub_plugin_group_t* zzub_player_get_plugin_group_by_index(zzub_player_t* player, zzub_plugin_group_t* currentlayer, int* counter, int groupindex);
zzub_plugin_group_t* zzub_player_get_plugin_group_by_index(zzub_player_t* player, int groupindex);
void FindVisibleInputs(buze_document_t* document, zzub_plugin_t* hiddenplugin, std::vector<zzub_plugin_t*>& result);

nodetype GetNodeType(zzub_plugin_t* machine);
edgetype GetEdgeType(zzub_connection_type type);

float get_scaled_amp(zzub_connection_t* conn);
zzub_plugin_group_t* zzub_plugin_group_get_child_group(zzub_plugin_group_t* plugingroup, zzub_plugin_group_t* childgroup);
bool is_valid_parent(zzub_plugin_group_t* selectedgroup, zzub_plugin_group_t* parentgroup);
int zzub_plugin_get_note_group(zzub_plugin_t* plugin);

std::string GetOutputChannelName(zzub_plugin_t* plugin, int index);
std::string GetInputChannelName(zzub_plugin_t* plugin, int index);

