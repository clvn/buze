#include <windows.h>
#include <iostream>
#include <set>
#include <list>
#include "PvstMachines.h"

// most of this code was ripped from the sources of polacs machines.dll

using std::cerr;
using std::endl;

struct PLUGID
{	
	char *effname;
	char *plugpath;
	char *plugroot;	

	DWORD uid;
	char type;
	CLSID *clsid;	

	char *vstdir;
	
	PLUGID()
	{						
		effname=0;
		plugpath=0;
		plugroot=0;
		clsid=0;
		uid=0;		
		type=0;	
		vstdir=0;
	}
	
	PLUGID(char *name, char *path, char *root, char *vdir, DWORD id, BYTE t, CLSID *c)
	{						
		effname=name;		
		plugpath=path;
		plugroot=root;																
		clsid=c;
		uid=id;
		type=t;
		vstdir=vdir;
	}	
	
	bool operator <(const PLUGID&p)const
	{		
		if ( _stricmp(effname,p.effname)<0 ) return true;		
		
											
		return false;
	}	

	bool operator==(const PLUGID&p)const
	{		
		if ( !_stricmp(effname,p.effname) ) return true;		
		
		return false;
	}

};

struct MENUITEM
{
	char name[60];
	int  menuID;

	MENUITEM()
	{
		name[0]=0;
	}
	MENUITEM(const char* n, int i)
	{
		if(n)strncpy(name,n,60);
		else name[0]=0;
		menuID=i;
	}
	bool operator<(MENUITEM& m)
	{
		int result=strcmpi(name,m.name);
		if(result<0)return true;
		else return false;
	}
};

struct MENU;

struct LPMENU
{
	MENU* menu;
	LPMENU(){ menu=0; }
	bool LPMENU::operator<(const LPMENU& p);
};


struct MENU
{
	enum {
		max_columns = 128,
	};

	std::list<LPMENU>	subMenus;
	std::list<MENUITEM>	items;
	char				name[60];
	MENU()
	{
		ZeroMemory(name,60);
		//name[0]=0;
	}
	~MENU()
	{
		items.clear();
		
		std::list<LPMENU>::iterator it;
		for(it=subMenus.begin();it!=subMenus.end();++it)
		{
			if(it->menu)delete it->menu;
		}
		subMenus.clear();	
	}


	void clear()
	{
		items.clear();		
		std::list<LPMENU>::iterator it;
		for(it=subMenus.begin();it!=subMenus.end();++it)
		{
			if(it->menu)delete it->menu;
		}
		subMenus.clear();	
	}
	MENU(const char*n)
	{		
		if(n) strcpy(name,n);
		else name[0]=0;
	}

