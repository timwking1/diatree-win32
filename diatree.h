#ifndef DIATREE_H
#define DIATREE_H

#ifndef UNICODE
#define UNICODE
#endif 

#ifndef FILE_DIALOG_H
#define FILE_DIALOG_H
#endif

#include <commctrl.h>
#include <commdlg.h>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <fstream>

using namespace std;
using json = nlohmann::json;

//Window procedure definition
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wPaam, LPARAM lParam);

//Context menu
#define WM_CONTEXTMENU                  0x007B
#define IDM_FILE_NEW 1
#define IDM_FILE_OPEN 2
#define IDM_FILE_QUIT 3

#define IDM_TREE_CREATE 101
#define IDM_TREE_UPDATE 102
#define IDM_TREE_DELETE 103

#endif // DIATREE_H