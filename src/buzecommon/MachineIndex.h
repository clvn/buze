#pragma once

#include "FileReader.h"

class MachineItem;
class MachineMenu;

class IndexItem {
public:
	IndexItem* parent;
	int type;	// 0=machinemenu, 1=machineitem, 2=separator
	bool hide;
	std::string label;
	std::vector<IndexItem*> items;
	void* userData;

	IndexItem() { parent = 0; hide = false; userData = 0; }
	virtual ~IndexItem();
	IndexItem* getAt(int index, int& currentIndex);
	IndexItem* getItemByIndex(int index);
	IndexItem* getItemByData(void* userData);
	MachineItem* getMachineByName(const std::string& label);
	MachineMenu* getMenuByName(const std::string& label);

	bool replaceItem(IndexItem* prevItem, IndexItem* newItem);
	bool removeItem(IndexItem* item);
	void append(IndexItem* item);
	bool insertBefore(IndexItem* node, IndexItem* refnode);
	bool insertAfter(IndexItem* node, IndexItem* refnode);

	void clear();
};

class MachineItem : public IndexItem {
public:
	std::string fileName;
	bool preload;
	std::string fullMachineName;	// used internally, has a slash to separate dll name and selected instrument
	std::string instrumentName;

	MachineItem() { type=1; }
};

class MachineSeparator : public IndexItem {
public:
	std::string identifier;
	MachineSeparator() { type=2; }
	MachineSeparator(std::string id) { identifier = id; type=2; }
};

class MachineMenu : public IndexItem {
public:
	MachineItem* preloadReplaced;	

	MachineMenu() { type = 0; preloadReplaced = 0; }
};

class MachineIndex {
	FileWriter writer;
public:
	MachineMenu root;

	IndexItem* parseLine(const std::string& line, FileReader &reader);
	MachineMenu* parseMenu(const std::string& firstLine, FileReader &reader);
	bool open(const char* fn, MachineMenu *menu = NULL);


	int saveDepth;

	void saveItem(MachineItem* item);

	void saveItem(MachineMenu* menu);

	void saveItem(MachineSeparator* sep);
	void save(const char* fn);
};
