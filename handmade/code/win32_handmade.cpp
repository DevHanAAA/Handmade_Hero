/*
* @Author: Jin
* @Date:   2020-11-14 20:17:42
* @Last Modified by:   Jin
* @Last Modified time: 2020-11-14 20:21:33
*/

#include <windows.h>

#define internal static // function is local to the file
#define local_persist static // persist, retain the value
#define global_variable static // Not preferred method, temporary

//TODO(Han):this is global for now (sometimes something will popup when user tries to close. so we can implement this in different way)
global_variable bool Running; // Ver.3

global_variable BITMAPINFO BitmapInfo; // We only care about bmiHeader
global_variable void* BitmapMemory; // "Actual memory" we are going to receive back to window. we used void because we don't know format of the things to which it is pointing to, we will cast it for the format we actually want to use
global_variable HBITMAP BitmapHandle; // Handle to a bitmap
global_variable HDC BitmapDeviceContext;

//Displaying GDI
internal void
Win32ResizeDIBSection(int Width, int Height) // Device Independent Bitmap => Search for "ResizeDIBSection"
{
	// TODO(Han): Bulletproof this
	// May be don't free first, free after, then free first if that fails.

	 // Free our DIBSection
	if (BitmapHandle) // if function fails return value is null, so we will have to delete Bitmaphandle
	{
		DeleteObject(BitmapHandle); // Search for "DeleteObject"
	}
	if (!BitmapDeviceContext) // if we don't have BitmapDeviceContextr
	{
		// TODO(Han): Should we recreate these under certain special circumstances? (What if some user change the resolution?)
		BitmapDeviceContext = CreateCompatibleDC(0); // Search for "CreateCompatibleDC"
	}

	// Allocate BITMAP (// Search for "BITMAPINFO")
	// Search for "BITMAPINFOHEADER"
	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader); // # Bytes => How big is the structure actually is? find out structure itself by using "sizeof", should not include color table "bmiColors" so you should not use sizeof(BitmapInfo);
	BitmapInfo.bmiHeader.biWidth = Width; // Width
	BitmapInfo.bmiHeader.biHeight = Height; // Height
	BitmapInfo.bmiHeader.biPlanes = 1; // Always 1
	BitmapInfo.bmiHeader.biBitCount = 32; // # of bit per pixel, 8 bit for red,green,blue each + @
	BitmapInfo.bmiHeader.biCompression = BI_RGB; // We don't want compression

	/* IGNORE(Han) All Default to zero because we declared BitmapInfo as global_variable
	BitmapInfo.bmiHeader.biSizeImage = 0; // Set 0 since we don't have compression format
	BitmapInfo.bmiHeader.biXPelsPerMeter = 0;  // For Pixel per meter
	BitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biClrUsed = 0; // How many colors are used in the color table? We do not have color table
	BitmapInfo.bmiHeader.biClrImportant = 0; // We don't have any index PAL stuff
	*/

	// TODO(Han): May be we  can just allocate this ourselves?
	BitmapHandle = CreateDIBSection( // Ask Windows to create Bitmap => Search for "CreateDIBSection"
		BitmapDeviceContext,
		&BitmapInfo, // We will fill this info
		DIB_RGB_COLORS, // iUsage, PAL or RGB => We use RGB
		&BitmapMemory, // **ppvBits,Pointer to Bits, Actual memory
		0, // hSection, Don't care
		0 // dwOffset, Referencing hSection
	);
}

internal void
Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
	StretchDIBits // For scale the window => Turns out to be bad call later => Search for StretchDIBITS
	(
		DeviceContext,
		X, Y, Width, Height, // Window we want to draw
		X, Y, Width, Height, // Buffer, Src, Backbuffer is the same size as Window
		BitmapMemory, // Pointer to Bits
		&BitmapInfo, // Pointer to Bitmap Info
		DIB_RGB_COLORS, // type of buffer (PAL or RGB) => Always RGB
		SRCCOPY // What kind of bit-wise operation? "SRCCOPY" (Directly copy) => Search for "BitBlt"
	);
}

// Window CALLBACK => "Search for WindowProc"
LRESULT CALLBACK
Win32MainWindowCallback(
	HWND   Window, //A handle to the window, telling the windows that which window we are talking about?
	UINT   Message, // Message that windows is asking us to handle "Search for System-Defined Messages"
	WPARAM WParam,
	LPARAM LParam)
{
	LRESULT Result = 0;

	switch (Message)
	{
	case WM_SIZE: // When the user changes size of the window
	{
		RECT ClientRect;
		GetClientRect(Window, &ClientRect); // Defines rectangle on the screen => Search for "GetClientRect"
		int Width = ClientRect.right - ClientRect.left; // Width
		int Height = ClientRect.bottom - ClientRect.top; // Height
		Win32ResizeDIBSection(Width, Height);
	} break;

	case WM_CLOSE: // When the user clicks "X" in the window
	{
		// PostQuitMessage(0); //Ver.1
		// DestroyWindow(Window); // Ver.2
		//TODO(HAN): Handle this with a message to the user?
		Running = false; // Ver.3
	} break;

	case WM_ACTIVATEAPP: // When the user clicks the window
	{
		OutputDebugStringA("WM_ACTIVATEAPP\n");
	} break;

	case WM_DESTROY: // When Windows deletes the window, We may want to treat this an error => Maybe Recreate window?
	{
		// PostQuitMessage(0); // Ver.2
		//TODO(HAN): Handle this as an error - recreate window?
		Running = false; // Ver.3
	} break;

	// Fill the Message on the window
	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(Window, &Paint);
		LONG X = Paint.rcPaint.left;
		LONG Y = Paint.rcPaint.top;
		LONG Width = Paint.rcPaint.right - Paint.rcPaint.left; // Width
		LONG Height = Paint.rcPaint.bottom - Paint.rcPaint.top; // Height
		Win32UpdateWindow(DeviceContext, X, Y, Width, Height); // Call
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
	WindowClass.lpfnWndProc = Win32MainWindowCallback; // pointer to a function, Search for "WindowProc"
	WindowClass.hInstance = Instance;
	// WindowClass.hIcon;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClassA(&WindowClass))
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
				0, // Parent Window (We don't need)
				0, // We don't need @Custom
				Instance, // Instance
				0 // passing parameter to window? No
			);
		if (WindowHandle)
		{
			Running = true; // set window running set "true"
			// Keep running until GetMessage returns false
			while (Running) // Message Queue
			{
				MSG Message;
				BOOL MessageResult = GetMessageA(&Message, 0, 0, 0); // Search for "GetMessage"
				if (MessageResult > 0)
				{
					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}
				else
				{
					break; // break out of for loop
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
	return(0); // Program end
}