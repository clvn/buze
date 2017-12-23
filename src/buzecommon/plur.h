/*
implements support for overloader index.plur for skinning buzz' create machine menu

pseudish example:

	class CPlurView : public CWindowImpl<CPlurView>
	{
	public:
		MachineIndex machineIndex;
		PlurManager plur;
	...


		BEGIN_MSG_MAP(CPlurView)
			MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
			MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
			MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		END_MSG_MAP()

		OnRButtonDown() {
			if (machineIndexNode.type == 2)
				menu.InsertItem(0, "", MF_BYPOSITION|MF_OWNERDRAW, machineIndexNode.fileName);
			TrackPopupMenu(menu)
		}

		OnMeasureItem() {
			plurItem = plur.getItem(measureItem->itemData->name)
			measureItem->itemSize = plurItem.size
		}

		OnDrawItem() {
			plurItem = plur.getItem(measureItem->itemData->name)

		}
	}
*/


namespace ini {

struct section {
	std::string fileName;
	std::string sectionName;

	template <typename T>
	T get(std::string name, T defaultValue);

	template<>
	int get(std::string name, int defaultValue) {
		char defaultStr[16];
		itoa(defaultValue, defaultStr, 10);
		char result[1024];
		GetPrivateProfileString(sectionName.c_str(), name.c_str(), defaultStr, result, 1024, fileName.c_str());
		return atoi(result);
	}
	template<>
	std::string get(std::string name, std::string defaultValue) {
		if (defaultValue == "") defaultValue = name;
		char result[1024];
		GetPrivateProfileString(sectionName.c_str(), name.c_str(), defaultValue.c_str(), result, 1024, fileName.c_str());
		return result;
	}
};

struct file {
	std::string fileName;	// MSDN: If this parameter does not contain a full path to the file, the system searches for the file in the Windows directory. 

	file(std::string fn) { fileName = fn; }
	ini::section section(std::string sectionName) {
		ini::section inis = { fileName, sectionName };
		return inis;
	}
};

}

struct MachineMenuItemStyle {
	int headerWidth, headerHeight;
	COLORREF textColor, backgroundColor;
	int textSize;
	bool textBold, textItalic, textUnderline;
	std::string textFace;
	int textLeft, textTop;
	int textFlags;	// DT_VCENTER, DT_CENTER
	CFont font;
};

struct MachineMenuItem {
	HBITMAP bitmap;
	std::string text;
	MachineMenuItemStyle* style;
};

struct PlurManager {
	std::map<std::string, MachineMenuItem> bitmaps;
	MachineMenuItemStyle bigStyle, normalStyle;

	bool open(std::string plurFullPath);
	void readStyles(ini::section& section, MachineMenuItemStyle& style);
	void readImages(ini::section& section, MachineMenuItemStyle& style, HMODULE hImages);
	
	MachineMenuItem* getItem(std::string name);
	void drawItem(LPDRAWITEMSTRUCT lpDrawItem, MachineMenuItem* item);
};
