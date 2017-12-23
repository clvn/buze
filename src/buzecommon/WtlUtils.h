#define REGISTERED_MSG_HANDLER(msg, func) \
	if (uMsg == *((UINT*)(&msg))) { \
		bHandled = TRUE; \
		lResult = func(uMsg, wParam, lParam, bHandled); \
		if (bHandled) \
			return TRUE; \
	}

#define CMD_ID_HANDLER(id, func) \
	if (uMsg == WM_COMMAND && id == LOWORD(wParam)) { \
		bHandled = TRUE; \
		func(); \
		return TRUE; \
	}

#define CMD_HANDLER(id, code, func) \
	if (uMsg == WM_COMMAND && id == LOWORD(wParam) && code == HIWORD(wParam)) { \
		bHandled = TRUE; \
		func(); \
		return TRUE; \
	}

#define CMD_RANGE_HANDLER(idFirst, idLast, func) \
	if (uMsg == WM_COMMAND && LOWORD(wParam) >= idFirst  && LOWORD(wParam) <= idLast) { \
		bHandled = TRUE; \
		func(LOWORD(wParam)); \
		return TRUE; \
	}
