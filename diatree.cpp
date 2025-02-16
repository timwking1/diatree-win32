/*
*       diatree.cpp
*       timwking1
*       9-Feb 2025
*/
#define WIN32_LEAN_AND_MEAN
#define WINVER                          0x0501
#define _WIN32_WINNT                    0x0501

#include <windows.h>
#include <nlohmann/json.hpp>
#include "diatree.h"

//Convert wide (Unicode) strings to narrow strings...
std::string WideStringToMultiString(const std::wstring& wstr)
{
    int bufSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string convertedString(bufSize, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &convertedString[0], bufSize, NULL, NULL);
    return convertedString;
}

struct DiaChoice
{
    public:
        std::wstring text;
        int nextWindow;

        json to_json() const 
        {
            return 
            {
                {"text", text}, 
                {"nextWindow", nextWindow}
            };
        }
};

void from_json(const json& j, DiaChoice& choice)
{
    j.at("text").get_to(choice.text);
    j.at("nextWindow").get_to(choice.nextWindow);
}

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

        json to_json() const
        {
            return
            {
                {"id", id},
                {"text", text},
                {"windowSize", static_cast<int>(windowSize)},
                {"isChoice", isChoice},
                {"choices", json::array()}
            };
        }
};

void from_json(const json& j, DiaWindow& window)
{
    j.at("id").get_to(window.id);
    j.at("text").get_to(window.text);
    int size;
    j.at("windowSize").get_to(size);
    window.windowSize = static_cast<DiaWindow::WindowSize>(size);
    j.at("isChoice").get_to(window.isChoice);
    j.at("choices").get_to(window.choices);
}

class DiaConversation
{
    public:
        int id;
        std::wstring title;
        std::vector<DiaWindow> windows;

        void AddWindow(int windowID, const std::wstring& text, bool isChoice)
        {
            windows.push_back({windowID, text, DiaWindow::WindowSize::medium, isChoice, {}});
        }

        void DeleteWindow(int windowID)
        {
            auto it = std::remove_if(windows.begin(), windows.end(),
            [=](const DiaWindow& win) { return win.id == windowID; });

            windows.erase(it, windows.end()); //Erase the removed elements
        }

        json to_json() const
        {
            json j;
            j["id"] = id;
            j["title"] = title;
            j["windows"] = json::array();
            for(const auto& window : windows)
            {
                j["windows"].push_back(window.to_json());
            }
            return j;
        }

        void SaveToFile(const std::string& filename) const
        {
            std::ofstream file(filename);
            if(file.is_open())
            {
                file << to_json().dump(4);
                file.close();
            }
        }
};

void from_json(const json& j, DiaConversation& convo)
{
    j.at("id").get_to(convo.id);
    j.at("title").get_to(convo.title);
    j.at("windows").get_to(convo.windows);
}

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

bool LoadConversationFromFile(const std::string& filename, DiaConversation& conversation)
{
    std::ifstream file(filename);
    if(!file.is_open())
    {
        return false;
    }
    json j;
    file >> j;
    conversation = j.get<DiaConversation>();
    return true;
}

