// ZrxProject.cpp : Defines the initialization routines for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "FinalProject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//华南理工大学软件工程专业中望CAD实训课程结题项目————扫雷游戏-黄玉米版 SCUT ZWCAD Practical training courses Final project ——— MineClearance - HuangYuning version
//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CFinalProjectApp

BEGIN_MESSAGE_MAP(CFinalProjectApp, CWinApp)
END_MESSAGE_MAP()


// CFinalProjectApp construction

CFinalProjectApp::CFinalProjectApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CFinalProjectApp object

CFinalProjectApp theApp;


// CFinalProjectApp initialization

BOOL CFinalProjectApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
