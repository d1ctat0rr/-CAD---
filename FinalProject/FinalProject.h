// FinalProject.h : main header file for the FinalProject DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CFinalProjectApp
// See FinalProject.cpp for the implementation of this class
//

class CFinalProjectApp : public CWinApp
{
public:
	CFinalProjectApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
