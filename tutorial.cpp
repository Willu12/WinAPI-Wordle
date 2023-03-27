#include "framework.h"
#include "tutorial.h"
#include <string>
#include <time.h>
#include <fstream>
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <algorithm> 

#include <iostream>
#include <map>
#pragma comment (lib,"Gdiplus.lib")
using namespace Gdiplus;


#define MAX_LOADSTRING 100
#define ANIMATION_WAITING 1
#define ANIMATION_IN_PROGGRES 0
#define ANIMATION_END -1
#define ANIMATION_TIME 180

HINSTANCE hInst;
WCHAR szTitleK[MAX_LOADSTRING] = L"Wordle - Keyboard";
WCHAR szTitleP[MAX_LOADSTRING] = L"Wordle - Puzzle";

WCHAR szWindowClass[MAX_LOADSTRING] = L"NAZWA";
WCHAR szWindowClassChild[MAX_LOADSTRING] = L"KLASABb";
WCHAR szWindowClassOverlay[MAX_LOADSTRING] = L"overlay";
DWORD start;
HFONT hFont = CreateFont(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
	OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
	DEFAULT_PITCH | FF_SWISS, L"Arial"); // Create a font with size 26

HFONT hFont_END = CreateFont(40, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
	OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
	DEFAULT_PITCH | FF_SWISS, L"Arial"); // Create a font with size 40
HBRUSH Yellow = CreateSolidBrush(RGB(243, 194, 55));
HBRUSH Green = CreateSolidBrush(RGB(121, 184, 81));
HBRUSH White = CreateSolidBrush(RGB(251, 252, 255));
HBRUSH Gray = CreateSolidBrush(RGB(164, 174, 196));

HPEN penGreen = CreatePen(PS_SOLID, 1, RGB(121, 184, 81));
HPEN penYellow = CreatePen(PS_SOLID, 1, RGB(243, 194, 55));
HPEN penGray = CreatePen(PS_SOLID, 1, RGB(164, 174, 196));
HPEN penWhite = CreatePen(PS_SOLID, 1, (RGB(251, 252, 255)));

struct window
{
	int color = 0;
	WCHAR lett =' ';
};

int overlay_percent;
std::wstring possible_letters = L"QWERTYUIOPASDFGHJKLZXCVBNM";

void clearGame();
void PrintBoard(int k);
void colorBoard(int k);
bool isInside(const std::wstring& str, WCHAR c);
HWND hWndM;
HWND hWndBoards[4] = {nullptr};
HWND hWndOverlays[4] = { nullptr };
LPCWSTR  Letters = L"QWERTYUIOPASDFGHJKLZXCVBNM";

int board_count;
int curr_letter = 0;
int word = 0;
int words_count = 6;

int Animation_state = ANIMATION_END;


bool is_correct[4] = { true };

WCHAR check_word[5];
WCHAR curr_word[5];
LPCWSTR correct_words[4] = { L"MICHA",L"TRZON",L"BRZEG",L"TREFL" };
int key_colors[4][26];
window windows[4][50];
int boards[4]{ 0 };
std::map<WCHAR, int> map_letters;


HBITMAP offBitmap[4];
HBITMAP offOldBitmap[4];
HDC offDC[4];

WCHAR DIFF[10];
DWORD HARD = MF_UNCHECKED;
DWORD MEDIUM = MF_UNCHECKED;
DWORD EASY = MF_CHECKED;
ATOM MyRegisterClass(HINSTANCE hInstance);
ATOM MyRegisterClassChild(HINSTANCE hInstance);

HBRUSH SelectCorrectBrush(HDC hdc, int color);
HPEN SelectCorrectPen(HDC hdc, int color);

void Generate_Words();

BOOL InitInstance(HINSTANCE, int);


BOOL InitInstanceChild(HINSTANCE, int);


//myszka :)
void GetTextInfoForMouseMsg(WPARAM wParam, LPARAM lParam, const TCHAR* msgName, TCHAR* buf, int bufSize);
void GetTextInfoForMouseMsg(HWND hWnd, WPARAM wParam, LPARAM lParam, const TCHAR* msgName, TCHAR* buf, int bufSize);
void GetTextInfoForKeyMsg(WPARAM wParam, const TCHAR* msgName, TCHAR* buf, int bufSize);
RECT GetLocalCoordinates(HWND hWnd);
void PrintKeyboardMain();
bool comparei(std::wstring stringA, std::wstring stringB);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND,UINT,WPARAM,LPARAM);

