#pragma once

namespace zzub {

struct find_note_column : public std::unary_function<const zzub::parameter*, bool> {
	bool operator()(const zzub::parameter* param) {
		return param->type == zzub_parameter_type_note;
	}
};

struct find_wave_column : public std::unary_function<const zzub::parameter*, bool> {
	bool operator()(const zzub::parameter* param) {
		return param->flags & zzub_parameter_flag_wavetable_index;
	}
};

struct find_velocity_column : public std::unary_function<const zzub::parameter*, bool> {
	bool operator()(const zzub::parameter* param) {
		return (param->description.find("Velocity") != std::string::npos) || (param->description.find("Volume") != std::string::npos );
	}
};

bool get_note_info(const zzub::info* info, int& note_group, int& note_column) {
	std::vector<const zzub::parameter*>::const_iterator param;

	param = find_if(info->global_parameters.begin(), info->global_parameters.end(), find_note_column());
	if (param != info->global_parameters.end()) {
		note_group = 1;
		note_column = int(param - info->global_parameters.begin());
		return true;
	}

	param = find_if(info->track_parameters.begin(), info->track_parameters.end(), find_note_column());
	if (param != info->track_parameters.end()) {
		note_group = 2;
		note_column = int(param - info->track_parameters.begin());
		return true;
	}

	return false;
}

bool get_wave_info(const zzub::info* info, int note_group, int& wave_column) {
	std::vector<const zzub::parameter*>::const_iterator param;

	if(note_group == 1)
	{
		param = find_if(info->global_parameters.begin(), info->global_parameters.end(), find_wave_column());
		if (param != info->global_parameters.end()) {
			wave_column = int(param - info->global_parameters.begin());
			return true;
		}
	}

	param = find_if(info->track_parameters.begin(), info->track_parameters.end(), find_wave_column());
	if (param != info->track_parameters.end()) {
		wave_column = int(param - info->track_parameters.begin());
		return true;
	}

	return false;
}

bool get_velocity_info(const zzub::info* info, int note_group, int& velocity_column) {
	std::vector<const zzub::parameter*>::const_iterator param;

	if(note_group == 1)
	{
		param = find_if(info->global_parameters.begin(), info->global_parameters.end(), find_velocity_column());
		if (param != info->global_parameters.end()) {
			velocity_column = int(param - info->global_parameters.begin());
			return true;
		}
	}

	param = find_if(info->track_parameters.begin(), info->track_parameters.end(), find_velocity_column());
	if (param != info->track_parameters.end()) {
		velocity_column = int(param - info->track_parameters.begin());
		return true;
	}

	return false;
}

}
