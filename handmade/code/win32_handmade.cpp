/*
* @Author: Jin
* @Date:   2020-11-14 20:17:42
* @Last Modified by:   Jin
* @Last Modified time: 2020-11-17 19:21:06
*/
#include <windows.h>
#include <stdint.h>

#define internal static // function is local to the file
#define local_persist static // persist, retain the value
#define global_variable static // Not preferred method, temporary

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

//TODO(Han):this is global for now (sometimes something will popup when user tries to close. so we can implement this in different way)
global_variable bool Running; // Ver.3

global_variable BITMAPINFO BitmapInfo; // We only care about bmiHeader
global_variable void* BitmapMemory; // "Actual memory" we are going to receive back to window. we used void because we don't know format of the things to which it is pointing to, we will cast it for the format we actually want to use
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal void
RenderWeiredGradient(int BlueOffset, int GreenOffset)
{
  int Width = BitmapWidth;
  int Height = BitmapHeight;
// Draw Window
  int Pitch = Width*BytesPerPixel; // Difference between row and next row, How big each individual row
  uint8 *Row = (uint8 *)BitmapMemory;
  for(int Y = 0;
      Y< BitmapHeight;
      ++Y)
  {
    uint32 *Pixel = (uint32 *)Row;
    for(int X = 0;
            X < BitmapWidth;
            ++X)
    {
      //
      uint8 Blue = (X + BlueOffset);
      uint8 Green = (Y + GreenOffset);

    /*
      Memory:   BB GG RR xx (Blue 8bit, Green 8bit, Red 8bit, xx 8bit)
      Register: xx RR GG BB => Little Endian Load
      Pixel (32-bits)
    */
    *Pixel++ = ((Green << 8) | Blue << 0); // Access Memory pointed to Pixel => @Custom
    }

  Row += Pitch;
  }
}

//Displaying GDI
internal void
Win32ResizeDIBSection(int Width, int Height) // Device Independent Bitmap => Search for "ResizeDIBSection"
{
// Free Memory
  if (BitmapMemory)
  {
    VirtualFree(BitmapMemory,
                0,
                MEM_RELEASE // Relase memory to OS
                );
  }

  BitmapWidth = Width;
  BitmapHeight = Height;
	// Allocate BITMAP (// Search for "BITMAPINFO")
	// Search for "BITMAPINFOHEADER"
	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader); // # Bytes => How big is the structure actually is? find out structure itself by using "sizeof", should not include color table "bmiColors" so you should not use sizeof(BitmapInfo);
	BitmapInfo.bmiHeader.biWidth = BitmapWidth; // Width
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight; // Height, Row goes from Top-Down (-BitmapHeight)
	BitmapInfo.bmiHeader.biPlanes = 1; // Always 1
	BitmapInfo.bmiHeader.biBitCount = 32; // # of bit per pixel, 8 bit for red,green,blue each + @
	BitmapInfo.bmiHeader.biCompression = BI_RGB; // We don't want compression

  // NOTE(HAN): Clarifying the deal with StretchDITits & BitBlt
  // No more DC for us.
  int BitmapMemorySize = (BitmapWidth*BitmapHeight)*BytesPerPixel; // Size of BitmapMemory
  BitmapMemory = VirtualAlloc(0, // We don't care where the memory is
                              BitmapMemorySize,
                              MEM_COMMIT, // Tell Windows that we will use the memory from
                              PAGE_READWRITE // READ & Write Access to memory
                              ); // Allocates a certain # of mamory pages

  // TODO(Han): Probably clear this to black
}

// WINDOWS asks us to repaint
internal void
Win32UpdateWindow(HDC DeviceContext,
                  RECT *ClientRect, // Size of window RECT
                  int X, int Y, int Width, int Height)
{
  int WindowWidth = ClientRect->right - ClientRect->left;
  int WindowHeight = ClientRect->bottom - ClientRect->top;
	StretchDIBits // For scale the window (gdi32.lib) => Turns out to be bad call later => Search for "StretchDIBITS"
	(
		DeviceContext,
    /*
		X, Y, Width, Height, // Window we want to draw
		X, Y, Width, Height, // Buffer, Src, Backbuffer is the same size as Window
    */
    0, 0, BitmapWidth, BitmapHeight, // Whole Bitmap
    0, 0, WindowWidth, WindowHeight, // to whole window
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
		//TODO(HAN): Handle this with a message to the user?
		Running = false; // Ver.3
	} break;

	case WM_ACTIVATEAPP: // When the user clicks the window
	{
		OutputDebugStringA("WM_ACTIVATEAPP\n");
	} break;

	case WM_DESTROY: // When Windows deletes the window, We may want to treat this an error => Maybe Recreate window?
	{
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

		RECT ClientRect;
    GetClientRect(Window, &ClientRect);

		Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height); // Call
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
		HWND Window =
			CreateWindowExA(
				0, // Extended Style
				WindowClass.lpszClassName, // Class Name
				"Handmade Hero",  // Window Name
				WS_OVERLAPPEDWINDOW|WS_VISIBLE, // Window Style
				CW_USEDEFAULT,// X
				CW_USEDEFAULT,// Y
				CW_USEDEFAULT, // Width
				CW_USEDEFAULT, // Height
				0, // Parent Window (We don't need)
				0, // We don't need @Custom
				Instance, // Instance
				0 // passing parameter to window? No
			);
		if (Window)
		{
        int XOffset = 0;
        int YOffset = 0;
			Running = true; // set window running set "true"
			// Keep running until GetMessage returns false
			while (Running) // Message Queue
			{

        MSG Message;
         while(PeekMessageA(
           &Message, // Address of Message
           0, // Window Handle
           0, // FIlter
           0, // Filter
           PM_REMOVE // Queue
            ))
         {
          if(Message.message == WM_QUIT)
          {
            Running = false;
          }
          //Process the queue
          TranslateMessage(&Message);
          DispatchMessageA(&Message);
         }

           RenderWeiredGradient(XOffset, YOffset);

           HDC DeviceContext = GetDC(Window);
           RECT ClientRect;
           GetClientRect(Window, &ClientRect);
           int WindowWidth = ClientRect.right - ClientRect.left; // Not a pointer
          int WindowHeight = ClientRect.bottom - ClientRect.top;
           Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight); // Call
           ReleaseDC(Window, DeviceContext);
           ++XOffset; // @Custom => Animation Speed in X-axis
		   // YOffset += 5; // @Custom => Animation Speed in Y-axis
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