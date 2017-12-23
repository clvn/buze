#pragma once

// dsound utils
void ds_create_waveformat(int channels, int samplerate, int bitspersample, WAVEFORMATEX* waveformat);
bool ds_buffer_lock(LPDIRECTSOUNDBUFFER buffer, DWORD offset, DWORD bytes, LPVOID* buffer1, DWORD* buffersize1, LPVOID* buffer2, DWORD* buffersize2);
void ds_create_buffer_notifications(int count, int bytesmultiple, HANDLE event, std::vector<DSBPOSITIONNOTIFY>* positions);

// output
LPDIRECTSOUND ds_create_output_device(const std::string& name);
bool ds_set_output_device_format(LPDIRECTSOUND device, WAVEFORMATEX& waveFormat);
LPDIRECTSOUNDBUFFER ds_create_output_device_buffer(LPDIRECTSOUND device, WAVEFORMATEX& waveFormat, int bufferbytes);
bool ds_get_output_caps(LPDIRECTSOUND device, int* outputs, int* bitspersample, int* convertformat);

// input
LPDIRECTSOUNDCAPTURE ds_create_input_device(const std::string& indevice);
LPDIRECTSOUNDCAPTUREBUFFER ds_create_input_device_buffer(LPDIRECTSOUNDCAPTURE device, WAVEFORMATEX& waveFormat, int bufferbytes);
bool ds_get_input_channels(LPDIRECTSOUNDCAPTURE device, int* inputs);
