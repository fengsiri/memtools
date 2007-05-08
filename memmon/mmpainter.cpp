#include "stdafx.h"
#include "mmpainter.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include "hrfmt.h"

using std::cos;
using std::sin;
using std::vector;
using std::copy;

MMPainter::MMPainter(int r)
: hMemDC(NULL), hBmp(NULL), hOldBmp(NULL), hProc(NULL)
, radius(r), dradius(r), width(r * 3 /  5), maxaddr(0x1000000u)
, next_update(0.0)
{
	hBrush = CreateSolidBrush(RGB(255, 0, 0));
	hWBrush = CreateSolidBrush(RGB(255, 255, 255));
	hPen = CreatePen(PS_SOLID, 0, RGB(255, 255, 255));

	rsize.left = 0;
	rsize.top = 0;
	rsize.right = radius << 1;
	rsize.bottom = radius + 180 + 16;

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	maxaddr = (size_t)sysinfo.lpMaximumApplicationAddress;
	processor_count = (double)sysinfo.dwNumberOfProcessors;
}

MMPainter::~MMPainter()
{
	if (hProc != NULL)
	{
		CloseHandle(hProc);
	}

	if (hBmp != NULL)
	{
		if (hMemDC != NULL)
		{
			SelectObject(hMemDC, hOldBmp);
		}
		DeleteObject(hBmp);
	}
	if (hMemDC != NULL)
	{
		DeleteDC(hMemDC);
	}

	DeleteObject(hPen);
	DeleteObject(hWBrush);
	DeleteObject(hBrush);
}

void MMPainter::Paint(HDC hdc, PAINTSTRUCT* ps)
{
	if (hMemDC == NULL)
	{
		hMemDC = CreateCompatibleDC(hdc);
		hBmp = CreateCompatibleBitmap(hdc, rsize.right, rsize.bottom);
		hOldBmp = SelectObject(hMemDC, hBmp);

		SetBkColor(hMemDC, RGB(255, 255, 255));
	}

	int x, y, w, h;
	x = max(ps->rcPaint.left, 0);
	y = max(ps->rcPaint.top, 0);
	w = min(ps->rcPaint.right - x, (rsize.right) - x);
	h = min(ps->rcPaint.bottom - y, (rsize.bottom) - y);
	BitBlt(hdc, x, y, w, h, hMemDC, x, y, SRCCOPY);
}

void MMPainter::MemPaint(HDC hdc) const
{
	FillRect(hdc, &rsize, hWBrush);
	DisplayGauge(hdc, false);
	//HGDIOBJ oldPen = SelectObject(hdc, hPen);
	//HGDIOBJ oldBrush = SelectObject(hdc, hWBrush);
	//int ww = (radius << 1) - width;
	//Pie(hdc, width, width, ww, ww, ww, radius, width, radius);
	//SelectObject(hdc, oldPen);
	//Arc(hdc, width, width, ww, ww, ww, radius, width, radius);
	//Arc(hdc, 0, 0, radius << 1, radius << 1, radius << 1, radius, 0, radius);
	//SelectObject(hdc, oldBrush);
	DisplayBlobs(hdc);
	DisplayTotals(hdc, 280);
}


COLORREF MMPainter::GetColour(std::vector<Mem::Region>::const_iterator& reg
							, std::vector<Mem::Region>::const_iterator rend
							, size_t base, size_t end) const
{
	COLORREF c = RGB(0, 0, 0);

	if (reg != rend)
	{
		while (reg != rend && (reg->base + reg->size) < base)
		{
			++reg;
		}

		size_t tmp = 0, tmp2 = 0;
		int max_type = 0;

		std::vector<Mem::Region>::const_iterator oreg = reg;
		while (reg != rend && reg->base < end)
		{
			size_t sbegin = max(base, reg->base);
			size_t send = min(end, reg->base + reg->size);

			tmp += (send - sbegin) * reg->type;
			tmp2 += (send - sbegin) * max(reg->type, 1);

			if (reg->type > max_type) max_type = reg->type;

			oreg = reg;
			++reg;
		}
		// Reset back to the last interval so that the overlaps match up
		reg = oreg;

		if (max_type > 0) tmp = tmp2;

		if (tmp > end - base)
		{
			c = RGB(255, 255 - ((tmp - (end - base)) * 255) / (end - base), 0);
		}
		else
		{
			c = RGB((tmp * 255) / (end - base), 255, 0);
		}
	}

	return c;
}

