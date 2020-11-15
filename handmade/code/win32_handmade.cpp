/*
* @Author: Jin
* @Date:   2020-11-14 20:17:42
* @Last Modified by:   Jin
* @Last Modified time: 2020-11-14 20:21:33
*/
#include <windows.h>

// Window CALLBACK => "Search for WindowProc"
LRESULT CALLBACK
MainWindowCallback(
	HWND   Window, // A handle to the window, telling the windows that which window we are talking about?
	UINT   Message, // Message that windows is asking us to handle "Search for System-Defined Messages"
	WPARAM WParam,
	LPARAM LParam)
{
	LRESULT Result = 0;

	switch (Message)
	{
	case WM_SIZE: // When the user changes size of the window
	{
		OutputDebugStringA("WM_SIZE\n");
	} break;

	case WM_DESTROY: // When Windows deletes the window
	{
		OutputDebugStringA("WM_DESTROY\n");
	} break;

	case WM_CLOSE: // When the user clicks "X" in the window
	{
		OutputDebugStringA("WM_CLOSE\n");
	} break;

	case WM_ACTIVATEAPP: // When the user clicks the window
	{
		OutputDebugStringA("WM_ACTIVATEAPP\n");
	} break;

	// Fill the Message on the window
	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(Window, &Paint);
		LONG X = Paint.rcPaint.left;
		LONG Y = Paint.rcPaint.top;
		LONG Height = Paint.rcPaint.bottom - Paint.rcPaint.top; // Width
		LONG Width = Paint.rcPaint.right - Paint.rcPaint.left; // Height
		static DWORD Operation = WHITENESS; // Initially White background
		PatBlt(DeviceContext, X, Y, Width, Height, Operation);
		if (Operation == WHITENESS)
		{
			Operation = BLACKNESS;
		}
		else
		{
			Operation = WHITENESS;
		}
		EndPaint(Window, &Paint);

	} break;

	default:
	{
		//OutputDebugStringA("default\n");
		Result = DefWindowProc(Window, Message, WParam, LParam);
	} break;
	}

	return(Result);
}

// Entry Point for Window.
int CALLBACK WinMain(HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CommandLine,
	int ShowCode)
{
	WNDCLASS WindowClass = {};
	//MSDN => Search "WindowClass" from MSDN
	//TODO(HAN): check if HREDDRAW/VREDRAW/OWNDC still matter
	WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW; // binary flag, Search for "Window Class Styles" for flags that you can set
	WindowClass.lpfnWndProc = MainWindowCallback; // pointer to a function, Search for "WindowProc"
	WindowClass.hInstance = Instance;
	// WindowClass.hIcon;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&WindowClass))
	{
		HWND WindowHandle =
			CreateWindowExA(
				0, // Extended Style
				WindowClass.lpszClassName, // Class Name
				"Handmade Hero",  // Window Name
				WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Window Style
				CW_USEDEFAULT,// X
				CW_USEDEFAULT,// Y
				CW_USEDEFAULT, // Width
				CW_USEDEFAULT, // Height
				0, // Parent Window
				0,
				Instance,
				0);
		if (WindowHandle)
		{
			MSG Message;
			// Keep running until GetMessage returns false
			for (;;)
			{
				BOOL MessageResult = GetMessageA(&Message, 0, 0, 0);
				if (MessageResult > 0)
				{
					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}
				else
				{
					break;
				}

			}
		}

		else
		{
			// TODO(HAN): Logging
		}
	}
	else
	{
		//TODO(HAN): Logging
	}
	return(0);
}