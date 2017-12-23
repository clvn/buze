#include <windows.h>
#include <string>
#include <vector>
#include <cstring>
#include <cassert>
#include "MachineIndex.h"
#include "utils.h"

/*

Buzz machine index.txt parser.

This was made a while back, but updated in july 2006 to support loading machine instruments.
the save-method does not yet handle this, so outputted indexes from this code may grow unepextedly
when populating the instrument lists.

*/

using namespace std;

// found the trims in one of the comments at http://www.codeproject.com/vcpp/stl/stdstringtrim.asp

IndexItem* MachineIndex::parseLine(const std::string& line, FileReader &reader) {
	if (line.length() == 0) return 0;
	// try to split on comma, if second part is blank, it is hidden

	std::string::size_type cp = line.find_first_of(',');
	string name, fileName;
	if (cp != std::string::npos) {
		fileName = trim(line.substr(0, cp));
		name = trim(line.substr(cp + 1));
	} else {
		name = fileName = trim(line);
	}

	if (fileName == "1") {
		// separator
		return new MachineSeparator(name);
	} else {
		MachineItem* item=new MachineItem();
		if (fileName.length()>0 && fileName.at(0)=='*') {
			item->preload=true;
			fileName=fileName.substr(1);
		} else
			item->preload=false;
		item->fileName=fileName;
		item->fullMachineName=fileName;
		item->label=name;
		if (name.length()==0)
			item->hide=true;
		return item;
	}
}

MachineMenu* MachineIndex::parseMenu(const std::string& firstLine, FileReader &reader) {
	MachineMenu* menu=new MachineMenu();
	assert(firstLine.length()>0);
	menu->label=firstLine.substr(1);

	string line;
	while (!reader.eof()) {
		line=trim(reader.readLine());
		if (line=="/..") break;
		if (line.length()==0) {
			continue;
		} else
		if (line.at(0)=='/') {
			MachineMenu* childMenu=parseMenu(line, reader);
			if (childMenu)
				menu->append(childMenu);
		} else
		if (line.at(0) == '#') {
			const char *str;

			str = line.c_str();
			if(str && strlen(str) > 15 && !strncmp(str, "# buze-include ", 15))
			{
				open(str + 15, menu);
			}
		} else {
			IndexItem* item=parseLine(line, reader);
			if (item) {
				menu->append(item);
			}
		}
	}
	return menu;
}

bool MachineIndex::open(const char* fn, MachineMenu *menu) {
	FileReader reader;
	string line;

	if(!menu)
		menu = &root;

	if (!reader.open(fn)) return false;
	while (!reader.eof()) {
		//cout << reader.readLine() << endl;
		line=trim(reader.readLine());
		IndexItem* item=0;

		if (line.length()==0) {
			item=0;
		} else
		if (line.at(0)=='/') {
			item=parseMenu(line, reader);
		} else
		if (line.at(0) == '#') {
			const char *str;

			str = line.c_str();
			if(str && strlen(str) > 15 && !strncmp(str, "# buze-include ", 15))
			{
				open(str + 15, menu);
			}
			item=0;
		} else
			item=parseLine(line, reader);

		if (item)
			menu->append(item);
			
	}
	reader.close();
	return true;
}

void MachineIndex::saveItem(MachineItem* item) {
	string fileName = (item->preload?"*":"") + item->fileName;
	if (item->hide) {
		writer.writeLine(fillString(' ', saveDepth) + fileName + ",");
	} else 
	if (item->fileName==item->label) {
		writer.writeLine(fillString(' ', saveDepth) + fileName);
	} else {
		writer.writeLine(fillString(' ', saveDepth) + fileName + "," + item->label);
	}
}

void MachineIndex::saveItem(MachineMenu* menu) {
	if (menu->preloadReplaced != 0) {
		saveItem(menu->preloadReplaced);
		return ;
	}

	if (menu->parent) {
		writer.writeLine(fillString(' ', saveDepth) + "/" + menu->label);
		saveDepth++;
	}
	for (size_t i=0; i<menu->items.size(); i++) {
		IndexItem* item=menu->items[i];
		if (item->type==0) saveItem((MachineMenu*)item); else
		if (item->type==1) saveItem((MachineItem*)item); else
		if (item->type==2) saveItem((MachineSeparator*)item);
	}
	if (menu->parent) {
		writer.writeLine(fillString(' ', saveDepth) + "/..");
		saveDepth--;
	}
}