void MMPainter::DisplayGauge(HDC hdc, bool bQuick) const
{
	double pi = acos(-1.0);

	if (!bQuick)
	{
		double dstart = pi;
		double ddiff = dstart / (dradius*2.0);
		double dnext;

		double addrmax = double(maxaddr);

		vector<Mem::Region>::const_iterator pReg = mem.blocklist.begin();

		HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(DC_BRUSH));
		HGDIOBJ hOldPen = SelectObject(hdc, GetStockObject(DC_PEN));
		for (double d = dstart; d > ddiff; d = dnext)
		{
			dnext = d - ddiff;

			size_t base = size_t((pi - d) * addrmax / pi);
			size_t end = size_t((pi - dnext) * addrmax / pi);

			COLORREF c = GetColour(pReg, mem.blocklist.end(), base, end);
			SetDCBrushColor(hdc, c);
			SetDCPenColor(hdc, c);
			int x1 = radius + int(dradius * cos(dnext));
			int y1 = radius - int(dradius * sin(dnext));
			int x2 = radius + int(dradius * cos(d));
			int y2 = radius - int(dradius * sin(d));
			Pie(hdc, 0, 0, radius << 1, radius << 1, x1, y1, x2, y2);

		}
		SelectObject(hdc, hOldPen);
		SelectObject(hdc, hOldBrush);
	}
	//
	HGDIOBJ hOldPen = SelectObject(hdc, hPen);
	HGDIOBJ hOldBrush = SelectObject(hdc, hWBrush);
	int ww = (radius << 1) - width;
	Pie(hdc, width, width, ww, ww, ww, radius, width, radius + 1);
	SelectObject(hdc, hOldPen);
	Arc(hdc, width, width, ww, ww, ww, radius, width, radius + 1);
	Arc(hdc, 0, 0, radius << 1, radius << 1, radius << 1, radius, 0, radius + 1);
	//SelectObject(hdc, oldBrush);
	//

	double dpos = cpup.GetPos() * pi / processor_count;

	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SelectObject(hdc, GetStockObject(DC_PEN));
	SetDCBrushColor(hdc, RGB(127, 0, 0));
	SetDCPenColor(hdc, RGB(127, 0, 0));
	
	int x1 = radius - int(dradius * cos(dpos));
	int y1 = radius - int(dradius * sin(dpos));
	int x2 = 0;
	int y2 = radius;

	if (y1 == radius) y1 -= 1;

	Pie(hdc, width + 3, width + 3, ww - 3, ww - 3, x1, y1, x2, y2);
	
	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
}

COLORREF MMPainter::GetBlobColour(const Mem::FreeRegion& reg) const
{
	int c = int(511.0 * double(reg.size + reg.base)/double(maxaddr));
	int r, g, b;
	if (c < 256)
	{
		r = 255;
		g = 255 - c;
		b = c;
	}
	else
	{
		r = 511 - c;
		g = c - 256;
		b = 255;
	}

	return RGB(r, g, b);
}

void MMPainter::DisplayBlobs(HDC hdc) const
{
/*	if (!mem.freelist.empty())
	{
		HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(DC_BRUSH));
		HGDIOBJ hOldPen = SelectObject(hdc, GetStockObject(DC_PEN));
		const double scale = double(0x1000000);
		const double scale2 = log(double(maxaddr) / scale);

		SetDCPenColor(hdc, RGB(0, 0, 0));

		for (int j = 0; j < int(mem.freelist.size()); ++j)
		{
			const Mem::FreeRegion& r = mem.freelist[j];
			if (r.size == 0) break;
			BYTE c = BYTE(255 * double(r.size + r.base)/double(maxaddr));
			SetDCBrushColor(hdc, RGB(255 - c, c, 255));

			int size = int(log(double(r.size) / scale) / scale2 * dradius);
			if (size > 0)
			{
				Ellipse(hdc, radius - size, radius + 20 * j, radius + size
													, radius + 20 * (j + 1));
			}
		}

		SelectObject(hdc, hOldPen);
		SelectObject(hdc, hOldBrush);
	}*/
	int currentline = 1;
	int currentpos = 0;

	HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(DC_BRUSH));
	HGDIOBJ hOldPen = SelectObject(hdc, GetStockObject(DC_PEN));
	SetDCPenColor(hdc, RGB(0, 0, 0));

	for (vector<Mem::FreeRegion>::const_iterator k = mem.freelist.begin();
			k != mem.freelist.end(); ++k)
	{
		double width = dradius * 8.0 * double(k->size) / double(maxaddr);
		int nwidth = int(width);

		if (nwidth < 6) break;

		if (currentpos > 0 && nwidth + currentpos > (radius << 1))
		{
			++currentline;
			currentpos = 0;
		}

		if (nwidth > (radius << 2)) nwidth = radius << 2;

		//BYTE c = BYTE(255 * double(k->size + k->base)/double(maxaddr));
		SetDCBrushColor(hdc, GetBlobColour(*k));
		Ellipse(hdc, currentpos, radius + 20 * currentline, currentpos + nwidth
											, radius + 20 * (currentline + 1));

		currentpos += nwidth + 1;

	}

	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
}