	void insertItem(const char* subMenu,MENUITEM m)
	{
		char sub[256];
		if(!subMenu)
		{
			items.push_back(m);
			return;
		}
		strcpy(sub,subMenu);
		char* slash;
		slash=strchr(sub,'\\');
		if(slash)
		{
			for(;;)
			{
				*slash++=0;
				if(*slash!='\\')break;
			}
		}
		if(!strlen(sub))
		{
			items.push_back(m);
			return;
		}
		else
		{
			for(std::list<LPMENU>::iterator it=subMenus.begin();it!=subMenus.end();++it)
			{
				if(!strcmpi(it->menu->name,sub))
				{
					it->menu->insertItem(slash,m);
					return;
				}
			}
		}

		LPMENU lpmenu;
		lpmenu.menu=new MENU(sub);
		subMenus.push_back(lpmenu);
		lpmenu.menu->insertItem(slash,m);
	}
	
	

void createMenu(HMENU hMenu, int separators, int columns)
	{
		subMenus.sort();
		items.sort();

		int tmpcolumns=columns;
		int tmpseparators=separators;

		MENUITEMINFO mi={0};		
		mi.cbSize=sizeof(mi);
		
		mi.fMask=MIIM_TYPE|MIIM_SUBMENU;
		mi.fType=MFT_STRING;		

		std::list<LPMENU>::iterator itm;				
		
		float i=0;		
		int dxmenu=0;
		
		int menuitems=subMenus.size();
		menuitems+=items.size();

		if (menuitems>max_columns) tmpcolumns=0;				
		
		int ssize=subMenus.size();

		for (itm=subMenus.begin();itm!=subMenus.end();++itm,i++)
		{											
			if (tmpcolumns)
			{
				if (i>=31.5f) 							
				{
					i=0;
					mi.fType=MFT_MENUBARBREAK|MFT_STRING;			
				}
				else mi.fType=MFT_STRING;				
			}																		

			dxmenu=0;
			if ((itm->menu->name[0]=='*')||(itm->menu->name[0]=='+')||(itm->menu->name[0]=='!'))
			{			
				mi.dwTypeData=&itm->menu->name[1];
				mi.hSubMenu=CreatePopupMenu();
				InsertMenuItem(hMenu,-1,TRUE,&mi);				
								
				//if ( items.size() )
				
				if ( items.size() || (i+1)!=ssize )
				{
					dxmenu=1;

					mi.fMask=MIIM_TYPE;
					mi.fType=MFT_SEPARATOR;
					InsertMenuItem(hMenu,-1,TRUE,&mi);

					i+=0.5f;
					mi.fMask=MIIM_TYPE|MIIM_SUBMENU;
					mi.fType=MFT_STRING;		
				}				
			}
			else if (itm->menu->name[0]=='|')
			{
				mi.dwTypeData=&itm->menu->name[1];
				mi.hSubMenu=CreatePopupMenu();
				InsertMenuItem(hMenu,-1,TRUE,&mi);	
			}
			else
			{
				mi.dwTypeData=itm->menu->name;
				mi.hSubMenu=CreatePopupMenu();
				InsertMenuItem(hMenu,-1,TRUE,&mi);
			}												
			
			itm->menu->createMenu(mi.hSubMenu,separators,columns);
			
		}
				
		if (tmpcolumns)
		{
			if (i<31.5f)				
			{
				if ( subMenus.size() && items.size() && !dxmenu)
				{						
					i+=0.5f;
					mi.fMask=MIIM_TYPE;
					mi.fType=MFT_SEPARATOR;
					InsertMenuItem(hMenu,-1,TRUE,&mi);			
				}
			}			
		}
		else 
		{
			if ( subMenus.size() && items.size() && !dxmenu)
			{					
				mi.fMask=MIIM_TYPE;
				mi.fType=MFT_SEPARATOR;
				InsertMenuItem(hMenu,-1,TRUE,&mi);			
			}
		}
		
		mi.fMask=MIIM_TYPE|MIIM_ID;
		mi.fType=MFT_STRING;	
		char oldletter=0;
		std::list<MENUITEM>::iterator it;		
		for(it=items.begin();it!=items.end();++it,i++)
		{											
			if (tmpcolumns)
			{
				if (i>=31.5f)
				{
					i=0;				
					mi.fType=MFT_MENUBARBREAK|MFT_STRING;			
				}				
				else mi.fType=MFT_STRING;								
			}

			char newletter=toupper(it->name[0]);				
			
			if (tmpseparators)
			{
				if (it!=items.begin())
				{				
					if (newletter!=oldletter)
					{							
						if (tmpcolumns)
						{
							if (i)
							{
								if (i<31)								
								{
									mi.fMask=MIIM_TYPE;
									mi.fType=MFT_SEPARATOR;									
									i+=0.5f;
									InsertMenuItem(hMenu,-1,TRUE,&mi);
								}
								else 
								{
									i=0;																						
								}
							}
							mi.fMask=MIIM_TYPE|MIIM_ID;						
							if (!i) mi.fType=MFT_MENUBARBREAK|MFT_STRING;
							else mi.fType=MFT_STRING;														
							oldletter=newletter;						
						}
						else
						{
							mi.fMask=MIIM_TYPE;
							mi.fType=MFT_SEPARATOR;
							InsertMenuItem(hMenu,-1,TRUE,&mi);						
							mi.fMask=MIIM_TYPE|MIIM_ID;												
							mi.fType=MFT_STRING;														
							oldletter=newletter;	
						}
					}					
				}
				else
				{
					oldletter=newletter;
				}	
			}
									
			mi.dwTypeData=it->name;

			int leng=strlen(mi.dwTypeData)-1;			
			if (mi.dwTypeData[leng]=='*')
			{
				*strrchr(mi.dwTypeData,' ')=0;			
			}

			mi.wID=it->menuID;
			InsertMenuItem(hMenu,-1,TRUE,&mi);
		}		
	}
};


inline bool LPMENU::operator<(const LPMENU& p) {
	if(!menu)return true;
	if(stricmp(menu->name,p.menu->name)<0)return true;
	else return false;
}