int start_anim_time;

std::wstring c_word1[4];

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	//te parametry moga nie zostac uzyte
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	srand(time(NULL));

	 GetPrivateProfileStringW(
		L"Wordle",
		L"DIFF",
		L"EASY" ,
		DIFF,
		10,
		L"./Wordle.ini"
	);

	 for (int i = 0; i < 26; i++)
	 {
		 map_letters[Letters[i]] = i;
	 }

	Generate_Words();

	MyRegisterClass(hInstance);
	MyRegisterClassChild(hInstance);

	
	if ( !InitInstance(hInstance, nCmdShow) )
	{
		return FALSE;
	}
	if (!InitInstanceChild(hInstance, nCmdShow))
	{
		return FALSE;
	}


	
	HACCEL hAccelTable = LoadAccelerators(hInstance,nullptr);

	//jakas struktura
	MSG msg;

	//glowna petla PROGRAMU
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	GdiplusShutdown(gdiplusToken);
	return static_cast<int>(msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	//kolejna sturktura
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW| CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor = nullptr;
	wcex.hbrBackground = CreateSolidBrush(RGB(255 ,255, 255));	
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDR_MENU1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	return RegisterClassExW(&wcex);
}

ATOM MyRegisterClassChild(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = nullptr;
	wcex.hCursor = nullptr;
	wcex.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szWindowClassChild;
	wcex.hIconSm = nullptr;
	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	int w = GetSystemMetrics(SM_CXFULLSCREEN);
	int h = GetSystemMetrics(SM_CYFULLSCREEN);

	int sizex = 634;
	int sizey =260;

	hWndM = CreateWindow(szWindowClass, szTitleK, WS_OVERLAPPEDWINDOW | WS_VISIBLE, (w-sizex)/2 , h/4 + 460, sizex, sizey, NULL, NULL, hInstance, NULL);
	if (!hWndM) return FALSE;
	
	SetWindowLong(hWndM, GWL_EXSTYLE, GetWindowLong(hWndM, GWL_EXSTYLE) | WS_EX_LAYERED);


	LONG lStyle = GetWindowLong(hWndM, GWL_STYLE);
	lStyle &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX);
	SetWindowLong(hWndM, GWL_STYLE, lStyle);
	ShowWindow(hWndM, nCmdShow);
	UpdateWindow(hWndM);
	HMENU Menu = GetMenu(hWndM);

	if (lstrcmpW(DIFF,L"EASY") == 0)
	{
		HARD = CheckMenuItem(Menu, ID_DIFFICULTY_HARD, MF_UNCHECKED);
		MEDIUM = CheckMenuItem(Menu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
		EASY = CheckMenuItem(Menu, ID_DIFFICULTY_EASY, MF_CHECKED);
		board_count = 1;
		words_count = 6;
		is_correct[0] = false;
		is_correct[1] = is_correct[2] = is_correct[3] = true;
	}
	else if (lstrcmpW(DIFF, L"MEDIUM") == 0)
	{
		HARD = CheckMenuItem(Menu, ID_DIFFICULTY_HARD, MF_UNCHECKED);
		MEDIUM = CheckMenuItem(Menu, ID_DIFFICULTY_MEDIUM, MF_CHECKED);
		EASY = CheckMenuItem(Menu, ID_DIFFICULTY_EASY, MF_UNCHECKED);
		board_count = 2;
		words_count = 8;

		is_correct[0] = is_correct[1] = false;
		is_correct[3] = is_correct[2] = true;
	}
	else
	{
		HARD = CheckMenuItem(Menu, ID_DIFFICULTY_HARD, MF_CHECKED);
		MEDIUM = CheckMenuItem(Menu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
		EASY = CheckMenuItem(Menu, ID_DIFFICULTY_EASY, MF_UNCHECKED);
		board_count = 4;
		words_count = 10;
		is_correct[0] = is_correct[1] = is_correct[2] = is_correct[3] = false;
	}
	return true;
}
BOOL InitInstanceChild(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	word = curr_letter = 0;
	int w = GetSystemMetrics(SM_CXFULLSCREEN);
	int h = GetSystemMetrics(SM_CYFULLSCREEN);

	int size = 55;

	int sizex = 55 * 5 + 6 * 9;


	if (board_count == 1)
	{
		int sizey = 55 * 6 + 6 * 14;
		hWndBoards[0] = CreateWindow(szWindowClassChild, szTitleP,  WS_VISIBLE | WS_POPUP | WS_CAPTION, (w-sizex)/2, (h-sizey)/2, sizex, sizey, hWndM, NULL, hInstance, NULL);
		if (!hWndBoards[0]) return FALSE;
		ShowWindow(hWndBoards[0], nCmdShow);
		UpdateWindow(hWndBoards[0]);
	}
	else if (board_count == 2)
	{
		int sizey = 55 * 8 + 6 * 16;
		hWndBoards[0] = CreateWindow(szWindowClassChild, szTitleP, WS_VISIBLE | WS_POPUP | WS_CAPTION, (w - sizex)* 1/ 4  - 70, (h - sizey) / 2, sizex, sizey, hWndM, NULL, hInstance, NULL);
		if (!hWndBoards[0]) return FALSE;

		hWndBoards[1] = CreateWindow(szWindowClassChild, szTitleP, WS_VISIBLE | WS_POPUP | WS_CAPTION, (w - sizex) * 3/ 4 + 70, (h - sizey) / 2, sizex, sizey, hWndM, NULL, hInstance, NULL);
		if (!hWndBoards[1]) return FALSE;

		ShowWindow(hWndBoards[0], nCmdShow);
		UpdateWindow(hWndBoards[0]);
		ShowWindow(hWndBoards[1], nCmdShow);
		UpdateWindow(hWndBoards[1]);

	}
	else
	{
		int sizey = 55 * 10 + 6 * 18;
		hWndBoards[0] = CreateWindow(szWindowClassChild, szTitleP, WS_VISIBLE | WS_POPUP | WS_CAPTION, (w - sizex) / 4 - 100, 0, sizex, sizey, hWndM, NULL, hInstance, NULL);
		if (!hWndBoards[0]) return FALSE;

		hWndBoards[1] = CreateWindow(szWindowClassChild, szTitleP, WS_VISIBLE | WS_POPUP | WS_CAPTION, (w - sizex) / 4 -100, h/2 , sizex, sizey, hWndM, NULL, hInstance, NULL);
		if (!hWndBoards[1]) return FALSE;

		hWndBoards[2] = CreateWindow(szWindowClassChild, szTitleP, WS_VISIBLE | WS_POPUP | WS_CAPTION, (w - sizex) *3 / 4 + 100, 0, sizex, sizey, hWndM, NULL, hInstance, NULL);
		if (!hWndBoards[2]) return FALSE;

		hWndBoards[3] = CreateWindow(szWindowClassChild, szTitleP, WS_VISIBLE | WS_POPUP | WS_CAPTION, (w - sizex)*3 / 4 +100, h/2 , sizex, sizey, hWndM, NULL, hInstance, NULL);
		if (!hWndBoards[3]) return FALSE;

		ShowWindow(hWndBoards[0], nCmdShow);
		UpdateWindow(hWndBoards[0]);
		ShowWindow(hWndBoards[1], nCmdShow);
		UpdateWindow(hWndBoards[1]);
		ShowWindow(hWndBoards[2], nCmdShow);
		UpdateWindow(hWndBoards[2]);
		ShowWindow(hWndBoards[3], nCmdShow);
		UpdateWindow(hWndBoards[3]);
	}
	
	return true;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int minSize = 200;
	const int maxSize = 400;
	static int stepSize = 10;
	const int bufSize = 256;
	TCHAR buf[bufSize];
	static HCURSOR cursor = NULL;
	int count = 0;
	HMENU Menu = GetMenu(hWndM);
	SetLayeredWindowAttributes(hWndM, 0, (255 * 60) / 100, LWA_ALPHA);


	switch (message)
	{
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			switch (wmId)
			{
				//https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-checkmenuitem
				case ID_DIFFICULTY_EASY:
				{
					clearGame();
					HARD = CheckMenuItem(Menu, ID_DIFFICULTY_HARD, MF_UNCHECKED);
					MEDIUM = CheckMenuItem(Menu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
					EASY = CheckMenuItem(Menu, ID_DIFFICULTY_EASY, MF_CHECKED);
					for (int i = 0; i < 4; i++)
					{
						DestroyWindow(hWndBoards[i]);

					}
					words_count = 6;
					board_count = 1;
					InitInstanceChild(hInst, 1);
					InvalidateRect(
						hWndM,
						NULL,
						TRUE
					);

					break;
				}
				case ID_DIFFICULTY_MEDIUM:
				{
					clearGame();
					HARD = CheckMenuItem(Menu, ID_DIFFICULTY_HARD, MF_UNCHECKED);
					MEDIUM = CheckMenuItem(Menu, ID_DIFFICULTY_MEDIUM, MF_CHECKED);
					EASY = CheckMenuItem(Menu, ID_DIFFICULTY_EASY, MF_UNCHECKED);
					words_count = 8;
					board_count = 2;
					for (int i = 0; i < 4; i++)
					{
						DestroyWindow(hWndBoards[i]);

					}
					InitInstanceChild(hInst, 1);
					InvalidateRect(
						hWndM,
						NULL,
						TRUE
					);
					break;
				}
				case ID_DIFFICULTY_HARD:
				{
					clearGame();
					HARD = CheckMenuItem(Menu, ID_DIFFICULTY_HARD, MF_CHECKED);
					MEDIUM = CheckMenuItem(Menu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
					EASY = CheckMenuItem(Menu, ID_DIFFICULTY_EASY, MF_UNCHECKED);
					for (int i = 0; i < 4; i++)
					{
						DestroyWindow(hWndBoards[i]);
					}
					words_count = 10;
					board_count = 4;
					InitInstanceChild(hInst, 1);
					InvalidateRect(
						hWndM,
						NULL,
						TRUE
					);

					break;
				}
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		}
		case WM_CREATE:
		{
			if (hWnd == hWndBoards[0])
			{
				RECT rect;
				GetClientRect(hWnd, &rect);

				HDC hdc = GetDC(hWnd);
				offDC[0] = CreateCompatibleDC(hdc);
				offBitmap[0] = CreateCompatibleBitmap(hdc, 1000,1000);
				ReleaseDC(hWnd, hdc);

			}
			else if (hWnd == hWndBoards[1])
			{
				RECT rect;
				GetClientRect(hWnd, &rect);
				HDC hdc = GetDC(hWnd);
				offDC[1] = CreateCompatibleDC(hdc);
				offBitmap[1] = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);

				ReleaseDC(hWnd, hdc);
			}
			else if (hWnd == hWndBoards[2])
			{
				RECT rect;
				GetClientRect(hWnd, &rect);
				HDC hdc = GetDC(hWnd);
				offDC[2] = CreateCompatibleDC(hdc);
				offBitmap[1] = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);

				ReleaseDC(hWnd, hdc);
			}
			else if (hWnd == hWndBoards[3])
			{

				RECT rect;
				GetClientRect(hWnd, &rect);
				HDC hdc = GetDC(hWnd);
				offDC[3] = CreateCompatibleDC(hdc);
				offBitmap[1] = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);

				ReleaseDC(hWnd, hdc);
			}
			start = GetTickCount();
			SetTimer(hWndM, 7, 10, NULL);
			break;
		}

		case WM_PAINT:
		{
			if(hWnd == hWndM)	PrintKeyboardMain();
			else if(hWnd == hWndBoards[0])
			{

				if(boards[0] != 0)
				InvalidateRect(
					hWndBoards[0],
					NULL,
					TRUE
				);
				PrintBoard(0);
				colorBoard(0);

			}
			else if (hWnd == hWndBoards[1])
			{

				if (boards[1] != 0)
					InvalidateRect(
						hWndBoards[1],
						NULL,
						TRUE
					);
				PrintBoard(1);
				colorBoard(1);


			}
			else if (hWnd == hWndBoards[2])
			{
				if (boards[2] != 0)
					InvalidateRect(
						hWndBoards[2],
						NULL,
						TRUE
					);
				PrintBoard(2);
				colorBoard(2);


			}
			else if (hWnd == hWndBoards[3])
			{
				if (boards[3] != 0)
					InvalidateRect(
						hWndBoards[3],
						NULL,
						TRUE
					);
				PrintBoard(3);
				colorBoard(3);


			}
			break;
		}
		case WM_SIZE:
		{
			// get the size of the client area
			int clientWidth = LOWORD(lParam);	
			int clientHeight = HIWORD(lParam);
			break;
		}
		case WM_CHAR:
		{
			if (hWnd != hWndM) break;
			char c = (TCHAR)wParam;
			std::wstring s;
			std::wstring curr_word_wstring(curr_word);
			switch (c)
			{
				case 0x0D:
				{
					InvalidateRect(
						hWndM,
						NULL,
						FALSE
					);
					if (curr_letter < 5)	break;

					//sprawdzenie czt takie slowo istenieje
					std::wifstream file("./Wordle.txt");
					while (std::getline(file, s))
					{
						if(comparei(s, curr_word_wstring)) break;
					}
					
					curr_letter = 0;
					if (file.eof())
					{
						for (int i = 0; i < board_count; i++)
						{
							for (int j = 0; j < 5; j++)
							{
								windows[i][j + 5 * word].lett = ' ';
								curr_word[j] = ' ';
							}
							if(is_correct[i] == false)
							InvalidateRect(
								hWndBoards[i],
								NULL,
								FALSE
							);
						}
					}
					else
					{
						//kolorujemy odpowiednie  klocki
						for (int k = 0; k < board_count; k++)
						{
							if (is_correct[k]) continue;
							for (int i = 0; i < 5; i++)
							{
								if (windows[k][i + 5 * word].lett == correct_words[k][i])
								{
									windows[k][i + 5 * word].color = 1;
									key_colors[k][map_letters[windows[k][i + 5 * word].lett]] = 1;
								}
								else
								{
									int j = 0;
									//szukamy czy na innej pozycji jest ta literka
									for (j = 0; j < 5; j++)
									{
										if (windows[k][i + 5 * word].lett == correct_words[k][j])
										{
											windows[k][i + 5 * word].color = 2;
											key_colors[k][map_letters[windows[k][i + 5 * word].lett]] = 2;

											break;
										}
									}
									if (j == 5)
									{
										windows[k][i + 5 * word].color = -1;
										key_colors[k][map_letters[windows[k][i + 5 * word].lett]] = -1;
									}
								}
							}
						}
						start_anim_time = GetTickCount();
							Animation_state = ANIMATION_WAITING;
						for (int i = 0; i < 5; i++)
						{
							check_word[i] = curr_word[i];
						}
						word++;


						
					}
					break;

				}
				case VK_BACK:
				{
					if (curr_letter == 0)	break;

					curr_letter--;
					for (int i = 0; i < board_count; i++)
					{
						if (is_correct[i] == true) continue;
						windows[i][curr_letter + 5 * word].lett = ' ';
						InvalidateRect(
							hWndBoards[i],
							NULL,
							FALSE
						);

					}

					curr_word[curr_letter] = ' ';
					break;
				}
				default:
				{
					std::wstring s(1, c);
					std::transform(s.begin(), s.end(), s.begin(), ::toupper);
					if (isInside(Letters, s[0]) == false) break;

					if (curr_letter == 5)
					{
						break;
					}
					for (int i = 0; i < board_count; i++)
					{
						if (is_correct[i] == true) continue;
						windows[i][curr_letter + 5 * word].lett = s[0];
						InvalidateRect(
							hWndBoards[i],
							NULL,
							FALSE
						);
					}
					curr_word[curr_letter++] = s[0];

					RECT rc;
					break;
				}
			}
			break;
		}
		case WM_TIMER:
		{
			if (wParam == 7) // check timer id
			{
				DWORD time = (GetTickCount() - start)/1000;
				wchar_t s[256];
			    swprintf_s(s, 256,(L" Wordle - Keyboard  [%d.%5d]"),time, GetTickCount() - start - time * 1000);
			    SetWindowText(hWndM, s);

				if (Animation_state ==  ANIMATION_WAITING)
				{
					Animation_state = ANIMATION_IN_PROGGRES;
				}
				else if (Animation_state == ANIMATION_IN_PROGGRES)
				{
					RECT rect;
					rect.left = 0;
					rect.top = (word - 1) * 61 + 6;
					rect.right = rect.left + 5 * 61 + 6 ;
					rect.bottom = rect.top + 55;
					
					if ((int)GetTickCount()  -start_anim_time - 10> ANIMATION_TIME * 5 )
					{
						Animation_state = ANIMATION_END;
						for (int i = 0; i < 4; i++)
						{
							if (is_correct[i]) continue;
							if (lstrcmpW(correct_words[i], check_word) == 0)
							{
								is_correct[i] = true;
								boards[i] = 1;
								InvalidateRect(
									hWndBoards[i],
									NULL,
									FALSE
								);
							}

							else if (word >= words_count)
							{
							//dla kazdego nie wygrabego

								if (is_correct[i]) continue;
								
								boards[i] = -1;
								
								InvalidateRect(
									hWndBoards[i],
									NULL,
									FALSE
								);
							}
						}
						
					}
					else
					{
						for (int i = 0; i < board_count; i++)
						{
							if (is_correct[i]) continue;

							InvalidateRect(hWndBoards[i], &rect, FALSE);
						}
					}
				
				}

			}
			break;
		}
		case WM_NCHITTEST:
		{
			if (hWnd == hWndOverlays[0] || hWnd == hWndOverlays[1] || hWnd == hWndOverlays[2] || hWnd == hWndOverlays[3]) break;
			else if (hWnd != hWndM)	return HTCAPTION;
			else
			{
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}


		case WM_DESTROY:
			if (hWnd == hWndM)
			{
				LPCWSTR s;
				if (board_count == 1)
				{
					s = L"EASY";
				}
				else if (board_count == 2)
				{
					s = L"MEDIUM";

				}
				else
				{
					s = L"HARD";

				}
				WritePrivateProfileStringW(
					L"WORDLE",
					L"DIFF",
					s,
					L"./Wordle.ini"
				);

				PostQuitMessage(0);
			}


			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
			return static_cast<INT_PTR>(TRUE);
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return static_cast<INT_PTR>(TRUE);
			}
			break;
	}
	return static_cast <INT_PTR>(FALSE);
}
void GetTextInfoForKeyMsg(WPARAM wParam, const TCHAR* msgName,TCHAR * buf, int bufSize)
{
	static int counter = 0;
	counter++;
	_stprintf_s(buf, bufSize, _T("%s key: %d ( counter : %d)"), msgName, wParam, counter);
}

