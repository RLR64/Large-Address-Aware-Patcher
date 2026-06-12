// STL
#include <filesystem>
#include <string>
#include <vector>

// Windows
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>

// Local
#include "gui.hpp"
#include "patcher.hpp"

namespace gui {

	// Constants
	constexpr int IDC_LISTVIEW          = 101;
	constexpr int IDC_CLEAR_BTN         = 103;
	constexpr int IDC_HINT_LABEL        = 104;
	constexpr int IDC_CHECK_ALL_BTN     = 105;
	constexpr int IDC_UNCHECK_ALL_BTN   = 106;
	constexpr int IDC_APPLY_BTN         = 107;
	constexpr int IDC_CTX_REMOVE        = 201;
	constexpr int IDC_CTX_TOGGLE_BACKUP = 202;

	constexpr int COL_FILENAME = 0;
	constexpr int COL_ARCH     = 1;
	constexpr int COL_STATUS   = 2;
	constexpr int COL_BACKUP   = 3;
	constexpr int COL_DIR      = 4;

	constexpr int TOOLBAR_H = 40;  // height of the top bar
	constexpr int HINT_H    = 24;  // height of the bottom hint label
	constexpr int MARGIN    = 10;

	constexpr int WINDOW_W = 800;
	constexpr int WINDOW_H = 600;

	// State
	static HWND                   g_hWnd      = nullptr;
	static HWND                   g_hList     = nullptr;
	static HWND                   g_hClearBtn = nullptr;
	static HWND                   g_hHint     = nullptr;
	static HFONT                  g_hFont     = nullptr;
	static std::vector<FileEntry> g_entries;

	// Helpers
	static void setListViewItem(int row, int column, const std::wstring &text) {
		LVITEMW lvi{};
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = row;
		lvi.iSubItem = column;
		lvi.pszText  = const_cast<wchar_t *>(text.c_str());

		if (column == 0) {
			ListView_SetItem(g_hList, &lvi);
		} else {
			ListView_SetItemText(g_hList, row, column, const_cast<wchar_t *>(text.c_str()));
		}
	}

	static void updateRowText(int index) {
		const auto &e = g_entries[static_cast<std::size_t>(index)];

		setListViewItem(index, COL_FILENAME, e.filename);
		setListViewItem(index, COL_DIR,      e.directory);
		setListViewItem(index, COL_BACKUP,   e.backup ? L"Yes" : L"No");

		if (e.checkResult != laa::Result::Success) {
			setListViewItem(index, COL_ARCH,   L"—");
			setListViewItem(index, COL_STATUS, laa::resultToString(e.checkResult));
			return;
		}

		setListViewItem(index, COL_ARCH, e.info.is32Bit ? L"32-bit" : L"64-bit");

		// Show patched status if set, otherwise current flag state
		if (!e.statusText.empty()) {
			setListViewItem(index, COL_STATUS, e.statusText);
		} else {
			setListViewItem(index, COL_STATUS, e.info.laaEnabled ? L"Enabled" : L"Disabled");
		}
	}

	static void resizeLayout(int w, int h) {
		// ListView fills the space between toolbar and hint label
		const int listY = TOOLBAR_H;
		const int listH = h - TOOLBAR_H - HINT_H - MARGIN;
		const int listW = w - MARGIN * 2;

		SetWindowPos(g_hList, nullptr,
			MARGIN, listY, listW, listH,
			SWP_NOZORDER);

		// Hint label anchored to bottom
		SetWindowPos(g_hHint, nullptr,
			MARGIN, h - HINT_H - 2, listW, HINT_H,
			SWP_NOZORDER);
	}

	static void addEntry(const std::wstring &path) {
		// Avoid duplicates
		for (const auto& element : g_entries) {
			if (element.path == path) { return; }
		}

		FileEntry entry{};
		entry.path        = path;
		entry.filename    = std::filesystem::path(path).filename().wstring();
		entry.directory   = std::filesystem::path(path).parent_path().wstring();
		entry.backup      = true;
		entry.checkResult = laa::check(path, entry.info);

		const int index = static_cast<int>(g_entries.size());
		g_entries.push_back(entry);

		// Insert row
		LVITEMW lvi{};
		lvi.mask    = LVIF_TEXT;
		lvi.iItem   = index;
		lvi.pszText = const_cast<wchar_t *>(entry.filename.c_str());
		ListView_InsertItem(g_hList, &lvi);

		updateRowText(index);
	}

