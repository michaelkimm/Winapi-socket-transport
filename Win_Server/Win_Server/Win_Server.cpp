#define _WINSOCK_DEPRECATED_NO_WARNINGS

// : >> 디버깅용 콘솔창
#include <stdio.h>
#include <iostream>

#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

using namespace std;
static int cnt = 0;
// <<


// Win_Server.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Win_Server.h"
#include <stdio.h>
WORD a;
// >> :
#include <list>
#include <vector>
using namespace std;

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#define WM_ASYNC WM_USER+1
list <SOCKET> socketList;
WSADATA wsaData;
SOCKET s, cs;
TCHAR msg[100];
SOCKADDR_IN addr = { 0 }, c_addr;
int size, msgLen;
char buffer[100];

int Init_Server(HWND hWnd);
int Close_Server();

SOCKET AcceptSocket(HWND hWnd, SOCKET s, SOCKADDR_IN& c_addr);
void SendMessageToClient(char * buffer);
void ReadMessage(TCHAR * msg, char * buffer, HWND& msgList);
void CloseClient(SOCKET s);
// <<

// : >> 모달리스
static HWND hModallessDlg;
BOOL CALLBACK CHAT_Proc1(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

// TCHAR형 msg를 char형 buffer로 변환한다. (유니코드->멀티바이트)
void MsgToBuffer(TCHAR* msg, char* buffer);
void MyAddList(const TCHAR*, HWND);

// <<

// 이 서버의 아이디
TCHAR ID[10] = _T("Rubi");


#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// int WinServer();
// int WinServer_init();

//WSADATA wsadata;
//SOCKET	s;
//SOCKADDR_IN addr = { 0 };
//TCHAR message[256];

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINSERVER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINSERVER));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINSERVER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINSERVER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CHAT_DIALOG1), NULL, CHAT_Proc1);

   /*HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);*/

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	

    switch (message)
    {
	case WM_USER:

		break;

	case WM_CREATE:
		// WinServer_init();
		
		break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
			case ID_FUNC_CHAT:
				cout << "ID_FUNC_CHAT 눌림\n";
				if (!IsWindow(hModallessDlg))
				{
					hModallessDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CHAT_DIALOG2), hWnd, CHAT_Proc1);
					ShowWindow(hModallessDlg, SW_SHOW);
				}
				break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
			
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		// closesocket(s);
		
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


BOOL CALLBACK CHAT_Proc1(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hList;
	static HWND hConnectBtn;
	static HWND hDisconntectBtn;
	static HWND hSendBtn;

	switch (iMsg)
	{
		case WM_INITDIALOG:
		{
			hList = GetDlgItem(hDlg, IDC_LIST_CHAT);
			hConnectBtn = GetDlgItem(hDlg, IDC_BTN_CONNECT);
			hDisconntectBtn = GetDlgItem(hDlg, IDC_BTN_DISCONNECT);
			hSendBtn = GetDlgItem(hDlg, IDC_BTN_SEND);

			EnableWindow(hConnectBtn, true);
			EnableWindow(hDisconntectBtn, false);
			EnableWindow(hSendBtn, false);
		}
		break;

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_BTN_SEND:
				{
					cout << "메세지 전송 눌림\n";
					GetDlgItemText(hDlg, IDC_CHAT, msg, 100);
					SetDlgItemText(hDlg, IDC_CHAT, _T(""));

					// 아이디와 메세지를 합한 배열
					TCHAR IDAndMsg[100];

					// msg내 문자를 다른 문자열에 붙여서 IDAndMsg 생성
					_stprintf_s(IDAndMsg, 99, _T("%s : %s"), ID, msg);

					// 메세지를 버퍼로 변경
					MsgToBuffer(IDAndMsg, buffer);

					// 내가 적은 것 클라이언트에게 전송
					SendMessageToClient(buffer);

					// 내 채팅창도 업데이트
					MyAddList(IDAndMsg, hList);
				}
				break;

				case IDC_BTN_CONNECT:
				{
					cout << "서버 오픈 눌림 눌림\n";
					if (Init_Server(hDlg))
						MyAddList(_T("서버 오픈"), hList);
					EnableWindow(hConnectBtn, false);
					EnableWindow(hDisconntectBtn, true);
					EnableWindow(hSendBtn, true);
				}
				break;

				case IDC_BTN_DISCONNECT:
				{
					cout << "서버 종료 눌림\n";
					Close_Server();
					MyAddList(_T("서버 종료"), hList);
					EnableWindow(hConnectBtn, true);
					EnableWindow(hDisconntectBtn, false);
					EnableWindow(hSendBtn, false);

				}
				break;

				default:
					return 0;
			}
		}
		break;

		case WM_ASYNC:
		{
			switch (lParam)
			{
				case FD_ACCEPT:
				{
					cout << "FD_ACCEPT 받음\n";
					cs = AcceptSocket(hDlg, s, c_addr);
					MyAddList(_T("유저 입장"), hList);
				}
				break;

				case FD_READ:
				{
					cout << "FD_READ 받음\n";
					// 네트워크 통신으로 buffer 배열에 메세지를 받는다. 컴파일러 특성에 따라 msg로 알맞게 변환한다.
					// 버퍼에 데이터를 소켓 리스트에 있는 소켓들을 통해 전송한다.
					// 변환된 msg를 내 리스트 박스 컨트롤에 전송한다.
					ReadMessage(msg, buffer, hList);
				}
				break;

				case FD_CLOSE:
				{
					cout << "FD_CLOSE 받음\n";
					CloseClient(wParam);
					MyAddList(_T("유저 퇴장"), hList);
				}
				break;
			}
		}
		break;
	}
	return 0;
}