void MMPainter::DisplayTotals(HDC hdc, int offset) const
{
	COLORREF colcomm = RGB(170, 0, 0);
	COLORREF colresv = RGB(170, 170, 0);
	COLORREF colfree = RGB(0, 170, 0);

	COLORREF txtcol = GetTextColor(hdc);

	_TCHAR buf[100];
	RECT rtmp;

	TEXTMETRIC txtmet;
	GetTextMetrics(hdc, &txtmet);

	rtmp.left = 0;
	rtmp.top = offset;
	rtmp.right = rsize.right / 4;
	rtmp.bottom = rtmp.top + txtmet.tmHeight;

	HRFormat::hr_format(buf, 100, mem.total_commit[0] + mem.total_commit[1] + mem.total_commit[2] + mem.total_commit[3]);
	SetTextColor(hdc, colcomm);
	DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);

	rtmp.left = rtmp.right;
	rtmp.right += rsize.right / 4;

	HRFormat::hr_format(buf, 100, mem.total_reserve[0] + mem.total_reserve[1] + mem.total_reserve[2] + mem.total_reserve[3]);
	SetTextColor(hdc, colresv);
	DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);

	rtmp.left = rtmp.right;
	rtmp.right += rsize.right / 4;

	HRFormat::hr_format(buf, 100, mem.total_free);
	SetTextColor(hdc, colfree);
	DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);

	if (!mem.freelist.empty())
	{
		const Mem::FreeRegion& fr = mem.freelist.front();

		rtmp.left = rtmp.right;
		rtmp.right += rsize.right / 4;

		COLORREF bcol = GetBlobColour(fr);
		COLORREF bcol2 = RGB(GetRValue(bcol)*2/3, GetGValue(bcol)*2/3
												, GetBValue(bcol)*2/3);
		HRFormat::hr_format(buf, 100, fr.size);
		SetTextColor(hdc, bcol2);
		DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);
	}

	SetTextColor(hdc, txtcol);
/*
	rtmp.left = 0;
	rtmp.top = rtmp.bottom;
	rtmp.right = rsize.right / 4;
	rtmp.bottom = rtmp.top + txtmet.tmHeight;

	HRFormat::hr_format(buf, 100, mem.total_commit[0]);
	DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);

	rtmp.left = rtmp.right;
	rtmp.right += rsize.right / 4;

	HRFormat::hr_format(buf, 100, mem.total_reserve[0]);
	DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);

	rtmp.left = rtmp.right;
	rtmp.right += rsize.right / 4;

	HRFormat::hr_format(buf, 100, mem.total_commit[1]);
	DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);

	rtmp.left = rtmp.right;
	rtmp.right += rsize.right / 4;

	HRFormat::hr_format(buf, 100, mem.total_reserve[1]);
	DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);
	rtmp.left = 0;
	rtmp.top = rtmp.bottom;
	rtmp.right = rsize.right / 4;
	rtmp.bottom = rtmp.top + txtmet.tmHeight;

	HRFormat::hr_format(buf, 100, mem.total_commit[2]);
	DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);

	rtmp.left = rtmp.right;
	rtmp.right += rsize.right / 4;

	HRFormat::hr_format(buf, 100, mem.total_reserve[2]);
	DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);
*/
}

void MMPainter::SetProcessId(int p)
{
	if (hProc != NULL)
	{
		CloseHandle(hProc);
	}
	hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, p);
}

void MMPainter::Update()
{
	//HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, procid);
	if (hProc != NULL)
	{
		double ctime = cpup.Poll(hProc);

		if (ctime > next_update)
		{
			maxaddr = mem.Populate(hProc);

			if (hMemDC != NULL)
			{
				MemPaint(hMemDC);
			}
			next_update = ctime + 0.999;
		}
		else
		{
			if (hMemDC != NULL)
			{
				DisplayGauge(hMemDC, true);
			}
		}
	}
}

MMPainter::Mem::Mem()
{
}

MMPainter::Mem::~Mem()
{
}