struct pvst_menu_init {
	//extern "C" __declspec(dllexport) void * __cdecl getMenu(void *pl, int type);
	typedef void*(getMenuFunc)(void*, int);
	//extern "C" __declspec(dllexport) int __cdecl getListIndex(int n, int opcode, CMachineInterface** ppMI, FARPROC* pCallback, CMachine** ppM, CMachineInfo **ppInfo, void** ppOpt)
	typedef int(getListIndexFunc)(int, int, void** ppMI, FARPROC* pCallback, void* ppM, void** ppInfo, void** ppOpt);
	//typedef int(getListIndexFunc)(int, int, CMachineInterface** ppMI, FARPROC* pCallback, CMachine* ppM, CMachineInfo** ppInfo, void** ppOpt);

	HMODULE machines_handle;

	pvst_menu_init() {
		machines_handle = LoadLibrary("Gear/Machines.dll");
	}

	~pvst_menu_init() {
		if (machines_handle) {
			FreeLibrary(machines_handle);
			machines_handle = 0;
		}
	}

	std::set<PLUGID> get_plugins(int type) {
		std::set<PLUGID> result;

#if !(_MSC_VER == 1310 || (_MSC_VER == 1400 && !_DEBUG))
		// getMenu returns a std::set from vs2003's STL, so bail unless thats what we're using
		// VS2005 Release (but not Debug) also produces a compatible executable
		return result;
#endif

		if (!machines_handle) return result;

		getMenuFunc* getMenu = (getMenuFunc*)GetProcAddress(machines_handle, "getMenu");

		if (!getMenu) return result;

		// this works as long as machines.dll/pvst(i) and buze are compiled
		// with the same versions of vc++ and stl
		void* p = getMenu(0, type);
		if (!p) return result;

		result = *(std::set<PLUGID>*)p;

		return result;
	}

	HMENU get_menu(int type, DWORD dwFirstCommand) {

		std::set<PLUGID> plugins = get_plugins(type);
		if (plugins.size() == 0) return 0;

		char plugpath[MAX_PATH] = { 0 };
		int x = 0;

		MENU pvstiMenuBuzz = 0;
		DWORD column_flag = 1;
		DWORD separator_flag = 0;

		for (std::set<PLUGID>::iterator i = plugins.begin(); i != plugins.end(); ++i) {

			PLUGID id = *i;						
			if (id.type <= 0 || id.type >= 3) {									
				strcpy(plugpath, id.plugroot);

				char* buf = strrchr(plugpath,'\\');
				if (buf) *buf = 0;
				else plugpath[0] = 0;

			} else {
				strcpy(plugpath, id.plugroot);
			}

			MENUITEM mii(id.effname, dwFirstCommand+x);
			pvstiMenuBuzz.insertItem(plugpath, mii);

			x++;
/*

			cerr << "effname: " << i->effname << endl;
			cerr << "plugpath: " << i->plugpath << endl;
			cerr << "plugroot: " << i->plugroot << endl;
			cerr << "type: " << (int)i->type << endl;
			if (i->vstdir)
				cerr << "vstdir: " << i->vstdir << endl;
			cerr << endl;*/
		}


		HMENU buzzpvsti = CreatePopupMenu();
		pvstiMenuBuzz.createMenu(buzzpvsti, separator_flag, column_flag);

		return buzzpvsti;
	}
#define MACH_PVST					2
#define HIDE						0
#define SHOW						1
typedef int (*MACHINECALLBACK)(void*,int,int,int,float,void*);

	void set_show_hide_flag(bool flag) {
		getListIndexFunc* getListIndex = (getListIndexFunc*)GetProcAddress(machines_handle, "getListIndex");

		if (!getListIndex) return ;
		int show_hide_flag=flag?SHOW:HIDE;
		int size=getListIndex(-1,MACH_PVST,0,0,0,0,0);
		if (size)
		{					
			for (int i=0;i<size;i++)
			{
				void* cmi;
				FARPROC callback;						
				getListIndex(i,MACH_PVST,&cmi,&callback,0,0,0);
				MACHINECALLBACK MachineCallback=(MACHINECALLBACK)callback;											
				MachineCallback(cmi,MACH_PVST,show_hide_flag,0,0,NULL);
			}					
		}
	}
};

namespace {
	// this loads machines.dll for the lifetime of the app:
	pvst_menu_init pvst_menu;
};

HMENU pvst_get_menu(int type, DWORD dwFirstCommand) {
	return pvst_menu.get_menu(type, dwFirstCommand);
}

void pvst_show_hide_all(bool show) {
	pvst_menu.set_show_hide_flag(show);
}