int Init_Server(HWND hWnd)
{
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	s = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_port = 20;
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	// 주소 addr와 소켓 s를 연결한다.
	bind(s, (LPSOCKADDR)&addr, sizeof(addr));

	// 소켓 s에서 FD_ACCEPT 네트워크 이벤트가 발생하면 WM_ASYNC를 hWnd의 윈도우 프로시저에 전송한다.
	WSAAsyncSelect(s, hWnd, WM_ASYNC, FD_ACCEPT);

	if (listen(s, 5) == SOCKET_ERROR) return 0;
	return 1;
}

int Close_Server()
{
	closesocket(s);
	WSACleanup();
	return 0;
}

SOCKET AcceptSocket(HWND hWnd, SOCKET s, SOCKADDR_IN & c_addr)
{
	SOCKET cs;
	int size;
	size = sizeof(c_addr);
	cs = accept(s, (LPSOCKADDR)&c_addr, &size);
	WSAAsyncSelect(cs, hWnd, WM_ASYNC, FD_READ | FD_CLOSE);	// 접속했던 유저가 종료하면 이 메세지를 발생시켜라

	// 접속한 애들을 리스트로 관리
	socketList.push_back(cs);
	return cs;
}

void SendMessageToClient(char * buffer)
{
	for (list<SOCKET>::iterator it = socketList.begin(); it != socketList.end(); it++)
	{
		SOCKET cs = (*it);
		send(cs, (LPSTR)buffer, strlen(buffer) + 1, 0);
	}
}

void ReadMessage(TCHAR * msg, char * buffer, HWND& msgList)
{
	for (list<SOCKET>::iterator it = socketList.begin(); it != socketList.end(); it++)
	{
		SOCKET cs = (*it);

		// 소켓 cs에서 멀티바이트형으로 문자를 읽어서 buffer에 저장
		int msgLen = recv(cs, buffer, 100, 0);

		// 메세지가 왔으면 
		if (msgLen > 0)	
		{
			buffer[msgLen] = NULL;
#ifdef _UNICODE
			msgLen = MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), NULL, NULL);
			MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), msg, msgLen);
			msg[msgLen] = NULL;
#else
			strcpy(msg, buffer);
#endif _UNICODE

			SendMessageToClient(buffer);
			MyAddList(msg, msgList);
		}
	}
}

void CloseClient(SOCKET socket)
{
	for (list<SOCKET>::iterator it = socketList.begin(); it != socketList.end(); it++)
	{
		SOCKET cs = (*it);
		if (cs == socket)
		{
			closesocket(cs);
			it = socketList.erase(it);
			break;
		}
	}
}

void MsgToBuffer(TCHAR* msg, char* buffer)
{
	int msgLen;
#ifdef _UNICODE
	msgLen = WideCharToMultiByte(CP_ACP, 0, msg, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, msg, -1, buffer, msgLen, NULL, NULL);
#else
	strcpy(buffer, msg);
	msgLen = strlen(buffer);
#endif
	msg[msgLen] = 0;
	return;
}

void MyAddList(const TCHAR* str, HWND hTargetBox)
{
	static int chat_box_idx = 0;

	SendMessage(hTargetBox, LB_INSERTSTRING, -1, (LPARAM)str);
	cout << "server: chat_box_idx: " << chat_box_idx << endl;
}