	static void patchEntry(int index) {
		auto &element = g_entries[static_cast<std::size_t>(index)];

		// Only patch 32-bit executables
		if (element.checkResult != laa::Result::Success || !element.info.is32Bit) { return; }

		const bool enable = !element.info.laaEnabled;
		const bool backup = element.backup;
		const auto result = laa::set(element.path, enable, backup);

		if (result == laa::Result::Success) {
			element.checkResult = laa::check(element.path, element.info);
			element.statusText  = enable ? L"Enabled ✓" : L"Disabled ✓";
		} else {
			element.statusText = laa::resultToString(result);
		}

		updateRowText(index);
	}

	static void clearList() {
		g_entries.clear();
		ListView_DeleteAllItems(g_hList);
	}

	static void setAllChecked(bool checked) {
		const int count = static_cast<int>(g_entries.size());
		for (int i = 0; i < count; ++i) {
			ListView_SetCheckState(g_hList, i, checked ? TRUE : FALSE);
		}
	}

	static void applyChecked() {
		const int count = static_cast<int>(g_entries.size());
		for (int i = 0; i < count; ++i) {
			if (ListView_GetCheckState(g_hList, i) != 0) {
				patchEntry(i);
			}
		}
	}

	static void removeEntry(int index) {
		if (index < 0 || index >= static_cast<int>(g_entries.size())) { return; }

		g_entries.erase(g_entries.begin() + index);
		ListView_DeleteItem(g_hList, index);
	}

	// Subclass proc — lets us catch Delete key inside the ListView
	static WNDPROC g_listViewOrigProc = nullptr;

