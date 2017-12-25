// plur.cpp : main source file for plur.exe
//

#include <atlbase.h>
#include <atlwin.h>
#include <wtl/atlapp.h>
#include <wtl/atlgdi.h>
#include <wtl/atlmisc.h>
#include <atlgdix.h>
#include <string>
#include <map>
#include <algorithm>
#include "plur.h"

using namespace std;

extern std::string stringFromInt(int i, int len=0, char fillChar=' ');

bool PlurManager::open(std::string plurFullPath) {

	bitmaps.clear(); // TODO: does this leak?

	string path = plurFullPath;
	replace(path.begin(), path.end(), '/', '\\');

	size_t ls = path.find_last_of('\\');
	if (ls != string::npos)
		path = path.substr(0, ls+1); else
		path ="";

	ini::file inif(plurFullPath);
	std::string generatorName = inif.section("INDEX.GENERAL").get<std::string>("generatorname", "Generator");
	std::string effectName = inif.section("INDEX.GENERAL").get<std::string>("effectname", "Effect");

	readStyles(inif.section("INDEX.NORMALHEADERS"), normalStyle);
	readStyles(inif.section("INDEX.BIGHEADERS"), bigStyle);

	std::string imageBase = path + inif.section("INDEX.NORMALHEADERS").get<std::string>("imagebase", "Classic Header Images.plurimg");

	HMODULE hImages = LoadLibraryEx(imageBase.c_str(), 0, LOAD_LIBRARY_AS_DATAFILE);

	readImages(inif.section("INDEX.NORMALHEADERS"), normalStyle, hImages);
	readImages(inif.section("INDEX.BIGHEADERS"), bigStyle, hImages);

	FreeLibrary(hImages);

	return true;
}


void PlurManager::readStyles(ini::section& section, MachineMenuItemStyle& style) {
	style.headerWidth = section.get<int>("width", 200);
	style.headerHeight = section.get<int>("height", 12);
	int nhtr = section.get<int>("textred", 255);
	int nhtg = section.get<int>("textgreen", 255);
	int nhtb = section.get<int>("textblue", 255);
	style.textColor = RGB(nhtr, nhtg, nhtb);

	nhtr = section.get<int>("backgroundred", 255);
	nhtg = section.get<int>("backgroundgreen", 255);
	nhtb = section.get<int>("backgroundblue", 255);
	style.backgroundColor = RGB(nhtr, nhtg, nhtb);

	style.textFace = section.get<std::string>("textface", "Verdana");
	style.textBold = section.get<std::string>("textweight", "") == "bold" ? true : false;
	style.textItalic = section.get<std::string>("textitalic", "no") == "yes" ? true : false;
	style.textUnderline = section.get<std::string>("textunderline", "no") == "yes" ? true : false;
	style.textSize = section.get<int>("textsize", 13);

	string horiz = section.get<std::string>("texthorizontal", "0");
	if (horiz == "center") {
		style.textLeft = 0;
		style.textFlags = DT_CENTER;
	} else {
		style.textLeft = atoi(horiz.c_str());
		style.textFlags = 0;
	}
	string vert = section.get<std::string>("textvertical", "0");
	if (vert == "center") {
		style.textTop = 0;
		style.textFlags = DT_VCENTER;
	} else {
		style.textTop = atoi(vert.c_str());
		style.textFlags = 0;
	}

	CDC tempDC;
	tempDC.CreateCompatibleDC(0);
	style.font.CreateFont(
		-MulDiv(style.textSize, GetDeviceCaps(tempDC, LOGPIXELSY), 72), 
		0, 0, 0, 
		style.textBold?FW_BOLD :0, 
		style.textItalic?TRUE:FALSE, 
		style.textUnderline?TRUE:FALSE, 
		0, DEFAULT_CHARSET, 0, 0, 0, 0, style.textFace.c_str());
}

void PlurManager::readImages(ini::section& section, MachineMenuItemStyle& style, HMODULE hImages) {
	for (int i = 0; i<50; i++) {
		string headerAsc = section.get<std::string>("header" + stringFromInt(i) + "asc", "");
		string headerImg = section.get<std::string>("header" + stringFromInt(i) + "img", "");
		string headerTxt = section.get<std::string>("header" + stringFromInt(i) + "txt", "");

		HBITMAP bitmap = 0;
		if (hImages != 0)
			bitmap = LoadBitmap(hImages, headerImg.c_str());

		MachineMenuItem mmi = { bitmap, headerTxt, &style };
		bitmaps.insert(pair<string, MachineMenuItem>(headerAsc,  mmi));
	}
}

MachineMenuItem* PlurManager::getItem(std::string name) {
	std::map<string, MachineMenuItem>::iterator mmit = bitmaps.find(name);
	if (mmit == bitmaps.end()) {
//		assert(false);
		return 0;
	}

	return &mmit->second;
}


void PlurManager::drawItem(LPDRAWITEMSTRUCT lpDrawItem, MachineMenuItem* item) {

	CMemDC dc(lpDrawItem->hDC, &lpDrawItem->rcItem);

	dc.FillSolidRect(&lpDrawItem->rcItem, item->style->backgroundColor);

	if (item->bitmap != 0) {
		CDC memDC;
		memDC.CreateCompatibleDC(0);
		CBitmapHandle bmh = memDC.SelectBitmap(item->bitmap);
		dc.BitBlt(lpDrawItem->rcItem.left, lpDrawItem->rcItem.top, lpDrawItem->rcItem.right-lpDrawItem->rcItem.left, lpDrawItem->rcItem.bottom-lpDrawItem->rcItem.top, memDC, 0, 0, SRCCOPY);
		memDC.SelectBitmap(bmh);
	}

	// flags = DT_VCENTER, DT_CENTER
	int flags = item->style->textFlags;
	RECT rc = lpDrawItem->rcItem;
	rc.left += item->style->textLeft;
	rc.right -= item->style->textLeft;
	rc.top += item->style->textTop;
	rc.bottom -= item->style->textTop;

	CFontHandle prevFont = dc.SelectFont(item->style->font);
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(item->style->textColor);
	dc.DrawText(item->text.c_str(), item->text.length(), &rc, flags);

	dc.SelectFont(prevFont);

}
