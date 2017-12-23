#pragma once

struct keyboard_mapper {

	static int no_qwerty_keycodes[];
	static int fr_azerty_keycodes[];
	static int ch_qwerty_keycodes[];
	static int de_qwertz_keycodes[];
	static int qwerty_keycodes[];
	static int dvorak_keycodes[];

	struct note_table {
		int primary_language;
		int sub_language;
		int* key_codes;
	};

	static note_table tables[];
	static int num_tables;

	static int* lookup_locale(int plang, int slang) {
		for (int i = 0; i<num_tables; i++) {
			if (tables[i].primary_language == plang && (tables[i].sub_language == -1 || tables[i].sub_language == slang)) 
				return tables[i].key_codes;
		}
		return 0;
	}

	static int map_code_to_note(int octave, int v) {
		int plang, slang;
		get_os_layout(plang, slang);
		return map_code_to_note(octave, plang, slang, v);
	}

	static int map_code_to_note(int octave, int plang, int slang, int v) {
		int* key_codes = lookup_locale(plang, slang);
		// default to qwerty if none found
		if (!key_codes) key_codes = qwerty_keycodes;
		
		int i = 255;
		while (*key_codes) {
			if (*key_codes == v) return i == 255 ? i : (12*octave+i-12);
			// check if we're scanning into next octave
			if (*key_codes == -1) {
				if (i == 256)
					i = 0; else
					i = 12;
			} else
				i++;
			key_codes++;
		}
		return -1;
	}

	static void get_os_layout(int& plang, int& slang) {
		HKL kl = GetKeyboardLayout(GetCurrentThreadId());
		int layoutid = HIWORD(kl);  // device id, loword is language id
		plang = PRIMARYLANGID(layoutid);
		slang = SUBLANGID(layoutid);
	}

};
