#if defined(USE_LIBSF2)

import_sf2::import_sf2() {
	printf("import_sf2()");
	pRIFF = 0;
	file = 0;
}

bool import_sf2::open(std::string filename) {
	printf("import_sf2::open()");

	try {
		pRIFF = new RIFF::File(filename);
		file = new sf2::File(pRIFF);
	} catch (RIFF::Exception& e) {
		return false;
	}
	return true;
}

int import_sf2::get_wave_count() {
	assert(file != 0);
	return file->GetInstrumentCount();
}

std::string import_sf2::get_wave_name(int i) {
	sf2::Instrument* sf2i = file->GetInstrument(i);
	return sf2i->Name;
}

int import_sf2::get_wave_level_count(int i) {
	sf2::Instrument* sf2i = file->GetInstrument(i);
	return sf2i->GetRegionCount();
}

bool import_sf2::get_wave_level_info(int i, int level, importwave_info& info) {
	sf2::Instrument* sf2i = file->GetInstrument(i);
	sf2::Region* reg = sf2i->GetRegion(level);
	sf2::Sample* smp = reg->GetSample();

	info.channels = smp->GetChannelCount();
	info.sample_count = smp->GetTotalFrameCount();
	info.samples_per_second = smp->SampleRate;
	info.format = zzub_wave_buffer_type_si16; // ??
	info.name = smp->Name;

	printf("import_sf2::get_wave_level_info()");
	return true;
}

void import_sf2::read_wave_level_samples(int i, int level, void* buffer) {
	sf2::Instrument* sf2i = file->GetInstrument(i);
	sf2::Region* reg = sf2i->GetRegion(level);
	sf2::Sample* smp = reg->GetSample();

	smp->Read(buffer, smp->GetTotalFrameCount());
}

void import_sf2::close() {
	delete file;
	delete pRIFF;
	printf("import_sf2::close()");
}

#endif

#if defined(USE_SFENABLER)

import_sf2::import_sf2() {
	printf("import_sf2()");
}

bool import_sf2::open(std::string filename) {
	sfBank = sfReadSFBFile((char*)filename.c_str());
	return sfBank != -1;
}

int import_sf2::get_wave_count() {
	WORD count;
	sfGetPresetHdrs(sfBank, &count);
	return count;
}

std::string import_sf2::get_wave_name(int i) {
	WORD count;
	SFPRESETHDRPTR prshdrs = sfGetPresetHdrs(sfBank, &count);
	return prshdrs[i].achPresetName;
}

int import_sf2::get_wave_level_count(int i) {
	WORD count;
	SFPRESETHDRPTR prshdrs = sfGetPresetHdrs(sfBank, &count);

	return sfGetLayerCount(sfBank, prshdrs[i].wPresetBank, prshdrs[i].wPresetNum);
}

bool import_sf2::get_wave_level_info(int i, int level, importwave_info& info) {
	return false;
}

void import_sf2::read_wave_level_samples(int i, int level, void* buffer) {
}

void import_sf2::close() {
	sfUnloadSFBank(sfBank);
}

#endif


#if defined(USE_LIBSF2)

struct import_sf2_factory : importfactoryimpl<import_sf2> {
	import_sf2_factory() {
		extensions.push_back("sf2");
	}

	virtual bool is_container() {
		return true;
	}
};

struct import_sfz_factory : importfactoryimpl<import_sfz> {
	import_sfz_factory() {
		extensions.push_back("sfz");
	}

	virtual bool is_container() {
		return true;
	}
};

#endif

#if defined(USE_SFENABLER)

struct import_sf2_factory : importfactoryimpl<import_sf2> {
	import_sf2_factory() {
		extensions.push_back("sf2");
	}

	virtual bool is_container() {
		return true;
	}
};

#endif
