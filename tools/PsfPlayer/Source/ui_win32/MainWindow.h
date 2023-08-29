#pragma once

#include <deque>
#include "filesystem_def.h"
#include "../PsfVm.h"
#include "../PsfBase.h"
#include "../PsfTags.h"
#include "../Playlist.h"
#include "win32/Dialog.h"
#include "win32/Static.h"
#include "win32/Button.h"
#include "win32/Layouts.h"
#include "win32/TrayIconServer.h"
#include "win32/ToolTip.h"
#include "win32/TaskBarList.h"
#include "win32/GdiObj.h"
#include "PlaylistPanel.h"
#include "FileInformationPanel.h"
#include "SpuRegViewPanel.h"
#include "AcceleratorTable.h"
#include "PlaylistDiscoveryService.h"

class CMainWindow : public Framework::Win32::CDialog
{
public:
	CMainWindow(CPsfVm&);
	virtual ~CMainWindow();

	void Run();

protected:
	LRESULT OnWndProc(unsigned int, WPARAM, LPARAM) override;
	long OnCommand(unsigned short, unsigned short, HWND) override;
	long OnSize(unsigned int, unsigned int, unsigned int) override;
	long OnTimer(WPARAM) override;
	long OnClose() override;

private:
	enum
	{
		MAX_PANELS = 4,
	};

	enum
	{
		TIMER_UPDATE_CLOCK,
		TIMER_UPDATE_FADE,
		TIMER_UPDATE_DISCOVERIES,
	};

	enum REPEAT_MODE
	{
		PLAYLIST_ONCE,
		PLAYLIST_REPEAT,
		PLAYLIST_SHUFFLE,
		TRACK_REPEAT
	};

	struct SOUNDHANDLER_INFO
	{
		int id;
		const TCHAR* name;
		const TCHAR* dllName;
	};

	struct CHARENCODING_INFO
	{
		CPsfTags::CHAR_ENCODING id;
		const TCHAR* name;
	};

	CSoundHandler* CreateHandler(const TCHAR*);

	void OnNewFrame();

	void OnFileOpen();
	void OnPause();
	void OnPrev();
	void OnNext();
	void OnPrevPanel();
	void OnNextPanel();
	void OnAbout();
	void OnRepeat();
	void OnConfig();

	static LRESULT CALLBACK MessageHookProc(int, WPARAM, LPARAM);

	bool PlayFile(const CPsfPathToken&, const fs::path&);
	void LoadSingleFile(const fs::path&);
	void LoadPlaylist(const fs::path&);
	void LoadArchive(const fs::path&);

	void OnPlaylistItemDblClick(unsigned int);
	void OnPlaylistAddClick();
	void OnPlaylistRemoveClick(unsigned int);
	void OnPlaylistSaveClick();

	void OnClickReverbEnabled();

	void OnTrayIconEvent(Framework::Win32::CTrayIcon*, LPARAM);
	void DisplayTrayMenu();
	void UpdateConfigMenu();
	void UpdateClock();
	void UpdateFade();
	void UpdateTitle();
	void UpdatePlaybackButtons();
	void UpdateRepeatButton();

	void Reset();
	void ActivatePanel(unsigned int);

	void CreateAudioPluginMenu();
	void UpdateAudioPluginMenu();
	void ChangeAudioPlugin(unsigned int);
	void LoadAudioPluginPreferences();
	int FindAudioPlugin(unsigned int);

	void CreateCharEncodingMenu();
	void UpdateCharEncodingMenu();
	void ChangeCharEncoding(unsigned int);
	void LoadCharEncodingPreferences();
	static CPsfTags::CHAR_ENCODING FindCharEncoding(unsigned int);

	HACCEL CreateAccelerators();
	void CreateSymbolFonts();

	static uint32 GetNextRandomNumber(uint32);
	static uint32 GetPrevRandomNumber(uint32);

	Framework::Win32::CStatic m_timerLabel;
	Framework::Win32::CStatic m_titleLabel;

	Framework::Win32::CButton m_repeatButton;
	Framework::Win32::CButton m_configButton;

	Framework::Win32::CStatic m_placeHolder;

	Framework::Win32::CButton m_pauseButton;

	Framework::Win32::CToolTip* m_toolTip;

	bool m_useTrayIcon;

	Framework::Win32::CTrayIconServer* m_trayIconServer;
	Framework::Win32::CTaskBarList* m_taskBarList;

	Framework::Win32::CWindow* m_panels[MAX_PANELS];
	CPlaylistPanel* m_playlistPanel;
	CFileInformationPanel* m_fileInformationPanel;
	CSpuRegViewPanel* m_spu0RegViewPanel;
	CSpuRegViewPanel* m_spu1RegViewPanel;

	HMENU m_trayPopupMenu;
	HMENU m_configPopupMenu;

	static HHOOK g_messageFilterHook;
	static HWND g_messageFilterHookWindow;

	Framework::Win32::CAcceleratorTable m_accel;

	HICON m_playListOnceIcon;
	HICON m_repeatListIcon;
	HICON m_shuffleListIcon;
	HICON m_repeatTrackIcon;
	HICON m_configIcon;
	HICON m_playIcon;
	HICON m_pauseIcon;
	HICON m_prevTrackIcon;
	HICON m_nextTrackIcon;

	CPsfVm& m_virtualMachine;
	CPsfTags m_tags;
	CPlaylist m_playlist;
	CPlaylistDiscoveryService m_playlistDiscoveryService;
	unsigned int m_currentPlaylistItem;
	unsigned int m_currentPanel;
	bool m_ready;
	uint64 m_frames;
	uint64 m_lastUpdateTime;
	uint64 m_trackLength;
	uint64 m_fadePosition;
	float m_volumeAdjust;
	int m_selectedAudioPlugin;
	CPsfTags::CHAR_ENCODING m_selectedCharEncoding;
	REPEAT_MODE m_repeatMode;
	bool m_reverbEnabled;
	uint32 m_randomSeed;
	Framework::Win32::CFont m_webdingsFont;
	Framework::Win32::CFont m_segoeUiSymbolFont;

	static SOUNDHANDLER_INFO m_handlerInfo[];
	static CHARENCODING_INFO m_charEncodingInfo[];

	Framework::CSignal<void()>::Connection m_OnNewFrameConnection;
	CPlaylistPanel::OnItemDblClickEvent::Connection m_OnItemDblClickConnection;
	CPlaylistPanel::OnAddClickEvent::Connection m_OnAddClickConnection;
	CPlaylistPanel::OnRemoveClickEvent::Connection m_OnRemoveClickConnection;
	CPlaylistPanel::OnSaveClickEvent::Connection m_OnSaveClickConnection;
};
