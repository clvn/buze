#include <windows.h>
#include "Keymaps.h"

// Norwegian keyboard mapping
int keyboard_mapper::no_qwerty_keycodes[] = { 
	49, -1,
	90, 83, 88, 68, 67, 86, 71, 66, 72, 78, 74, 77, 188, 76, 190, 192, 189, -1,
	81, 50, 87, 51, 69, 82, 53, 84, 54, 89, 55, 85, 73, 57, 79, 48, 80, 221, 219, 186, 
	0 };

// French keyboard layout
int keyboard_mapper::fr_azerty_keycodes[] = { 
	49, -1,
	87, 83, 88, 68, 67, 86, 71, 66, 72, 78, 74, 188, 190, 76, 191, 77, 223, 192, -1, 
	65, 50, 90, 51, 69, 82, 53, 84, 54, 89, 55, 85, 73, 57, 79, 48, 80, 221, 187, 186,
	0 };

int keyboard_mapper::de_qwertz_keycodes[] = { 
	49, -1,
	89, 83, 88, 68, 67, 86, 71, 66, 72, 78, 74, 77, 188, 76, 190, -1,
	81, 50, 87, 51, 69, 82, 53, 84, 54, 90, 55, 85, 73, 57, 79, 48, 80,
	0 };

int keyboard_mapper::qwerty_keycodes[] = { 
	49, -1,
	90, 83, 88, 68, 67, 86, 71, 66, 72, 78, 74, 77, 188, 76, 190, -1,
	81, 50, 87, 51, 69, 82, 53, 84, 54, 89, 55, 85, 73, 57, 79, 48, 80,
	0 };

int keyboard_mapper::dvorak_keycodes[] = { 
	49, -1,
	186, 79, 81, 69, 74, 75, 73, 88, 68, 66, 72, 77, 87, 78, 86, 83, 90, -1, 
	222, 50, 188, 51, 190, 80, 53, 89, 54, 70, 55, 71, 67, 57, 82, 48, 76, 191, 221, 187,  
	0 };

keyboard_mapper::note_table keyboard_mapper::tables[] = { 
	{ 0x02,	0x3c, keyboard_mapper::dvorak_keycodes },		// us, dvorak
	{ 0x05,	  -1, keyboard_mapper::de_qwertz_keycodes },	// germany
	{ 0x07,	  -1, keyboard_mapper::de_qwertz_keycodes },	// austria
	{ 0x0c,	0x04, keyboard_mapper::de_qwertz_keycodes },	// france - switzerland
	{ 0x0c,	  -1, keyboard_mapper::fr_azerty_keycodes },	// france - rest
	{ 0x0e,	  -1, keyboard_mapper::de_qwertz_keycodes },	// hungary
	{ 0x13,	0x02, keyboard_mapper::fr_azerty_keycodes },	// belgium (punt)
	{ 0x14,	  -1, keyboard_mapper::no_qwerty_keycodes },	// norway
	{ 0x1e,	0x3c, keyboard_mapper::fr_azerty_keycodes },	// belgium (comma)

// everybody else gets qwerty
};

int keyboard_mapper::num_tables = sizeof(keyboard_mapper::tables) / sizeof(note_table);
