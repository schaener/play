#pragma once

#include "win32/Window.h"
#include "win32/HorizontalSplitter.h"
#include "win32/VerticalSplitter.h"
#include "DisAsm.h"
#include "RegViewGeneral.h"
#include "MemoryViewMIPS.h"
#include "FunctionsView.h"
#include "VirtualMachine.h"
#include "Debuggable.h"

class CMiniDebugger : public Framework::Win32::CWindow
{
public:
	CMiniDebugger(CVirtualMachine&, const CDebuggable&);
	virtual ~CMiniDebugger();
	void Run();

protected:
	long OnCommand(unsigned short, unsigned short, HWND) override;

private:
	void CreateAccelerators();
	void StepCPU();
	void OnFunctionDblClick(uint32);
	void InitializeConsole();

	CVirtualMachine& m_virtualMachine;
	CDebuggable m_debuggable;
	Framework::Win32::CHorizontalSplitter* m_subSplitter;
	Framework::Win32::CVerticalSplitter* m_mainSplitter;
	CDisAsm* m_disAsmView;
	CRegViewGeneral* m_registerView;
	CMemoryViewMIPS* m_memoryView;
	CFunctionsView* m_functionsView;
	HACCEL m_acceleratorTable;

	Framework::CSignal<void(uint32)>::Connection m_OnFunctionDblClickConnection;
};
