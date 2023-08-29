#pragma once

#include <windows.h>
#include <mmsystem.h>
#include "..\SoundHandler.h"

class CSH_WaveOut : public CSoundHandler
{
public:
	CSH_WaveOut();
	virtual ~CSH_WaveOut();

	static CSoundHandler* HandlerFactory();

	void Reset() override;
	bool HasFreeBuffers() override;
	void RecycleBuffers() override;
	void Write(int16*, unsigned int, unsigned int) override;

private:
	enum
	{
		MAX_BUFFERS = 25,
	};

	void InitializeWaveOut();
	void DestroyWaveOut();

	WAVEHDR* GetFreeBuffer();
	void WaveOutProc(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR);
	static void CALLBACK WaveOutProcStub(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);

	HWAVEOUT m_waveOut;
	WAVEHDR m_buffer[MAX_BUFFERS];
	//	int16*					m_bufferMemory;
};