size_t MMPainter::Mem::Populate(HANDLE hProc)
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	size_t max_addr = (size_t)sysinfo.lpMaximumApplicationAddress;

	Region r;

	freelist.resize(50);
	blocklist.clear();

	total_free = 0;
	total_reserve[0] = 0;
	total_commit[0] = 0;
	total_reserve[1] = 0;
	total_commit[1] = 0;
	total_reserve[2] = 0;
	total_commit[2] = 0;
	total_reserve[3] = 0;
	total_commit[3] = 0;

	for (vector<FreeRegion>::iterator i = freelist.begin();
									i != freelist.end(); ++i)
	{
		i->size = 0;
	}

	MEMORY_BASIC_INFORMATION meminfo;

	for (char* p = (char*)sysinfo.lpMinimumApplicationAddress;
		p < (char*)sysinfo.lpMaximumApplicationAddress;
		p += sysinfo.dwPageSize)
	{
		VirtualQueryEx(hProc, p, &meminfo, sizeof(meminfo));

		if (p != meminfo.BaseAddress) break;

		r.base = (size_t)meminfo.BaseAddress;
		r.size = meminfo.RegionSize;
		switch (meminfo.State)
		{
		case MEM_FREE:
			r.type = 0;
			total_free += r.size;
			break;
		case MEM_RESERVE:
			r.type = 1;
			switch (meminfo.Type)
			{
			case MEM_IMAGE:
				total_reserve[0] += r.size;
				break;
			case MEM_MAPPED:
				total_reserve[1] += r.size;
				break;
			case MEM_PRIVATE:
				total_reserve[2] += r.size;
				break;
			default:
				total_reserve[3] += r.size;
				break;
			}
			break;
		case MEM_COMMIT:
		default:
			r.type = 2;
			switch (meminfo.Type)
			{
			case MEM_IMAGE:
				total_commit[0] += r.size;
				break;
			case MEM_MAPPED:
				total_commit[1] += r.size;
				break;
			case MEM_PRIVATE:
				total_commit[2] += r.size;
				break;
			default:
				total_commit[3] += r.size;
				break;
			}
		}

		blocklist.push_back(r);

		if (meminfo.State == MEM_FREE)
		{
			if (freelist.back().size < meminfo.RegionSize)
			{
				int j;

				for(j = int(freelist.size()) - 2; j >= 0; --j)
				{
					if (freelist[j].size >= meminfo.RegionSize) break;
					freelist[j+1].size = freelist[j].size;
					freelist[j+1].base = freelist[j].base;
				}

				freelist[j+1].size = meminfo.RegionSize;
				freelist[j+1].base = size_t(meminfo.BaseAddress);
			}
		}

		if (meminfo.RegionSize > 0)
		{
			p += (meminfo.RegionSize - sysinfo.dwPageSize);
		}
	}

	max_addr = (size_t)meminfo.BaseAddress + meminfo.RegionSize;

	return max_addr;
}

MMPainter::CPUPerf::CPUPerf()
: actual_u(0.0)
, actual_k(0.0)
, ind_pos(0.0)
, ind_vel(0.0)
, last_poll(0.0)
{
}

inline double FT2dbl(LPFILETIME lpFt)
{
	__int64 tmp = (__int64(lpFt->dwHighDateTime) << 32) + __int64(lpFt->dwLowDateTime);
	return double(tmp) / 10000000.0;
}

double MMPainter::CPUPerf::Poll(HANDLE hProc)
{
	const double k = 2.0;
	const double delta_t = 0.1;
	const double damping = k * 2.0;
	FILETIME currtime;
	FILETIME sysidle, syskernel, sysuser;
	FILETIME proccreate, procexit, prockern, procuser;

	GetSystemTimeAsFileTime(&currtime);
	GetSystemTimes(&sysidle, &syskernel, &sysuser);
	GetProcessTimes(hProc, &proccreate, &procexit, &prockern, &procuser);

	double dtime = FT2dbl(&currtime);

	double new_u = FT2dbl(&procuser);
	double new_k = FT2dbl(&prockern);

	double cpufrac = ((new_u - actual_u) + (new_k - actual_k)) / (dtime - last_poll);

	if (last_poll > 0.0)
	{
		for (double time = last_poll; time < dtime; time += delta_t)
		{
			double daccel = k * (cpufrac - ind_pos) - damping * ind_vel;
			ind_vel += daccel * delta_t;
			ind_pos += ind_vel * delta_t;
		}
	}

	actual_u = new_u;
	actual_k = new_k;
	last_poll = dtime;

	return dtime;
}

double MMPainter::CPUPerf::GetPos() const
{
	return ind_pos;
}
