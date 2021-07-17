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

// Win_Client.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Win_Client.h"

// >> :
#include <list>
#include <vector>
using namespace std;

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#define WM_ASYNC WM_USER+1
// <<

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

// : >>
WSADATA wsadata;
SOCKET	s;
SOCKADDR_IN addr = { 0 };
TCHAR msg[100], str[100];
char buffer[100];
int msgLen;
int msgCount = 0;

int InitClient(HWND hWnd);
void CloseClient();

int Win_Client();
int Win_Client_init();
void SendMessageToServer();

// TCHAR형 msg를 char형 buffer로 변환한다. (유니코드->멀티바이트)
void MsgToBuffer(TCHAR* msg, char* buffer);

// 네트워크를 통해 메세지를 받은 후 가공하여 내 메세지 박스로 전송한다.
void MyReadMessage(HWND);

// 메세지를 가공하여 send server로 보낸다.
void MySendMessage(const TCHAR* str, HWND hTargetBox);

// 유니코드가된 메세지를 가공하여 내 메세지 박스로 전송한다.
void MyAddList(const TCHAR*, HWND);

// 이 클라이언트의 아이디t
TCHAR ID[10] = _T("kkami");
// <<

// : >> 모달리스
static HWND hModallessDlg;
BOOL CALLBACK CONTROL_Proc1(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
// <<

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
    LoadStringW(hInstance, IDC_WINCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINCLIENT));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINCLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINCLIENT);
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

   /*HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
*/
   HWND hWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CHAT_DIALOG1), NULL, CONTROL_Proc1);

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
	case WM_CREATE:
		
		//return Win_Client_init();
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
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case ID_FUNC1:
				if (!IsWindow(hModallessDlg))
				{
					hModallessDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CHAT_DIALOG1), hWnd, CONTROL_Proc1);
					ShowWindow(hModallessDlg, SW_SHOW);
				}

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
			
			EndPaint(hWnd, &ps);
        }
        break;
	case WM_KEYDOWN:
		break;
    case WM_DESTROY:
		// closesocket(s);
		// WSACleanup();
		//CloseClient();
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

int InitClient(HWND hWnd)
{
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	s = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_port = 20;
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");	// 로컬에서만 처리할 떄 주소
	if (connect(s, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR)
		return 0;
	
	/*
	WSAAsyncSelect
	- 소켓을 위한 윈도우 메세지와 처리할 네트워크 이벤트 등록
	- 등록한 네트워크 이벤트(FD_READ)가 발생하면 윈도우 메세지가 발생하고 윈도우 프로시저가 호출된다.
	- 윈도우 프로시저에서는 받은 메세지 종류에 따라 적절한 소켓 함수를 호출하여 처리
	*/
	WSAAsyncSelect(s, hWnd, WM_ASYNC, FD_READ);

	return 0;
}

void CloseClient()
{
	closesocket(s);
	WSACleanup();
}


BOOL CALLBACK CONTROL_Proc1(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

	
	// msg는 멀티바이트형 문자열을 저장한다. 즉 클라이언트 프로그램 내에서 메세지를 전송할 때 쓰인다.
	// static TCHAR msg[100];
	
	// buffer는 멀티바이트혈 문자열을 저장한다. 즉 네트워크 통신에 쓰인다. 보낼 때 & 받을 때 모두!
	// static char buffer[100];

	// 리스트 컨트롤 박스 핸들 값
	static HWND hList;
	static HWND hConnectBtn;
	static HWND hDisconntectBtn;
	static HWND hSendBtn;

	switch (iMsg)
	{
	case WM_INITDIALOG:
		hList			= GetDlgItem(hDlg, IDC_LIST_CHAT);
		hConnectBtn = GetDlgItem(hDlg, IDC_BUTTON_CONNECT);
		hDisconntectBtn = GetDlgItem(hDlg, IDC_BUTTON_DISCONNECT);
		hSendBtn		= GetDlgItem(hDlg, IDC_BUTTON_SEND);
		
		EnableWindow(hConnectBtn, true);
		EnableWindow(hDisconntectBtn, false);
		EnableWindow(hSendBtn, false);
		return 1;

	case WM_ASYNC:
	{
		switch (lParam)
		{
			case FD_READ:
			{
				cout << "클라이언트 : FD_READ 받음\n";
				// 메세지를 받아서 컴파일러의 문자 해석 형식으로 가공
				MyReadMessage(hList);
			}
			break;
		}
	}
	break;
		
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
			case IDC_BUTTON_CONNECT:
			{
				InitClient(hDlg);
				MyAddList(_T("서버 접속"), hList);
				EnableWindow(hConnectBtn, false);
				EnableWindow(hDisconntectBtn, true);
				EnableWindow(hSendBtn, true);
			}
			break;

			case IDC_BUTTON_DISCONNECT:
			{
				CloseClient();
				MyAddList(_T("접속 종료"), hList);
				EnableWindow(hConnectBtn, true);
				EnableWindow(hDisconntectBtn, false);
				EnableWindow(hSendBtn, false);
			}
			break;

			case IDC_BUTTON_SEND:
			{
				// IDC_CHAT 에디트 컨트롤에서 문자열을 받아온 후 에디트 컨트롤을 비운다
				GetDlgItemText(hDlg, IDC_CHAT, msg, 100);
				SetDlgItemText(hDlg, IDC_CHAT, _T(""));

				// msg내 문자를 다른 문자열에 붙여서 IDAndMsg 생성
				TCHAR IDAndMsg[100];
				_stprintf_s(IDAndMsg, 99, _T("%s : %s"), ID, msg);
				

				// TCHAR형 메세지 값을 char형 버퍼 배열 값으로 변환
				MsgToBuffer(IDAndMsg, buffer);


				// 연결된 소켓에 메세지 전송
				send(s, buffer, strlen(buffer) + 1, 0);
				return 0;
			}
			break;

		default:
			break;
		}
	}
		
	default:
		return 0;
	}
	return 0;
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

