#pragma once

// allows inputting '@' to return 2
inline int getShiftedNumeric(WORD c) {
	static const BYTE keystate[256] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x80 };

	WORD result[2];
	for (int i = 0; i <= 9; ++i) {
		//int count = ToAsciiEx((UINT)(0x30 + i), 0, keystate, result, 0, GetKeyboardLayout(GetCurrentThreadId()));
		int count = ToAscii((UINT)(0x30 + i), 0, keystate, result, 0);
		if ((count == 1) && (c == (char)result[0]))
			return i;
	}

	return -1;
}

