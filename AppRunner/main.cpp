/*
Sources
http://bobobobo.wordpress.com/2009/03/30/adding-an-icon-system-tray-win32-c/
http://www.cplusplus.com/forum/windows/26347/
http://www.cplusplus.com/forum/windows/4209/
http://forums.codeguru.com/showthread.php?474235-How-to-capture-quot-Enter-quot-Key
*/
#pragma region include and define
#include <windows.h>
#include "resource.h"
#include <shellapi.h>
#include <stdio.h>
#include <string>
#include <iostream>

#ifdef UNICODE
#define stringcopy wcscpy
#else
#define stringcopy strcpy
#endif

#define ID_TRAY_APP_ICON                5000
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM  3000
#define ID_TRAY_EXPAND_CONTEXT_MENU_ITEM  2000
#define WM_TRAYICON ( WM_USER + 1 )
#pragma endregion

#pragma region constants and globals
UINT WM_TASKBARCREATED = 0 ;

HWND g_hwnd ;
HMENU g_menu ;
HWND hWndTextbox;
NOTIFYICONDATA g_notifyIconData ;
char propertyName[1000][1000];
char propertyValue[1000][1000];
int propertyCount = 0;
#pragma endregion


LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);



#pragma region helper funcs
void ReadConfigFile()
{
	FILE *propertyFile = fopen("commands.properties", "r");
	char buffer[1000];
	int commandMapIndex=0;
	char dataByte;
	char propName[1000];
	char propValue[1000];
	int flag = 0;
	int propNameIndex = 0;
	int propValueIndex = 0;
	while((dataByte = fgetc(propertyFile)) != EOF) {
		if('\n' == dataByte) {
			propName[propNameIndex] = '\0';
			propValue[propValueIndex] = '\0';
			strcpy(propertyName[commandMapIndex], propName);
			strcpy(propertyValue[commandMapIndex], propValue);
			commandMapIndex++;
			flag = 0;
			propNameIndex = 0;
			propValueIndex = 0;
		}
		else if('=' == dataByte) {
			flag = 1;
		}
		else {
			if(flag) {
				propValue[propValueIndex++] = dataByte;
			}
			else {
				propName[propNameIndex++] = dataByte;
			}
		}
	}
	propertyCount = commandMapIndex;
	fclose(propertyFile);
}

void Minimize()
{
  Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);
  ShowWindow(g_hwnd, SW_HIDE);
}

void PreTranslateMessage(LPMSG msg)
{
	if(msg->message==WM_KEYUP)
       {
           switch(msg->wParam)
           {
            case VK_RETURN:
				if(GetActiveWindow()==g_hwnd)
				{
					char commandText[994];
					GetWindowTextA(hWndTextbox, commandText, 994);
					
					int result;
					int commandListCount=0;
					for(; commandListCount < propertyCount; commandListCount++) {
						if(0 == (result = strcmp(propertyName[commandListCount], commandText))) {
							break;
						}
					}
					char command[1000];
					strcpy(command, "start ");
					if(result) {
						strcat(command, commandText);
					}
					else {
						strcat(command, propertyValue[commandListCount]);						
					}
					system(command);
					Minimize();
				}
                  return;/*i don't want this message to reach the procedures anway*/
           }
       }

	TranslateMessage(msg);
	DispatchMessage(msg);
}

void Restore()
{
  Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);
  ShowWindow(g_hwnd, SW_SHOWNORMAL);
  SetFocus(g_hwnd);
  SetFocus(hWndTextbox);
}

void RegisterGlobalKeys() {
	RegisterHotKey(g_hwnd, 1, MOD_ALT , VK_F12);
}

