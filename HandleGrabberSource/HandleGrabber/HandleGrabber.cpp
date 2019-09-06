// HandleGrabber.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <comutil.h>
#include <vector>
#include <stack>
#include <string>
#include <atlconv.h>

#ifdef HANDLEGRABBER_EXPORTS
#define HANDLEGRABBER_API extern "C" _declspec(dllexport)
#else
#define HANDLEGRABBER_API extern "C" __declspec(dllimport)
#endif

struct handle_data {
	unsigned long process_id;
	HWND window_handle;
};

struct RETURNDATA {
	LPWSTR name;
	UINT id;
	UINT parentID;
};

HWND hwnd;
HMENU hmenu;
DWORD pid;
std::vector<HMENU> topMenus;
std::vector<MENUITEMINFO> menuItems;
std::vector<RETURNDATA> returnData;
std::stack<UINT> ancestors;
std::stack<LPWSTR> cchBuffers;

BOOL is_main_window(HWND handle) {
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam) {
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !is_main_window(handle))
		return TRUE;
	data.window_handle = handle;
	return FALSE;
}

HWND find_main_window(unsigned long process_id) {
	handle_data data;
	data.process_id = process_id;
	data.window_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.window_handle;
}

HANDLEGRABBER_API HWND InitGrabber(void) {
	pid = GetCurrentProcessId();
	hwnd = find_main_window(pid);
	while (GetParent(hwnd) != NULL)
		hwnd = GetParent(hwnd);
	hmenu = GetMenu(hwnd);
	return hwnd;
}

void Search(HMENU parent) {
	for (int i = 0; i < GetMenuItemCount(parent); i++) {
		MENUITEMINFOW tItem;
		tItem.cbSize = sizeof(MENUITEMINFO);
		tItem.fMask = MIIM_STRING | MIIM_ID;
		tItem.dwTypeData = NULL;
		bool tBool = GetMenuItemInfoW(parent, i, true, &tItem);
		if (tBool) {
			cchBuffers.push(new wchar_t[tItem.cch + 1]);
			tItem.dwTypeData = cchBuffers.top();
			tItem.cch++;
			GetMenuItemInfoW(parent, i, true, &tItem);
			LPWSTR tTypeData = tItem.dwTypeData;
			UINT tID = tItem.wID;
			UINT tAnc = 0;
			if (tID == 0)
				continue;
			if (ancestors.size() != 0)
				tAnc = ancestors.top();
			returnData.push_back({ tTypeData, tID, tAnc });
		}
		HMENU tMenu = GetSubMenu(parent, i);
		if (tMenu != NULL) {
			ancestors.push(returnData.back().id);
			Search(tMenu);
		}
	}
	if (ancestors.size() != 0)
		ancestors.pop();
}

HANDLEGRABBER_API void DiscoverMenuItems(void) {
	Search(hmenu);
}


HANDLEGRABBER_API size_t MenuItemCount(void) {
	return returnData.size();
}

HANDLEGRABBER_API BSTR _stdcall RetrieveDataName(int index) {
	return SysAllocString((BSTR)returnData.at(index).name);
}

HANDLEGRABBER_API UINT RetrieveDataID(int index) {
	return returnData.at(index).id;
}

HANDLEGRABBER_API UINT RetrieveDataParent(int index) {
	return returnData.at(index).parentID;
}

HANDLEGRABBER_API BSTR _stdcall RetrieveDataNameFromID(UINT id) {
	MENUITEMINFOW tItem;
	tItem.cbSize = sizeof(MENUITEMINFO);
	tItem.fMask = MIIM_STRING;
	tItem.dwTypeData = NULL;
	bool tBool = GetMenuItemInfoW(hmenu, id, false, &tItem);
	if (tBool) {
		cchBuffers.push(new wchar_t[tItem.cch + 1]);
		tItem.dwTypeData = cchBuffers.top();
		tItem.cch++;
		GetMenuItemInfoW(hmenu, id, false, &tItem);
		LPWSTR tTypeData = tItem.dwTypeData;
		return SysAllocString((BSTR(tTypeData)));
	}
}

HANDLEGRABBER_API void ExecuteMenuItemAt(int id) {
	PostMessageW(hwnd, WM_COMMAND, id, 0);
}

HANDLEGRABBER_API void FreeMemory() {
	topMenus.clear();
	menuItems.clear();
	returnData.clear();
	for (int i = 0; i < cchBuffers.size(); i++) {
		cchBuffers.pop();
	}
	for (int i = 0; i < ancestors.size(); i++) {
		ancestors.pop();
	}
}