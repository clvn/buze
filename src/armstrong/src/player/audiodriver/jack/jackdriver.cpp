#include <vector>
#include <sstream>
#include <iostream>
#include <cstring>
#if defined(_STDINT) && !defined(_STDINT_H)
#define _STDINT_H
#endif
#include <jack/jack.h>
#include "../audiodriver.h"
#include "jackdriver.h"

// the driver always creates "max_jack_channels" of each audio input and output ports (2x max total).
// physical ports are mapped 1:1 until the max. any unmapped channels can be used for aux.
const int max_jack_channels = 16;

using std::cerr;
using std::cout;
using std::cin;
using std::endl;

void jack_get_deviceinfo(std::vector<deviceinfo>* devices) {
	// assume linking against the delay loading JackWeakAPI. assume the 
	// version is never NULL intentionally when jack is installed.
	const char* jackversion = jack_get_version_string();
	if (jackversion == 0) return ;

	// return an informational jack device to allow starting the jack server at anytime
	deviceinfo info;
	info.name = "Jack";
	info.api = api_jack;
	info.numinputs = max_jack_channels;
	info.numoutputs = max_jack_channels;
	info.defaultbuffersize = 0;
	info.defaultsamplerate = 0;
	devices->push_back(info);
}

bool jack_port_is_audio(jack_port_t* port) {
	const char* porttype = jack_port_type(port);
	return strcmp(porttype, JACK_DEFAULT_AUDIO_TYPE) == 0;
}

void jack_get_audio_ports(jack_client_t* client, const char** ports, std::vector<std::string>* result) {
	if (ports == 0) return ;
	for (; *ports; ports++) { 
		jack_port_t* port = jack_port_by_name(client, *ports);
		if (jack_port_is_audio(port)) {
			result->push_back(*ports);
		}
	}
}

void jack_get_physical_output_ports(jack_client_t* client, std::vector<std::string>* result) {
	const char **ports = jack_get_ports(client, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
	jack_get_audio_ports(client, ports, result);
}

void jack_get_physical_input_ports(jack_client_t* client, std::vector<std::string>* result) {
	const char **ports = jack_get_ports(client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput);
	jack_get_audio_ports(client, ports, result);
}

int jack_process_callback(jack_nframes_t nframes, void *arg) {

	jack_device* device = (jack_device*)arg;

	float* plout[64] = {0};
	float* plin[64] = {0};
	for (int i = 0; i < (int)device->output_ports.size(); i++) {
		jack_port_t* port = device->output_ports[i].port;
		jack_default_audio_sample_t* portbuffer = (jack_default_audio_sample_t *) jack_port_get_buffer(port, nframes);
		plout[i] = portbuffer;
	}

	for (int i = 0; i < (int)device->input_ports.size(); i++) {
		jack_port_t* port = device->input_ports[i].port;
		jack_default_audio_sample_t* portbuffer = (jack_default_audio_sample_t *) jack_port_get_buffer(port, nframes);
		plin[i] = portbuffer;
	}

	device->usercallback(callbacktype_buffer, plout, plin, nframes, device->userdata);
	return 0;      
}

bool jack_create_device(const std::string& clientname, audiodriver_callback_t usercallback, void* userdata, jack_device* device) {
	device->client = jack_client_new(clientname.c_str());
	if (device->client == 0) return false;

	device->usercallback = usercallback;
	device->userdata = userdata;

	std::vector<std::string> inputnames, outputnames;
	jack_get_physical_output_ports(device->client, &outputnames);
	device->output_ports.resize(max_jack_channels);

	for (int i = 0; i < max_jack_channels; i++) {
		std::stringstream portname;
		portname << clientname << "-Out" << i;
		jack_deviceport& jdp = device->output_ports[i];
		if (i < (int)outputnames.size())
			jdp.physicalname = outputnames[i];
		else
			jdp.physicalname = "";
		jdp.port = jack_port_register(device->client, portname.str().c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		jdp.clientname = jack_port_name(jdp.port);
	}

	jack_get_physical_input_ports(device->client, &inputnames);
	device->input_ports.resize(max_jack_channels);
	for (int i = 0; i < max_jack_channels; i++) {
		std::stringstream portname;
		portname << clientname << "-In" << i;
		jack_deviceport& jdp = device->input_ports[i];
		if (i < (int)inputnames.size())
			jdp.physicalname = inputnames[i];
		else
			jdp.physicalname = "";
		jdp.port = jack_port_register(device->client, portname.str().c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
		jdp.clientname = jack_port_name(jdp.port);
	}

	//device->samplerate = jack_get_sample_rate(device->client);
	//jack_get_buffer_size(device->client);
	// jack_set_buffer_size_callback
	// jack_set_sample_rate_callback
	// jack_set_latency_callback
	jack_set_process_callback(device->client, &jack_process_callback, device);
	return true;
}

void jack_device_start(jack_device* device) {
	jack_activate(device->client);

	// connect physical outputs
	{
		for (std::vector<jack_deviceport>::iterator i = device->output_ports.begin(); i != device->output_ports.end(); ++i) {
			if (i->physicalname.empty()) continue;
			if (jack_connect(device->client, i->clientname.c_str(), i->physicalname.c_str()) != 0) {
				cerr << "jackdriver: cannot connect " << i->clientname << " to " << i->physicalname << endl;
			}
		}
	}

	// connect physical inputs
	{
		for (std::vector<jack_deviceport>::iterator i = device->input_ports.begin(); i != device->input_ports.end(); ++i) {
			if (i->physicalname.empty()) continue;
			if (jack_connect(device->client, i->physicalname.c_str(), i->clientname.c_str()) != 0) {
				cerr << "jackdriver: cannot connect " << i->clientname << " to " << i->physicalname << endl;
			}
		}
	}
}

void jack_device_stop(jack_device* device) {
	// disconnect physical outputs
	{
		for (std::vector<jack_deviceport>::iterator i = device->output_ports.begin(); i != device->output_ports.end(); ++i) {
			if (i->physicalname.empty()) continue;
			if (jack_disconnect(device->client, i->clientname.c_str(), i->physicalname.c_str()) != 0) {
				cerr << "jackdriver: cannot disconnect " << i->clientname << " to " << i->physicalname << endl;
			}
		}
	}

	// disconnect physical inputs
	{
		for (std::vector<jack_deviceport>::iterator i = device->input_ports.begin(); i != device->input_ports.end(); ++i) {
			if (i->physicalname.empty()) continue;
			if (jack_disconnect(device->client, i->physicalname.c_str(), i->clientname.c_str()) != 0) {
				cerr << "jackdriver: cannot disconnect " << i->clientname << " to " << i->physicalname << endl;
			}
		}
	}

	jack_deactivate(device->client);
}

void jack_destroy_device(jack_device* device) {
	jack_client_close(device->client);
	device->client = 0;
}