void InitNotifyIconData(HINSTANCE hInstance)
{
  memset( &g_notifyIconData, 0, sizeof( NOTIFYICONDATA ) ) ;
 
  g_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
  
  g_notifyIconData.hWnd = g_hwnd;
  g_notifyIconData.uID = ID_TRAY_APP_ICON;
  g_notifyIconData.uFlags = NIF_ICON | // promise that the hIcon member WILL BE A VALID ICON!!
    NIF_MESSAGE | // when someone clicks on the system tray icon,
    // we want a WM_ type message to be sent to our WNDPROC
    NIF_TIP;      // we're gonna provide a tooltip as well, son.

  g_notifyIconData.uCallbackMessage = WM_TRAYICON; //this message must be handled in hwnd's window procedure. more info below.
  
  //g_notifyIconData.hIcon = (HICON)LoadImage( NULL, TEXT("AppRunner.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE  ) ;
  g_notifyIconData.hIcon = (HICON)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));


  // set the tooltip text.  must be LESS THAN 64 chars
  stringcopy(g_notifyIconData.szTip, TEXT("Click to expand."));
}
#pragma endregion

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR args, int iCmdShow )
{
  TCHAR className[] = TEXT( "tray icon class" );

  ReadConfigFile();

  // I want to be notified when windows explorer
  // crashes and re-launches the taskbar.  the WM_TASKBARCREATED
  // event will be sent to my WndProc() AUTOMATICALLY whenever
  // explorer.exe starts up and fires up the taskbar again.
  // So its great, because now, even if explorer crashes,
  // I have a way to re-add my system tray icon in case
  // the app is already in the "minimized" (hidden) state.
  // if we did not do this an explorer crashed, the application
  // would remain inaccessible!!
  WM_TASKBARCREATED = RegisterWindowMessageA("TaskbarCreated") ;

  #pragma region add a console
  // add a console, because I love consoles.
  // To disconnect the console, just comment out
  // the next 3 lines of code.
  //// AllocConsole();
  //// AttachConsole( GetCurrentProcessId() ) ;
  //// freopen( "CON", "w", stdout ) ;
  #pragma endregion
  
  #pragma region get window up
  WNDCLASSEX wnd = { 0 };

  wnd.hInstance = hInstance;
  wnd.lpszClassName = className;
  wnd.lpfnWndProc = WndProc;
  wnd.style = CS_HREDRAW | CS_VREDRAW ;
  wnd.cbSize = sizeof (WNDCLASSEX);

  //wnd.hIcon = LoadIcon (NULL, IDI_APPLICATION);
  //wnd.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
  //wnd.hIcon = (HICON)LoadImage( NULL, TEXT("AppRunner.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE  ) ;
  //wnd.hIconSm = (HICON)LoadImage( NULL, TEXT("AppRunner.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE  ) ;
  wnd.hIcon = (HICON)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
  wnd.hIconSm = (HICON)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
  wnd.hCursor = LoadCursor (NULL, IDC_ARROW);
  wnd.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE ;
  
  if (!RegisterClassEx(&wnd))
  {
    FatalAppExit( 0, TEXT("Couldn't register window class!") );
  }
  
  g_hwnd = CreateWindowEx (
    
    0, className,
    
    TEXT( "App Launcher" ),
    WS_OVERLAPPEDWINDOW,
    
    CW_USEDEFAULT, CW_USEDEFAULT, 
    200, 75, 

    NULL, NULL, 
    hInstance, NULL
  );

  // Add the label with instruction text
  //CreateWindow( TEXT("static"), TEXT("right click the system tray icon to close"), WS_CHILD | WS_VISIBLE | SS_CENTER,
//                  0, 0, 100, 50, g_hwnd, 0, hInstance, NULL ) ;
  
  hWndTextbox = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,	0, 0, 190, 45, g_hwnd, 0, hInstance, NULL);
  SetFocus(hWndTextbox);
  // Initialize the NOTIFYICONDATA structure once
  InitNotifyIconData(hInstance);


  ShowWindow (g_hwnd, iCmdShow);
//MessageBox(g_hwnd, TEXT("Data"), TEXT("Title"), MB_OK);  
  RegisterGlobalKeys();
#pragma endregion

  MSG msg ;
  while (GetMessage (&msg, NULL, 0, 0))
  {
	PreTranslateMessage(&msg);
  }


  // Once you get the quit message, before exiting the app,
  // clean up and remove the tray icon
  if( !IsWindowVisible( g_hwnd ) )
  {
    Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);
  }
  UnregisterHotKey(g_hwnd, NULL);
  return msg.wParam;
}


LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if ( message==WM_TASKBARCREATED && !IsWindowVisible( g_hwnd ) )
  {
    Minimize();
    return 0;
  }

  switch (message)
  {
  case WM_CREATE:

    // create the menu once.
    // oddly, you don't seem to have to explicitly attach
    // the menu to the HWND at all.  This seems so ODD.
    g_menu = CreatePopupMenu();

	AppendMenu(g_menu, MF_STRING, ID_TRAY_EXPAND_CONTEXT_MENU_ITEM,  TEXT( "Expand" ) );
    AppendMenu(g_menu, MF_STRING, ID_TRAY_EXIT_CONTEXT_MENU_ITEM,  TEXT( "Exit" ) );

    break;
  
  case WM_SYSCOMMAND:
    switch( wParam & 0xfff0 )  // (filter out reserved lower 4 bits:  see msdn remarks http://msdn.microsoft.com/en-us/library/ms646360(VS.85).aspx)
    {
    case SC_MINIMIZE:
		Minimize() ; 
		return 0 ;
		break;
    case SC_CLOSE:  // redundant to WM_CLOSE, it appears
	  PostQuitMessage( 0 ) ;
	  return 0 ;
	  break;
    }
    break;


  // Our user defined WM_TRAYICON message.
  // We made this message up, and we told
  // 
  case WM_TRAYICON:
    {
      printf( "Tray icon notification, from %d\n", wParam ) ;
      
      switch(wParam)
      {
      case ID_TRAY_APP_ICON:
        printf( "Its the ID_TRAY_APP_ICON.. one app can have several tray icons, ya know..\n" ) ;
        break;
      }

      // the mouse button has been released.
      
      // I'd LIKE TO do this on WM_LBUTTONDOWN, it makes
      // for a more responsive-feeling app but actually
      // the guy who made the original post is right.
      // Most apps DO respond to WM_LBUTTONUP, so if you
      // restore your window on WM_LBUTTONDOWN, then some
      // other icon will scroll in under your mouse so when
      // the user releases the mouse, THAT OTHER ICON will
      // get the WM_LBUTTONUP command and that's quite annoying.
      if (lParam == WM_LBUTTONUP)
      {
        printf( "You have restored me!\n" ) ;
        Restore();
      }
      else if (lParam == WM_RBUTTONDOWN) // I'm using WM_RBUTTONDOWN here because
      {
        printf( "Mmm.  Let's get contextual.  I'm showing you my context menu.\n" ) ;
        // it gives the app a more responsive feel.  Some apps
        // DO use this trick as well.  Right clicks won't make
        // the icon disappear, so you don't get any annoying behavior
        // with this (try it out!)

        // Get current mouse position.
        POINT curPoint ;
        GetCursorPos( &curPoint ) ;
        
        // should SetForegroundWindow according
        // to original poster so the popup shows on top
        SetForegroundWindow(hwnd); 
		SetFocus(hWndTextbox);
	       
        
        // TrackPopupMenu blocks the app until TrackPopupMenu returns
        printf("calling track\n");
        UINT clicked = TrackPopupMenu(
          
          g_menu,
          TPM_RETURNCMD | TPM_NONOTIFY, // don't send me WM_COMMAND messages about this window, instead return the identifier of the clicked menu item
          curPoint.x,
          curPoint.y,
          0,
          hwnd,
          NULL

        );
        printf("returned from call to track\n");


        // Original poster's line of code.  Haven't deleted it,
        // but haven't seen a need for it.
        //SendMessage(hwnd, WM_NULL, 0, 0); // send benign message to window to make sure the menu goes away.
        if (clicked == ID_TRAY_EXIT_CONTEXT_MENU_ITEM)
        {
          // quit the application.
          printf("I have posted the quit message, biatch\n");
          PostQuitMessage( 0 ) ;
        }
		else if(clicked == ID_TRAY_EXPAND_CONTEXT_MENU_ITEM) {
			Restore();
		}
      }
    }
    break;

  // intercept the hittest message.. making full body of
  // window draggable.
  case WM_NCHITTEST:
  {
    // http://www.catch22.net/tuts/tips
    // this tests if you're on the non client area hit test
    UINT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
    if(uHitTest == HTCLIENT)
      return HTCAPTION;
    else
      return uHitTest;
  }

  case WM_CLOSE:
    printf( "Got an actual WM_CLOSE Message!  Woo hoo!\n" ) ;
    Minimize() ;
    return 0;
    break;

  case WM_DESTROY:
    printf( "DESTROY!!\n" ) ;
    PostQuitMessage (0);
    break;
	
  case WM_HOTKEY:
	Restore();
	break;

  }

  return DefWindowProc( hwnd, message, wParam, lParam ) ;
}