void MyReadMessage(HWND hTargetBox)
{
	// 연결된 소켓을 통해 buffer로 데이터를 받는다. 네트워크 통신으로 받았기 때문에 Buffer는 멀티바이트형으로 받아진 상태다.
	int msgLen = recv(s, buffer, 100, 0);

	// 컴파일러가 유니코드를 지원할 경우를 생각하기 위해 문자를 옮긴다.
	// msg는 프로그램 내 대화 상자와 상호작용하기 위한 문자열이며
	// buffer는 통신용이다. 멀티바이트형으로 받아질 수 있기 때문.
	if (msgLen > 0)
	{
		buffer[msgLen] = NULL;
#ifdef _UNICODE
		msgLen = MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), NULL, 0);
		MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), msg, msgLen);
#else
		strcpy(msg, buffer);
		msgLen = strlen(msg);
#endif
		msg[msgLen] = 0;

		// 출력 형식에 맞춰 리스트 박스 컨트롤에 데이터 전송
		MyAddList(msg, hTargetBox);
	}

}


void MySendMessage(const TCHAR* str, HWND hTargetBox)
{
	// 연결된 소켓을 통해 buffer로 데이터를 받는다. 네트워크 통신으로 받았기 때문에 Buffer는 멀티바이트형으로 받아진 상태다.
	int msgLen = recv(s, buffer, 100, 0);

	// 컴파일러가 유니코드를 지원할 경우를 생각하기 위해 문자를 옮긴다.
	// msg는 프로그램 내 대화 상자와 상호작용하기 위한 문자열이며
	// buffer는 통신용이다. 멀티바이트형으로 받아질 수 있기 때문.
	if (msgLen > 0)
	{
		buffer[msgLen] = NULL;
#ifdef _UNICODE
		msgLen = MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), NULL, 0);
		MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), msg, msgLen);
#else
		strcpy(msg, buffer);
		msgLen = strlen(msg);
#endif
		msg[msgLen] = 0;

		// 출력 형식에 맞춰 리스트 박스 컨트롤에 데이터 전송
		MyAddList(msg, hTargetBox);
	}

}

void MyAddList(const TCHAR* str, HWND hTargetBox)
{	
	static int chat_box_idx = 0;

	// 아이디와 메세지를 합한 배열
	// TCHAR IDAndMsg[100];

	// msg내 문자를 다른 문자열에 붙여서 IDAndMsg 생성
	// _stprintf_s(IDAndMsg, 99, _T("%s : %s"), ID, str);

	// 타겟 박스에 IDAndMsg 전송
	
	SendMessage(hTargetBox, LB_INSERTSTRING, -1, (LPARAM)str);
	cout << "client: chat_box_idx: " << chat_box_idx << endl;
}