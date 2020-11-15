/*
* @Author: Jin
* @Date:   2020-11-14 20:17:42
* @Last Modified by:   Jin
* @Last Modified time: 2020-11-14 20:21:33
*/
#include <windows.h>

// Entry Point => Search for "WinMain"
int CALLBACK WinMain(HINSTANCE hInstance,
  					 HINSTANCE hPrevInstance,
   					 LPSTR lpCmdLine,
   					 int nCmdShow)
{
	// Search "MessageBox function"
	MessageBoxA(0, "This is handmad hero",
		"Handmade Hero", MB_OK|MB_ICONINFORMATION);

	return(0);
}