//https://stackoverflow.com/questions/18034975/how-do-i-find-position-of-a-win32-control-window-relative-to-its-parent-window
RECT GetLocalCoordinates(HWND hWnd) 
{
	RECT Rect;
	GetWindowRect(hWnd, &Rect);
	MapWindowPoints(NULL, GetParent(hWnd), (LPPOINT)&Rect, 2);
	return Rect;
}

bool comparei(std::wstring stringA, std::wstring stringB)
{
	transform(stringA.begin(), stringA.end(), stringA.begin(), toupper);
	transform(stringB.begin(), stringB.end(), stringB.begin(), toupper);
	return (stringA == stringB);
}

//https://stackoverflow.com/questions/1974015/how-to-use-drawtext-to-write-text-in-a-given-window-whose-handle-is-known
void PrintKeyboardMain()
{
	PAINTSTRUCT ps;
	RECT rect;
	HDC hdc = BeginPaint(hWndM, &ps);

	for (int i = 0; i < 26; i++)
	{
		std::wstring s(1, Letters[i]);
		int x;
		int y;
		if (i < 10)
		{
			x = i * 61 + 6;
			y = 10;
		}
		else if (i < 19)
		{
			x = (i - 10) * 61 + 34; 
			y = 70;
		}
		else
		{
			x = (i - 19) * 61 + 96;
			y = 130;
		}

		rect.left = x;
		rect.top = y;
		rect.right = x + 55;
		rect.bottom = y + 55;

		int size = 55;
		if (board_count == 1)
		{
			HBRUSH oldBrush = SelectCorrectBrush(hdc, key_colors[0][i]);
			SelectObject(hdc, hFont);
			SetBkMode(hdc, TRANSPARENT);
			HPEN oldPen = SelectCorrectPen(hdc, key_colors[0][i]);

			BOOL idsasas = RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 3, 3);

			SelectObject(hdc, oldBrush);
		}
		else if (board_count == 2)
		{
			for (int j = 0; j < 2; j++)
			{
				if (key_colors[0][i] == 0 && key_colors[1][i] == 0)
				{
					HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, White);
					SelectObject(hdc, hFont);

					HPEN pen = CreatePen(PS_SOLID, 1, RGB(151, 150, 160));
					HPEN oldPen = (HPEN)SelectObject(hdc, pen);

					RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 8, 8);

					SelectObject(hdc, oldBrush);
					DeleteObject(pen);
					break;

				}
				else
				{
					HBRUSH oldBrush = SelectCorrectBrush(hdc, key_colors[j][i]);
					SelectObject(hdc, hFont);
					SetBkMode(hdc, TRANSPARENT);
					HPEN oldPen = SelectCorrectPen(hdc, key_colors[j][i]);

					BOOL idsasas = RoundRect(hdc, rect.left + size / 2 * (j % 2), rect.top , rect.right - size / 2 + size / 2 * (j % 2), rect.bottom, 3, 3);

					SelectObject(hdc, oldBrush);
				}
			}
		}
		else
		{
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, White);
			SelectObject(hdc, hFont);
			SetBkMode(hdc, TRANSPARENT);
			HPEN oldPen = SelectCorrectPen(hdc, key_colors[0][i]);

			BOOL idsasas = RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 3, 3);

			SelectObject(hdc, oldBrush);
			for (int j = 0; j < 4; j++)
			{
				if (key_colors[0][i] == 0 && key_colors[1][i] == 0 && key_colors[2][i] == 0 && key_colors[3][i] == 0)
				{
					HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, White);
					SelectObject(hdc, hFont);

					HPEN pen = CreatePen(PS_SOLID, 1, RGB(151, 150, 160));
					HPEN oldPen = (HPEN)SelectObject(hdc, pen);

					RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 8, 8);

					SelectObject(hdc, oldBrush);
					DeleteObject(pen);
					break;

				}
				else
				{
					if (key_colors[j][i] == 0) continue;
					HBRUSH oldBrush = SelectCorrectBrush(hdc, key_colors[j][i]);
					SelectObject(hdc, hFont);
					SetBkMode(hdc, TRANSPARENT);
					HPEN oldPen = SelectCorrectPen(hdc, key_colors[j][i]);

					RoundRect(hdc, rect.left + size / 2 * (j / 2), rect.top + size / 2 * (j % 2), rect.right - size / 2 + size / 2 * (j / 2), rect.bottom - size / 2 + size / 2 * (j % 2), 3, 3);

					SelectObject(hdc, oldBrush);
				}
			}
		}

		DrawTextW(hdc, s.c_str(), -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOCLIP);
	}
	DeleteDC(hdc);
	EndPaint(hWndM, &ps);

}
void PrintBoard(int k)
{
	PAINTSTRUCT ps;

	HBRUSH oldBrush = (HBRUSH)SelectObject(offDC[k], White);
	HDC hdc = BeginPaint(hWndBoards[k], &ps);
	RECT rect2;
	GetClientRect(hWndBoards[k], &rect2);

	HPEN oldPen = (HPEN)SelectObject(offDC[k], penWhite);
	offDC[k] = CreateCompatibleDC(hdc);
	offBitmap[k] = CreateCompatibleBitmap(hdc, rect2.right - rect2.left, rect2.bottom - rect2.top);

	SelectObject(offDC[k], offBitmap[k]);

	int size = 55;

	RECT rc;
	 GetClientRect(hWndBoards[k], &rc);
	 HBRUSH hBrush = CreateSolidBrush(RGB(251, 252, 255));

	 FillRect(offDC[k], &rc, hBrush);
	 DeleteObject(hBrush);

	int anim_word = word - 1;
	RECT rect;
	for (int i = 0; i < words_count; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			rect.left = j * 61 + 6;
			rect.top = i * 61 + 6;
			rect.right = rect.left + size;
			rect.bottom = rect.top + size;

			int index = i * 5 + j;
			std::wstring s(1, windows[k][index].lett);

			long time = GetTickCount() - start_anim_time - j * ANIMATION_TIME;

			double percent = (double)time / (double)ANIMATION_TIME;

			if (i == anim_word && Animation_state == ANIMATION_IN_PROGGRES && percent < 1)
			{
				
				if (time < 0)
				{
					HBRUSH oldBrush =  (HBRUSH)SelectObject(offDC[k], White);
					SelectObject(offDC[k], hFont);
					SetBkMode(offDC[k], TRANSPARENT);
					HPEN oldPen = (HPEN)SelectObject(offDC[k], penGray);


					RoundRect(offDC[k], rect.left, rect.top, rect.right, rect.bottom, 8, 8);
					DrawText(offDC[k], s.c_str(), -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOCLIP);
				}
				else if(percent < 1)
				{
					//currently animated
					int offset = size * (1 - abs( abs(percent) - 1/2));

					if (percent < 0.5)
					{
						HBRUSH oldBrush = (HBRUSH)SelectObject(offDC[k], White);
						SelectObject(offDC[k], hFont);
						SetBkMode(offDC[k], TRANSPARENT);
						HPEN oldPen = (HPEN)SelectObject(offDC[k], penGray);
					}
					else
					{
						HBRUSH oldBrush = SelectCorrectBrush(offDC[k], windows[k][index].color);
						SelectObject(offDC[k], hFont);

						SetBkMode(offDC[k], TRANSPARENT);
						HPEN oldPen = SelectCorrectPen(offDC[k], windows[k][index].color);
					}
					

					RoundRect(offDC[k], rect.left, rect.top + offset, rect.right, rect.bottom - offset, 8, 8);
					DrawText(offDC[k], s.c_str(), -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOCLIP);

				}
			}
			else
			{
				HBRUSH oldBrush = SelectCorrectBrush(offDC[k], windows[k][index].color);
				SelectObject(offDC[k], hFont);

				SetBkMode(offDC[k], TRANSPARENT);
				HPEN oldPen = SelectCorrectPen(offDC[k], windows[k][index].color);

				RoundRect(offDC[k], rect.left, rect.top, rect.right, rect.bottom, 8, 8);
				DrawText(offDC[k], s.c_str(), -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOCLIP);
			}
				
		}

	}
	if(!BitBlt(hdc, 0, 0,rect2.right+5, rect2.bottom+4, offDC[k], 0, 0, SRCCOPY)) return;
	DeleteDC(hdc);
	DeleteDC(offDC[k]);
	DeleteObject(offBitmap[k]);

	EndPaint(hWndBoards[k], &ps);

}
// gdi+ window microsoft poradnik
void colorBoard(int k)
{
	
	
	if(boards[k] == 0) return;


	RECT rect;
	HDC hdc = GetDC(hWndBoards[k]);
	std::wstring s(correct_words[k]);
	GetClientRect(hWndBoards[k], &rect);
	if (boards[k] == 1)
	{
		SolidBrush semiTransBrush(Color(200, 121, 184, 81));

		Graphics graphics(hWndBoards[k], FALSE);
		int l = rect.left;
		graphics.FillRectangle(&semiTransBrush, l,rect.top,rect.right-rect.left,rect.bottom-rect.top);

	}
	else
	{
		SolidBrush semiTransBrush(Color(203, 255, 0, 0));
		Graphics graphics(hWndBoards[k], FALSE);
		int l = rect.left;
		graphics.FillRectangle(&semiTransBrush, l, rect.top, rect.right - rect.left, rect.bottom - rect.top);

		SelectObject(hdc, hFont_END);
		SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, s.c_str(), -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOCLIP);
		int c = 0;
	}
	DeleteDC(hdc);



}
void Generate_Words()
{
	// wyznaczanie liczby linijek
	 //https://stackoverflow.com/questions/3072795/how-to-count-lines-of-a-file-in-c
	std::ifstream inFile("./Wordle.txt");
	int lines = std::count(std::istreambuf_iterator<char>(inFile),
		std::istreambuf_iterator<char>(), '\n');

	std::wifstream winFile("./Wordle.txt");
	

	for (int i = 0; i < 4; i++)
	{
		std::wifstream winFile("./Wordle.txt");
		int lim = rand() % (lines);

		for (int j = 0; j < lim; j++)
		{
			std::getline(winFile, c_word1[i]);
		}
		std::transform(c_word1[i].begin(), c_word1[i].end(), c_word1[i].begin(), ::toupper);
		correct_words[i] = c_word1[i].c_str();

	}
	

}
HBRUSH SelectCorrectBrush(HDC hdc,int color)
{
	if (color == 0)
	{
		return (HBRUSH)SelectObject(hdc, White);
	}
	else if (color == 1)
	{
		return (HBRUSH)SelectObject(hdc, Green);
	}
	else if (color == 2)
	{
		return (HBRUSH)SelectObject(hdc, Yellow);
	}
	else
	{
		return (HBRUSH)SelectObject(hdc, Gray);
	}
}

HPEN SelectCorrectPen(HDC hdc, int color)
{

	if (color == 1)
	{
		return (HPEN)SelectObject(hdc, penGreen);
	}
	else if (color == 2)
	{
		return (HPEN)SelectObject(hdc, penYellow);
	}
	else if (color == 0)
	{
		return (HPEN)SelectObject(hdc, penGray);
	}
	else
	{
		return (HPEN)SelectObject(hdc, penGray);
	}
}

//https://stackoverflow.com/questions/23433095/how-to-check-if-a-char-is-in-a-string-in-c
bool isInside(const std::wstring& str, WCHAR c)
{
	return str.find(c) != std::wstring::npos;
}

void clearGame()
{

	//potencjalnie losuj nowe slowa
	for (int i = 0; i < 4; i++)
	{
		boards[i] = 0;
		for (int j = 0; j < 50; j++)
		{
			windows[i][j].color = 0;
			windows[i][j].lett = ' ';
		}
		is_correct[i] = false;
		for (int j = 0; j < 26; j++)
		{
			key_colors[i][j] = 0;
		}
	}
	word = 0;
	curr_letter = 0;
	Generate_Words();
}