void MachineIndex::saveItem(MachineSeparator* sep) {
	//writer.writeLine(fillString(' ', saveDepth) + "1,-----");
	writer.writeLine(fillString(' ', saveDepth) + "1," + sep->identifier);
}

void MachineIndex::save(const char* fn) {
	saveDepth=0;
	writer.create(fn);
	saveItem(&root);
	writer.close();
}

IndexItem::~IndexItem() {
	clear();
}

void IndexItem::clear() {
	for (size_t i = 0; i < items.size(); i++) {
		delete items[i];
	}
	items.clear();
}

IndexItem* IndexItem::getAt(int index, int& currentIndex) {
	for (size_t i=0; i<items.size(); i++) {
		IndexItem* ii=items[i];
		if (ii->hide==true) continue;
		if (ii->type==0) {
			IndexItem* childItem=ii->getAt(index, currentIndex);
			if (childItem!=0) return childItem;
		} else
		if (ii->type==1) {
			if (currentIndex==index) return ii;
			currentIndex++;
		}
	}
	return 0;
}

IndexItem* IndexItem::getItemByIndex(int index) {
	int currentIndex=0;
	return getAt(index, currentIndex);
}

IndexItem* IndexItem::getItemByData(void* data) {
	if (userData == data) return this;
	for (size_t i = 0; i < items.size(); i++) {
		IndexItem* ii = items[i]->getItemByData(data);
		if (ii != 0) return ii;
	}
	return 0;
}

MachineMenu* IndexItem::getMenuByName(const std::string& name) {
	for (size_t i = 0; i<items.size(); i++) {
		if (items[i]->type == 0) {
			MachineMenu* mm = (MachineMenu*)items[i];
			if (!name.length() || !mm->label.length()) continue;
			if (mm->label == name) return mm;
		}
		MachineMenu* ii = items[i]->getMenuByName(name);
		if (ii) return ii;
	}
	return 0;
}

MachineItem* IndexItem::getMachineByName(const std::string& name) {
	for (size_t i=0; i<items.size(); i++) {
		if (items[i]->type==1) {
			MachineItem* mi=(MachineItem*)items[i];
			if (!name.length() || !mi->fileName.length()) continue;
			//assert(mi->fileName.length()>0);
			//assert(name.length()>0);
			if (stricmp(mi->fileName.c_str(), name.c_str())==0) return mi;
		}

		MachineItem* ii=items[i]->getMachineByName(name);
		if (ii) return ii;
	}
	return 0;
}


void IndexItem::append(IndexItem* item) {
	assert(item!=0);
	item->parent=this;
	items.push_back(item);
}

bool IndexItem::insertBefore(IndexItem* node, IndexItem* refnode) {
	std::vector<IndexItem*>::iterator i = find(items.begin(), items.end(), refnode);
	if (i == items.end()) return false;
	node->parent = this;
	items.insert(i, node);
	return true;
}

bool IndexItem::insertAfter(IndexItem* node, IndexItem* refnode) {
	std::vector<IndexItem*>::iterator i = find(items.begin(), items.end(), refnode);
	if (i == items.end()) return false;
	i++;
	node->parent = this;
	items.insert(i, node);
	return true;
}

bool IndexItem::replaceItem(IndexItem* prevItem, IndexItem* newItem) {
	for (size_t i=0; i<items.size(); i++) {
		if (items[i]==prevItem) {
			newItem->parent=prevItem->parent;
			//delete prevItem;
			items[i]=newItem;
			return true;
		}
	}
	return false;
}

bool IndexItem::removeItem(IndexItem* item) {
	vector<IndexItem*>::iterator i = find(items.begin(), items.end(), item);
	if (i == items.end()) return false;
	(*i)->parent = 0;
	items.erase(i);
	return true;
}
