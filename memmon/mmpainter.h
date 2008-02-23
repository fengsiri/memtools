#ifndef MMPAINTER_H
#define MMPAINTER_H

// Copyright (c) 2007,2008 Charles Bailey

#include <windows.h>
#include <vector>
#include <iosfwd>
#include <memory>
#include <fstream>
#include "memorymap.h"
#include "mmsource.h"

#ifdef _DEBUG
#define MEMMON_DEBUG
#endif

class MMPrefs;

class MMPainter
{
public:
	MMPainter(int r, MMPrefs* p);
	~MMPainter();

	void Paint(HDC hdc, PAINTSTRUCT* ps);

	void SetProcessId(int p);
	void Run( const TCHAR* c, const TCHAR* wd, const TCHAR* a );
	void Update();

	const RECT& GetRect() const { return rsize; }

	void Snapshot(HWND hwnd) const;
	void Snapshot( const char* fname ) const;
	void Read(HWND hwnd);
	void Read( const char* fname );

	bool Record(HWND hwnd);
	void Record( const char* fname );

private:
	class Recorder
	{
	public:
		virtual ~Recorder() {}
		virtual void Record( const MemMon::MemoryMap&, const MemMon::MemoryMap& ) = 0;
	};

	class FStreamRecorder : public Recorder
	{
	public:
		FStreamRecorder( const char* fname, const MemMon::MemoryMap& mm );
		~FStreamRecorder();
		void Record( const MemMon::MemoryMap&, const MemMon::MemoryMap& );

	private:
		std::filebuf _buf;
#ifdef MEMMON_DEBUG
		std::string _fname;
		int _count;
#endif
	};

	class PlaybackSource : public MemMon::Source
	{
	public:
		PlaybackSource( const char* fname, MemMon::MemoryMap& mm );
		~PlaybackSource();

		size_t Update( MemMon::MemoryMap& );
		double Poll( const MemMon::CPUPrefs& prefs );
		double GetPos() const;

	private:
		std::filebuf _buf;
	};

	void DisplayGauge(HDC hdc, bool bQuick) const;
	void DisplayBlobs(HDC hdc) const;
	void DisplayTotals(HDC hdc, int offset) const;

	void MemPaint(HDC hdc) const;
	COLORREF GetColour(std::vector<MemMon::Region>::const_iterator& preg
		, std::vector<MemMon::Region>::const_iterator rend, size_t base
		, size_t end) const;

	COLORREF GetBlobColour(const MemMon::FreeRegion& reg) const;

	MemMon::MemoryMap mem;
	MemMon::MemoryMap _memprev;

	HBRUSH hBrush;
	HBRUSH hWBrush;
	HPEN hPen;
	HDC hMemDC;
	HBITMAP hBmp;
	HGDIOBJ hOldBmp;
	RECT rsize;

	const int radius;
	const double dradius;
	const int width;

	size_t maxaddr;

	double next_update;
	double processor_count;

	MMPrefs* pPrefs;

	std::auto_ptr< MemMon::Source > _source;
	std::auto_ptr< Recorder > _recorder;
};

#endif//MMPAINTER_H