	static LRESULT CALLBACK listViewSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_KEYDOWN && wParam == VK_DELETE) {
			const int sel = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);
			removeEntry(sel);
			return 0;
		}
		return CallWindowProcW(g_listViewOrigProc, hWnd, msg, wParam, lParam);
	}

	// Window procedure
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {

		case WM_CREATE: {
			g_hWnd = hWnd;

			const auto hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hWnd, GWLP_HINSTANCE));

			// Font — Segoe UI 9pt
			g_hFont = CreateFontW(
				-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
				DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
				L"Segoe UI"
			);

			auto applyFont = [&](HWND h) -> void {
				SendMessage(h, WM_SETFONT, reinterpret_cast<WPARAM>(g_hFont), TRUE);
			};

			auto makeBtn = [&](const wchar_t *label, int x, int id) -> HWND {
				HWND h = CreateWindowExW(
					0, L"BUTTON", label,
					WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					x, 8, 90, 24,
					hWnd, reinterpret_cast<HMENU>(id), hInst, nullptr
				);
				applyFont(h);
				return h;
			};

			makeBtn(L"Check All",   MARGIN,       IDC_CHECK_ALL_BTN);
			makeBtn(L"Uncheck All", MARGIN + 98,  IDC_UNCHECK_ALL_BTN);
			makeBtn(L"Toggle LAA",       MARGIN + 196, IDC_APPLY_BTN);
			g_hClearBtn = makeBtn(L"Clear List",  MARGIN + 294, IDC_CLEAR_BTN);

			// ListView
			g_hList = CreateWindowExW(
				WS_EX_CLIENTEDGE,
				WC_LISTVIEWW, L"",
				WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
				MARGIN, TOOLBAR_H, WINDOW_W - MARGIN * 2, WINDOW_H - TOOLBAR_H - HINT_H - MARGIN,
				hWnd, reinterpret_cast<HMENU>(IDC_LISTVIEW), hInst, nullptr
			);
			applyFont(g_hList);
			ListView_SetExtendedListViewStyle(g_hList,
				LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER | LVS_EX_CHECKBOXES);

			// Subclass ListView to catch Delete key
			g_listViewOrigProc = reinterpret_cast<WNDPROC>(
				SetWindowLongPtrW(g_hList, GWLP_WNDPROC,
					reinterpret_cast<LONG_PTR>(listViewSubclassProc))
			);

			// Columns
			LVCOLUMNW col{};
			col.mask = LVCF_TEXT | LVCF_WIDTH;

			col.pszText = const_cast<wchar_t *>(L"File");
			col.cx      = 180;
			ListView_InsertColumn(g_hList, COL_FILENAME, &col);

			col.pszText = const_cast<wchar_t *>(L"Architecture");
			col.cx      = 90;
			ListView_InsertColumn(g_hList, COL_ARCH, &col);

			col.pszText = const_cast<wchar_t *>(L"LAA Status");
			col.cx      = 120;
			ListView_InsertColumn(g_hList, COL_STATUS, &col);

			col.pszText = const_cast<wchar_t *>(L"Backup");
			col.cx      = 60;
			ListView_InsertColumn(g_hList, COL_BACKUP, &col);

			col.pszText = const_cast<wchar_t *>(L"Directory");
			col.cx      = 340;
			ListView_InsertColumn(g_hList, COL_DIR, &col);

			// Hint label
			g_hHint = CreateWindowExW(
				0, L"STATIC", L"Double-click a row to toggle LAA  |  Drag and drop .exe files here",
				WS_CHILD | WS_VISIBLE | SS_CENTER,
				MARGIN, WINDOW_H - HINT_H - 2, WINDOW_W - MARGIN * 2, HINT_H,
				hWnd, reinterpret_cast<HMENU>(IDC_HINT_LABEL), hInst, nullptr
			);
			applyFont(g_hHint);

			DragAcceptFiles(hWnd, TRUE);
			return 0;
		}

		case WM_DROPFILES: {
			auto *hDrop        = reinterpret_cast<HDROP>(wParam);
			const UINT count   = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);

			for (UINT i = 0; i < count; ++i) {
				const UINT len = DragQueryFileW(hDrop, i, nullptr, 0);
				std::wstring path(len, L'\0');
				DragQueryFileW(hDrop, i, path.data(), len + 1);
				addEntry(path);
			}

			DragFinish(hDrop);
			return 0;
		}

		case WM_NOTIFY: {
			const auto *hdr = reinterpret_cast<NMHDR *>(lParam);

			if (hdr->idFrom == IDC_LISTVIEW && hdr->code == NM_DBLCLK) {
				const auto *nml = reinterpret_cast<NMITEMACTIVATE *>(lParam);
				if (nml->iItem >= 0 && nml->iItem < static_cast<int>(g_entries.size())) {
					patchEntry(nml->iItem);
				}
			}
			return 0;
		}

		case WM_CONTEXTMENU: {
			if (reinterpret_cast<HWND>(wParam) != g_hList) { return 0; }

			const int sel = ListView_GetNextItem(g_hList, -1, LVNI_SELECTED);
			if (sel < 0) { return 0; }

			const bool rowBackup = g_entries[static_cast<std::size_t>(sel)].backup;

			HMENU hMenu = CreatePopupMenu();
			AppendMenuW(hMenu,
				MF_STRING | (rowBackup ? MF_CHECKED : MF_UNCHECKED),
				IDC_CTX_TOGGLE_BACKUP, L"Backup");
			AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
			AppendMenuW(hMenu, MF_STRING, IDC_CTX_REMOVE, L"Remove");

			const int x   = GET_X_LPARAM(lParam);
			const int y   = GET_Y_LPARAM(lParam);
			const int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, x, y, 0, hWnd, nullptr);
			DestroyMenu(hMenu);

			if (cmd == IDC_CTX_TOGGLE_BACKUP) {
				g_entries[static_cast<std::size_t>(sel)].backup ^= true;
				updateRowText(sel);
			} else if (cmd == IDC_CTX_REMOVE) {
				removeEntry(sel);
			}
			return 0;
		}

		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
			case IDC_CHECK_ALL_BTN:   setAllChecked(true);  break;
			case IDC_UNCHECK_ALL_BTN: setAllChecked(false); break;
			case IDC_APPLY_BTN:       applyChecked();       break;
			case IDC_CLEAR_BTN:       clearList();          break;
			default: break;
			}
			return 0;
		}

		case WM_SIZE: {
			if (g_hList != nullptr) {
				resizeLayout(LOWORD(lParam), HIWORD(lParam));
			}
			return 0;
		}

		case WM_DESTROY:
			if (g_hFont != nullptr) { DeleteObject(g_hFont); }
			PostQuitMessage(0);
			return 0;

		default:
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		}
	}

	// Entry point
	auto run(HINSTANCE hInstance, int nCmdShow) -> int {
		INITCOMMONCONTROLSEX icc{};
		icc.dwSize = sizeof(icc);
		icc.dwICC  = ICC_LISTVIEW_CLASSES;
		InitCommonControlsEx(&icc);

		WNDCLASSEXW wc{};
		wc.cbSize        = sizeof(wc);
		wc.style         = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc   = wndProc;
		wc.hInstance     = hInstance;
		wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		wc.lpszClassName = L"LAAWindow";
		wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hIconSm       = LoadIcon(nullptr, IDI_APPLICATION);
		RegisterClassExW(&wc);

		HWND hWnd = CreateWindowExW(
			WS_EX_ACCEPTFILES,
			L"LAAWindow",
			L"Large Address Aware Patcher",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			WINDOW_W, WINDOW_H,
			nullptr, nullptr,
			hInstance, nullptr
		);

		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);

		MSG msg{};
		while (GetMessageW(&msg, nullptr, 0, 0) != 0) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		return static_cast<int>(msg.wParam);
	}

} // namespace gui