void UpdateTreeView(HWND hTreeView, const DiaConversation& conversation)
{
    TreeView_DeleteAllItems(hTreeView);

    //Root item
    TVINSERTSTRUCT tvinsert = {};
    tvinsert.hParent = NULL;
    tvinsert.hInsertAfter = TVI_ROOT;
    tvinsert.item.mask = TVIF_TEXT;
    tvinsert.item.pszText = const_cast<LPWSTR>(conversation.title.c_str());
    HTREEITEM hRoot = TreeView_InsertItem(hTreeView, &tvinsert);

    //Child Items
    for (const auto& window : conversation.windows)
    {
        tvinsert.hParent = hRoot;
        tvinsert.hInsertAfter = TVI_LAST;
        tvinsert.item.pszText = const_cast<LPWSTR>(window.text.c_str());
        HTREEITEM hWindow = TreeView_InsertItem(hTreeView, &tvinsert);

            //Sub Items
        for (const auto& choice : window.choices)
        {
            tvinsert.hParent = hWindow;
            tvinsert.hInsertAfter = TVI_LAST;
            tvinsert.item.pszText = const_cast<LPWSTR>(choice.text.c_str());
            TreeView_InsertItem(hTreeView, &tvinsert);
        }
    }

    TreeView_Expand(hTreeView, hRoot, TVE_EXPAND);
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

/*==================================================
*
*       wWinMain entry point
*
*==================================================*/

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

/*
*   Context Menus
*/
void ShowFileContextMenu(HWND hWnd, POINT pt)
{
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;

    AppendMenu(hMenu, MF_STRING, IDM_FILE_NEW, L"New");
    AppendMenu(hMenu, MF_STRING, IDM_FILE_OPEN, L"Open");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, IDM_FILE_QUIT, L"Quit");

    // Display menu at cursor position
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
    DestroyMenu(hMenu);
}

void ShowTreeContextMenu(HWND hWnd, POINT pt, HTREEITEM hItem)
{
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;

    AppendMenu(hMenu, MF_STRING, IDM_TREE_CREATE, L"Create");
    AppendMenu(hMenu, MF_STRING, IDM_TREE_UPDATE, L"Update");
    AppendMenu(hMenu, MF_STRING, IDM_TREE_DELETE, L"Delete");

    // Display menu at cursor position
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
    DestroyMenu(hMenu);
}


std::string OpenFileDialog(HWND hwnd) 
{
    OPENFILENAME ofn;         // common dialog box structure
    WCHAR szFile[260];        // buffer for file name
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
    ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialog box. 
    if (GetOpenFileName(&ofn) == TRUE) {
        return WideStringToMultiString(ofn.lpstrFile);
    } 
    else 
    {
        return "";
    }
}


/*==================================================
*
*       Window Procedure
*
*==================================================*/
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //Definitions for window ui elements
    static HWND hTreeView;

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

            std::unique_ptr<TreeNode> RootNode = std::make_unique<TreeNode>(L"ROOT_CONVO"); //Create new instance for root
            InsertTreeNode(hTreeView, NULL, std::move(RootNode));                           //Insert the instance
        




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
                //File operation commands
                case IDM_FILE_NEW:
                case IDM_FILE_OPEN:
                {
                    std::string filename = OpenFileDialog(hwnd);
                    if (!filename.empty())
                    {
                        DiaConversation conversation;
                        if (LoadConversationFromFile(filename, conversation))
                        {
                            UpdateTreeView(hTreeView, conversation);
                        }
                        else
                        {
                            MessageBox(hwnd, L"Failed to load the conversation.", L"Error", MB_OK | MB_ICONERROR);
                        }
                    }
                    break;
                }
                case IDM_FILE_QUIT:
                {
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
                }
                //TreeView commands
                case IDM_TREE_CREATE:
                {
                    
                }
                case IDM_TREE_UPDATE:
                case IDM_TREE_DELETE:
                {
                    break;
                }
            }
            break;
        }

        case WM_CONTEXTMENU:
        {
            HWND hWndContext = (HWND)wParam; // Handle of the clicked window

            // Get cursor position
            POINT pt;
            GetCursorPos(&pt);
        
            if (hWndContext == hTreeView) // If it's the TreeView
            {
                // Determine which TreeView item was clicked
                TVHITTESTINFO ht = {};
                ht.pt = pt;
                ScreenToClient(hTreeView, &ht.pt);
                HTREEITEM hItem = TreeView_HitTest(hTreeView, &ht);
        
                if (hItem)
                {
                    TreeView_SelectItem(hTreeView, hItem);
                    ShowTreeContextMenu(hwnd, pt, hItem);
                }
            }
            else // If it's another context (e.g., global file menu)
            {
                ShowFileContextMenu(hwnd, pt);
            }
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