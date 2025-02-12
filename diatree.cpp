/*
*       diatree.cpp
*       timwking1
*       9-Feb 2025
*       
*/
#ifndef UNICODE
#define UNICODE
#endif 

#define WIN32_LEAN_AND_MEAN
#define WINVER                          0x0501
#define _WIN32_WINNT                    0x0501

#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <memory>
using namespace std;

//Window procedure definition
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wPaam, LPARAM lParam);

//Context menu
#define WM_CONTEXTMENU                  0x007B
#define IDM_FILE_NEW 1
#define IDM_FILE_OPEN 2
#define IDM_FILE_QUIT 3

//wWinMain entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    //Initialize the window class
    const wchar_t CLASS_NAME[]  = L"Main";
    WNDCLASS wc = { };
    wc.lpfnWndProc      = WindowProc;
    wc.hInstance        = hInstance;
    wc.lpszClassName    = CLASS_NAME;
    RegisterClass(&wc);

    //Create the main window handle hwnd
    HWND hwnd = CreateWindowEx
    (
        0,                      //dwExStyle
        CLASS_NAME,		        //lpClassName
        L"Window Title",	    //lpWindowName
        WS_OVERLAPPEDWINDOW,	//dwStyle
        CW_USEDEFAULT, 		    //X
        CW_USEDEFAULT, 		    //Y
        640, 		            //nWidth
        480,		            //nHeight
        NULL,			        //hWndParent
        NULL,			        //hMenu
        hInstance,		        //hInstance
        NULL			        //lpParam
    );

    if (hwnd == NULL)
    {
        return 0;
    }
    ShowWindow(hwnd, nCmdShow);

    //Initialzie the message loop
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

//Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //Definitions for window ui elements
    static HWND hTreeView;
    static HMENU hMenu;
    POINT point;

    //Message loop cases
    switch(uMsg)
    {
        //When the Window is created we initialize the TreeView
        case WM_CREATE:
        {
            //Initialize common controls
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icex.dwICC = ICC_TREEVIEW_CLASSES;
            InitCommonControlsEx(&icex);

            //Treeview Control
            hTreeView = CreateWindowEx
            (0, WC_TREEVIEW, L"Tree View",
            WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES,
            10, 10, 120, 400,
            hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            LPNMTREEVIEW lpnmtv;

            //Populate Treeview
            TVINSERTSTRUCT tvinsert;    //Insert struct
            HTREEITEM hRoot;            //Root item of the tree
            
            tvinsert.hParent = NULL;
            tvinsert.hInsertAfter = TVI_ROOT;
            tvinsert.item.mask = TVIF_TEXT;
            tvinsert.item.pszText = TEXT("Root Item");
        
            hRoot = TreeView_InsertItem(hTreeView, &tvinsert);
        
            tvinsert.hParent = hRoot;
            tvinsert.hInsertAfter = TVI_LAST;
            tvinsert.item.pszText = TEXT("Child Item");
        
            TreeView_InsertItem(hTreeView, & tvinsert);


            break;
        }
        //When comands are received (ex. by context menu or menubar buttons)
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDM_FILE_NEW:
                case IDM_FILE_OPEN:
                {
                    MessageBeep(MB_ICONINFORMATION);
                    break;
                }
                case IDM_FILE_QUIT:
                {
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
                }
            }
            break;
        }

        //When the right button is clicked (context menu opens)
        case WM_RBUTTONUP:
        {
            point.x = LOWORD(lParam);
            point.y = HIWORD(lParam);

            hMenu = CreatePopupMenu();
            ClientToScreen(hwnd, &point);

            AppendMenuW(hMenu, MF_STRING, IDM_FILE_NEW, L"&New");
            AppendMenuW(hMenu, MF_STRING, IDM_FILE_OPEN, L"&Open");
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, IDM_FILE_QUIT, L"&Quit");

            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
            break;
        }
        
        //When the TreeView is updated
        case WM_NOTIFY:
        {
            if(((LPNMHDR)lParam)->hwndFrom == hTreeView)
            {
                switch (((LPNMHDR)lParam)->code)
                {
                    case TVN_SELCHANGED:
                    {
                        LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;
                        HTREEITEM hSelectedItem = lpnmtv->itemNew.hItem;

                        //MessageBox(hwnd, L"TreeView selection changed", L"Notification", MB_OK);
                        break;
                    }
                    case TVN_ITEMEXPANDING:
                    {
                        LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;
                        HTREEITEM hItemExpanding = lpnmtv->itemNew.hItem;

                        //MessageBox(hwnd, L"Treeview item expanding", L"Notification", MB_OK);
                        break;
                    }
                    case TVN_ITEMEXPANDED:
                    {
                        LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;
                        HTREEITEM hItemExpanded = lpnmtv->itemNew.hItem;

                        //MessageBox(hwnd, L"Treeview item expanded", L"Notification", MB_OK);
                        break;
                    }
                }
            }
            break;
        }

        //Destroy our window when we are done with it
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }

        //Paint our window
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
            EndPaint(hwnd, &ps);
        }
        
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

struct DiaChoice
{
    public:
        std::wstring text;
        int nextWindow;
};

struct DiaWindow
{
    enum WindowSize 
    {
        small, 
        medium, 
        large
    };

    public:
        int id;
        std::wstring text;
        WindowSize windowSize;
        bool isChoice;
        std::vector<DiaChoice> choices;
};

class DiaConversation
{
    public:
        int id;
        std::wstring title;
        std::vector<DiaWindow> windows;

        void AddWindow(int windowID, const std::wstring& text, bool isChoice)
        {
            windows.push_back({windowID, text, isChoice, {}});
        }

        void DeleteWindow(int windowID)
        {
            windows.erase(std::remove_if(windows.begin(), windows.end(), [windowID](const DiaWindow& win) {return win.id == windowID; }), windows.end());
        }
};

class TreeNode
{
    public:
        std::wstring text;
        std::vector<std::shared_ptr<TreeNode>> children;
        //Constructor
        TreeNode(const std::wstring&text) : text(text) {}

        void addChild(std::shared_ptr<TreeNode> child)
        {
            children.push_back(child);
        }
};

std::shared_ptr<TreeNode> ConversationToTreeNode(const DiaConversation& conversation)
{
    auto rootNode = std::make_shared<TreeNode>(conversation.title);

    for(const auto& window : conversation.windows)
    {
        auto windowNode = std::make_shared<TreeNode>(L"Window " + std::to_wstring(window.id) + L": " + window.text);

        if(window.isChoice)
        {
            for (const auto& choice : window.choices)
            {
                auto choiceNode = std::make_shared<TreeNode>(L"Choice: " + choice.text);
                windowNode->addChild(choiceNode);
            }
        }

        rootNode->addChild(windowNode);
    }
    return rootNode;
}

//Function to insert a TreeNode into the TreeView
HTREEITEM InsertTreeNode(HWND hTreeView, HTREEITEM hParent, std::shared_ptr<TreeNode> node)
{
    TVINSERTSTRUCT tvinsert = {};
    tvinsert.hParent = hParent;
    tvinsert.hInsertAfter = TVI_LAST;
    tvinsert.item.mask = TVIF_TEXT;

    LPWSTR stringText = const_cast<LPWSTR>(node->text.c_str());
    tvinsert.item.pszText = stringText;

    HTREEITEM hItem = TreeView_InsertItem(hTreeView, &tvinsert);

    for (auto& child : node-> children)
    {
        InsertTreeNode(hTreeView, hItem, child);
    }

    return hItem;
}