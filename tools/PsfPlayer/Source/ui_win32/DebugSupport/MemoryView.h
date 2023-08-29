#pragma once

#include "Types.h"
#include "win32/CustomDrawn.h"
#include "win32/DeviceContext.h"
#include "win32/GdiObj.h"
#include "signal/Signal.h"

class CMemoryView : public Framework::Win32::CCustomDrawn
{
public:
	CMemoryView(HWND, const RECT&);
	virtual ~CMemoryView() = default;

	void SetMemorySize(uint32);

	uint32 GetBytesPerLine() const;
	void SetBytesPerLine(uint32);

	void SetDisplayUnit(uint32);

	uint32 GetSelection();
	void SetSelectionStart(unsigned int);

	Framework::CSignal<void(uint32)> OnSelectionChange;

protected:
	enum
	{
		MAX_UNITS = 10,
		ID_MEMORYVIEW_COLUMNS_AUTO = 40001,
		ID_MEMORYVIEW_COLUMNS_16BYTES,
		ID_MEMORYVIEW_COLUMNS_32BYTES,
		ID_MEMORYVIEW_UNIT_BASE,
		ID_MEMORYVIEW_UNIT_MAX = ID_MEMORYVIEW_UNIT_BASE + MAX_UNITS,
		ID_MEMORYVIEW_MENU_MAX
	};

	virtual uint8 GetByte(uint32) = 0;
	virtual HMENU CreateContextualMenu();

	void Paint(HDC) override;

	long OnCommand(unsigned short, unsigned short, HWND) override;
	long OnSize(unsigned int, unsigned int, unsigned int) override;
	long OnVScroll(unsigned int, unsigned int) override;
	long OnSetFocus() override;
	long OnKillFocus() override;
	long OnMouseWheel(int, int, short) override;
	long OnLeftButtonDown(int, int) override;
	long OnLeftButtonUp(int, int) override;
	long OnRightButtonUp(int, int) override;
	long OnKeyDown(WPARAM, LPARAM) override;

	Framework::Win32::CFont m_font;

private:
	typedef std::tstring (CMemoryView::*UnitRenderer)(uint32);

	struct UNITINFO
	{
		unsigned int bytesPerUnit = 0;
		unsigned int charsPerUnit = 0;
		UnitRenderer renderer = nullptr;
		const TCHAR* description = nullptr;
	};

	struct RENDERMETRICS
	{
		unsigned int xmargin = 0;
		unsigned int ymargin = 0;
		unsigned int yspace = 0;
		unsigned int unitSpacing = 0;
		unsigned int lineSectionSpacing = 0;
	};

	struct RENDERPARAMS
	{
		unsigned int lines = 0;
		unsigned int totallyVisibleLines = 0;
		unsigned int bytesPerLine = 0;
		const UNITINFO* unit = nullptr;
		uint32 address = 0;
	};

	void UpdateScrollRange();
	unsigned int GetScrollOffset();
	unsigned int GetScrollThumbPosition();
	void UpdateCaretPosition();
	void EnsureSelectionVisible();
	RENDERPARAMS GetRenderParams();

	std::tstring RenderByteUnit(uint32);
	std::tstring RenderWordUnit(uint32);
	std::tstring RenderSingleUnit(uint32);

	static const UNITINFO g_units[];
	uint32 m_selectionStart = 0;
	uint32 m_size = 0;
	uint32 m_bytesPerLine = 0;
	uint32 m_selectedUnit = 0;
	RENDERMETRICS m_renderMetrics;
};
