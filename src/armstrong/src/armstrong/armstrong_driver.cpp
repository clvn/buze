#include <cstring>
#include "library.h"
#include "player.h"
#include "driver.h"
#include "audiodriver/audiodriver.h"

using namespace armstrong::frontend;

extern "C" {


/* class DeviceInfo */

int zzub_device_info_get_api(zzub_device_info_t* device) {
	return device->api_id;
}

const char* zzub_device_info_get_name(zzub_device_info_t* device) {
	static char name[256];
	strncpy(name, device->name.c_str(), 256);
	return name;
}

int zzub_device_info_get_supported_buffersizes(zzub_device_info_t* device, int* result, int maxsizes) {
	if (maxsizes > (int)device->buffersizes.size()) maxsizes = (int)device->buffersizes.size();
	std::copy(device->buffersizes.begin(), device->buffersizes.begin() + maxsizes, result);
	return (int)device->buffersizes.size();
}

int zzub_device_info_get_supported_samplerates(zzub_device_info_t* device, int* result, int maxrates) {
	if (maxrates > (int)device->samplerates.size()) maxrates = (int)device->samplerates.size();
	std::copy(device->samplerates.begin(), device->samplerates.begin() + maxrates, result);
	return (int)device->samplerates.size();
}

int zzub_device_info_get_supported_output_channels(zzub_device_info_t* device) {
	return device->out_channels;
}

int zzub_device_info_get_supported_input_channels(zzub_device_info_t* device) {
	return device->in_channels;
}

/* class DeviceInfoIterator */

void zzub_device_info_iterator_next(zzub_device_info_iterator_t* it) {
	it->i++;
}

int zzub_device_info_iterator_valid(zzub_device_info_iterator_t* it) {
	return it->i != it->items.end();
}

zzub_device_info_t* zzub_device_info_iterator_current(zzub_device_info_iterator_t* it) {
	return *it->i;
}

void zzub_device_info_iterator_reset(zzub_device_info_iterator_t* it) {
	it->i = it->items.begin();
}

void zzub_device_info_iterator_destroy(zzub_device_info_iterator_t* it) {
	delete it;
}


/***

	Audio driver APIs

***/

/** \brief Create an audio driver that uses the homemade native API. */
zzub_audiodriver_t* zzub_audiodriver_create_native(zzub_player_t* player) {
	audiodriver* driver = audiodriver::create_native();
	if (driver) driver->initialize(player);
	return driver;
}

/** \brief Create a silent, non-processing audio driver that has one device with the specified properties. */
zzub_audiodriver_t* zzub_audiodriver_create_silent(zzub_player_t* player, const char* name, int out_channels, int in_channels, int* supported_rates, int num_rates) {
	std::vector<unsigned int> rates;
	rates.assign(supported_rates, supported_rates + num_rates);

	audiodriver* driver = audiodriver::create_silent(name, out_channels, in_channels, rates);
	if (driver) driver->initialize(player);
	return driver;
}

/** \brief Creates the preferred audio driver. */
zzub_audiodriver_t* zzub_audiodriver_create(zzub_player_t* player) {
	{
		zzub_audiodriver_t* d = zzub_audiodriver_create_native(player);
		if (d) return d;
	}

	return 0;
}

int zzub_audiodriver_get_count(zzub_audiodriver_t* driver) {
	return driver->get_device_count();
}

int zzub_audiodriver_create_device(zzub_audiodriver_t* driver, const char* input_name, const char* output_name, int buffersize, int samplerate) {
	return driver->create_device(output_name, input_name, buffersize, samplerate)?0:-1;
}

void zzub_audiodriver_enable(zzub_audiodriver_t* driver, int state) {
	driver->enable(state?true:false);
}

int zzub_audiodriver_get_enabled(zzub_audiodriver_t* driver) {
	return driver->worker->work_started?1:0;
}

void zzub_audiodriver_destroy(zzub_audiodriver_t* driver) {
	driver->destroy_device();
	delete driver;
}

/** \brief De-allocate the current device. */
void zzub_audiodriver_destroy_device(zzub_audiodriver_t* driver) {
	driver->destroy_device();
}

unsigned int zzub_audiodriver_get_samplerate(zzub_audiodriver_t* driver) {
	return driver->get_samplerate();
}

unsigned int zzub_audiodriver_get_buffersize(zzub_audiodriver_t* driver) {
	return driver->get_buffersize();
}

int zzub_audiodriver_get_master_channel(zzub_audiodriver_t* driver) {
	return driver->master_channel;
}

void zzub_audiodriver_set_master_channel(zzub_audiodriver_t* driver, int index) {
	driver->master_channel = index;
}

double zzub_audiodriver_get_cpu_load(zzub_audiodriver_t* driver) {
	return driver->get_cpu_load();
}

zzub_device_info_t* zzub_audiodriver_get_device_info(zzub_audiodriver_t* audiodriver, int index) {
	return audiodriver->get_device(index);
}

zzub_device_info_t* zzub_audiodriver_get_device_info_by_name(zzub_audiodriver_t* audiodriver, const char* name) {
	return audiodriver->get_device_by_name(name);
}

zzub_device_info_t* zzub_audiodriver_get_current_device(zzub_audiodriver_t* audiodriver, int for_input) {
	if (for_input)
		return audiodriver->get_device_by_name(audiodriver->worker->work_in_device);
	else
		return audiodriver->get_device_by_name(audiodriver->worker->work_out_device);

}

zzub_device_info_iterator_t* zzub_audiodriver_get_output_iterator(zzub_audiodriver_t* audiodriver) {
	audiodeviceiterator* it = new audiodeviceiterator();
	it->owner = audiodriver;
	for (int i = 0; i < audiodriver->get_device_count(); ++i)
		if (audiodriver->get_device(i)->out_channels > 0)
			it->items.push_back(audiodriver->get_device(i));
	it->i = it->items.begin();
	return it;
}

zzub_device_info_iterator_t* zzub_audiodriver_get_input_iterator(zzub_audiodriver_t* audiodriver) {
	audiodeviceiterator* it = new audiodeviceiterator();
	it->owner = audiodriver;
	for (int i = 0; i < audiodriver->get_device_count(); ++i)
		if (audiodriver->get_device(i)->in_channels > 0)
			it->items.push_back(audiodriver->get_device(i));
	it->i = it->items.begin();
	return it;
}

zzub_device_info_iterator_t* zzub_audiodriver_get_input_iterator_for_output(zzub_audiodriver_t* audiodriver, zzub_device_info_t* info) {
	audiodeviceiterator* it = new audiodeviceiterator();
	it->owner = audiodriver;

	for (int i = 0; i < audiodriver->get_device_count(); ++i)
		if (audiodriver->get_device(i)->api_id == info->api_id && audiodriver->get_device(i)->in_channels > 0) {
			// add all non-asio devices, only add the same asio devices with the same name
			// TODO: haxx! belongs elsewhere
			if (info->api_id != api_asio || info->name == audiodriver->get_device(i)->name)
				it->items.push_back(audiodriver->get_device(i));
		}
	it->i = it->items.begin();
	return it;
}

void zzub_audiodriver_configure(zzub_audiodriver_t* driver) {
	driver->configure();
}

// midi driver

int zzub_mididriver_get_count(zzub_player_t *player) {
	return (int)player->midiDriver->getDevices();
}

const char *zzub_mididriver_get_name(zzub_player_t *player, int index) {
	return player->midiDriver->getDeviceName(index);
}

int zzub_mididriver_is_input(zzub_player_t *player, int index) {
	return player->midiDriver->isInput(index)?1:0;
}

int zzub_mididriver_is_output(zzub_player_t *player, int index) {
	return player->midiDriver->isOutput(index)?1:0;
}

int zzub_mididriver_open(zzub_player_t *player, int index) {
	int result = player->midiDriver->openDevice(index)?0:-1;
	player->midi_device_changed();
	return result;
}

int zzub_mididriver_close_all(zzub_player_t *player) {
	int result = player->midiDriver->closeAllDevices()?0:-1;
	player->midi_device_changed();
	return result;
}

}
