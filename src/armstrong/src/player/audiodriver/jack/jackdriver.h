#pragma once

struct jack_deviceport {
	std::string physicalname;
	std::string clientname;
	jack_port_t* port;
};

struct jack_device {
	jack_client_t *client;
	std::vector<jack_deviceport> output_ports;
	std::vector<jack_deviceport> input_ports;

	audiodriver_callback_t usercallback;
	void* userdata;
};

void jack_get_deviceinfo(std::vector<deviceinfo>* devices);
bool jack_create_device(const std::string& clientname, audiodriver_callback_t usercallback, void* userdata, jack_device* device);
void jack_device_start(jack_device* device);
void jack_device_stop(jack_device* device);
void jack_destroy_device(jack_device* device);
