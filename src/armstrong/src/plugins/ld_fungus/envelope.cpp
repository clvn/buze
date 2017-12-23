#ifdef WIN32
#pragma warning(disable: 4244)
#pragma warning(disable: 4786)
#endif

#include<math.h>
#include<stdio.h>
#include<string.h>

#ifdef WIN32
#include<windows.h>
#include<assert.h>
#endif

#include "envelope.h"

extern int envtimescale;

double slopefunction(double x)
{
	if(x > 0.0940573) {
		const double a = 0.978368;
		const double b = 0.275597;
		const double c = -1.86418;
		const double d = 7.28012;
		const double e = -16.2591;
		const double f = 19.3327;
		const double g = -9.4787;

		return (((((x * g + f) * x + e) * x + d) * x + c) * x + b) * x + a;
	} else {
		return (-0.00832509) / log(11.8443 * x + 1.00569) + 1.0038;
	}
}

void Envelope::reset(void) {
	selected.clear();
#ifdef _WIN32
	drag = 0;
	memset(&drag_start, 0, sizeof(POINT));
#endif
	memset(&data, 0, sizeof(EnvelopeData));

	data.nodes = 4;
	data.nodedata[0].x = 0;
	data.nodedata[0].y = 0;
	data.nodedata[0].x2 = 0;
	data.nodedata[0].y2 = 0;
	data.nodedata[0].slope = 32768;
	
	data.nodedata[1].x = 5;
	data.nodedata[1].y = 65535;
	data.nodedata[1].x2 = 0;
	data.nodedata[1].y2 = 65535;
	data.nodedata[1].slope = 32768;
	data.nodedata[1].mode = 1; // loopstart
	
	data.nodedata[2].x = 153;
	data.nodedata[2].y = 8000;
	data.nodedata[2].x2 = 0;
	data.nodedata[2].y2 = 8000;
	data.nodedata[2].slope = 20000;
	data.nodedata[2].mode = 2; // loopend
	
	data.nodedata[3].x = 250;
	data.nodedata[3].y = 0;
	data.nodedata[3].x2 = 0;
	data.nodedata[3].y2 = 0;
	data.nodedata[3].slope = 8000;

	scalex = 1000;
	viewx = 0;
}

double morph(double a, double b, double morph)
{
	return a + (b - a) * morph;
}

void TriggerEnvelope(Envelope *e, EnvelopeState *state, double m, bool rst, int samplerate)
{
	state->d.nodes = e->data.nodes;

	if(rst)
		state->lastvalue = morph((double)e->data.nodedata[0].y / 65535.0, (double)e->data.nodedata[0].y2 / 65535.0, m);

	int n = 0;
	int n2 = 0;
	int n3 = 0;

	for(int i = 1; i < e->data.nodes; i++) {
		
		n += (int)e->data.nodedata[i].x;
		n3 += (int)e->data.nodedata[i].x + (int)e->data.nodedata[i].x2;

		int t = morph(n, n3, m) - n2;
		if(t < 1) t = 1;

		n2 += t;

		state->d.nodedata[i].n = envtimescale * (t * samplerate) / 1000;
		state->d.nodedata[i].target = morph(e->data.nodedata[i].y / 65535.0, e->data.nodedata[i].y2 / 65535.0, m);

		// TODO: should precalc somewhere else

		double b = (double)e->data.nodedata[i].slope / 65.536;
		if(b < 2) b = 2;
		if(b > 980) b = 980;
		if(b == 500) b = 499;

		double a;

		if(b < 500) a = slopefunction(b * 0.001);
		else if(b > 500) a = 1.0 / slopefunction(1 - (b * 0.001));

		state->d.nodedata[i].mul = pow(a, 1000.0 / (double)state->d.nodedata[i].n);

		if(e->data.nodedata[i].mode == 1)
			state->loopstart = i;
		else if(e->data.nodedata[i].mode == 2)
			state->loopend = i;
	}

	state->curnode = 0;
	state->curtime = 0;
	state->isover = false;
}

void EnvelopeState::End(void)
{
	if(curnode < loopend) {
		curnode = loopend;
		curtime = 0;
	}
}

void linear_interpolate(float *d, float y1, float y2, int t)
{
	y2 = (y2 - y1) / (float)t;

	while(t--) {
		*d = y1;
		y1 += y2;
		d++;
	}
}

float EnvelopeState::get(int ns, int fill)
{
	int i = 0;

	while(ns) {
		if(curnode >= d.nodes) {
			if(fill) memset(shape + i, 0, ns * 4);
			return 0;
		}

		if(curtime > 0) {
			int l;

			double nextvalue;

			if(curtime < ns) {
				nextvalue = value * pow(mul, curtime);
				l = curtime;
				curtime = 0;
				ns -= l;
			} else {
				if(ns == 64)
					nextvalue = value * mul64;
				else nextvalue = value * pow(mul, ns);
				curtime -= ns;
				l = ns;
				ns = 0;
			}

			float newvalue = nextvalue * scale + offset;
			if(fill) {
				if(fabs(newvalue - lastvalue) > 0.1) {
					nextvalue = value;
					for(int x = 0; x < l; x++) {
						shape[x+i] = nextvalue * scale + offset;
						nextvalue = nextvalue * mul;
					}
				} else {
					linear_interpolate(shape + i, lastvalue, newvalue, l);
				}
			}
			i += l;
			lastvalue = newvalue;
			value = nextvalue;
		}

		if(curtime == 0) {

			curnode++;
			if(curnode >= d.nodes) {
				isover = true;
				lastvalue = 0;
				curtime = 0x7FFFFFFF;
				if(fill) memset(shape + i, 0, ns * 4);
				return 0;
			}

			value = 1;
			mul = d.nodedata[curnode].mul;
			mul64 = pow(mul, 64);

			curtime = d.nodedata[curnode].n;
			double y_n = pow(d.nodedata[curnode].mul, (double)curtime);
			
			scale = (double)(lastvalue - d.nodedata[curnode].target) / (1.0 - y_n);
			offset = d.nodedata[curnode].target - scale * y_n;

			if(curnode == loopend) curnode = loopstart;
		}
	}

	return lastvalue;
}

bool EnvelopeCanDelete(Envelope *e, int node)
{
	if(node == 0) return false;
	if(e->data.nodedata[node].mode != 0) return false;

	return true;
}

void EnvelopeDeleteNode(Envelope *e, int node)
{
	e->data.nodes--;
	e->data.nodedata[node + 1].x += e->data.nodedata[node].x;
	e->data.nodedata[node + 1].xo += e->data.nodedata[node].xo;
	memmove(&e->data.nodedata[node], &e->data.nodedata[node + 1], sizeof(Node) * (200 - node - 1));
}

void EnvelopeAddNode(Envelope *e, double x, double y)
{
	int i;

	double sum = 0;
	for(i = 0; i < e->data.nodes; i++) {
		if(x < sum + e->data.nodedata[i].x) {
			memmove(&e->data.nodedata[i + 1], &e->data.nodedata[i], sizeof(Node) * (e->data.nodes - i));
			double newx = x - sum;
			e->data.nodedata[i + 1].x -= newx;
			e->data.nodedata[i + 1].xo -= newx;
			if(newx < 1) newx = 1;
			if(newx > 65535) newx = 65535;
			e->data.nodedata[i].x = newx;
			e->data.nodedata[i].y = y;
			e->data.nodedata[i].xo = newx;
			e->data.nodedata[i].yo = y;
			e->data.nodedata[i].x2 = 0;
			e->data.nodedata[i].y2 = y;
			e->data.nodedata[i].mode = 0;
			e->data.nodedata[i].slope = 32768;
			e->data.nodes++;
			return;
		}

		sum += e->data.nodedata[i].x;
	}

	double newx = x - sum;
	if(newx < 1) newx = 1;
	if(newx > 65535) newx = 65535;
	e->data.nodedata[i].x = newx;
	e->data.nodedata[i].y = y;
	e->data.nodedata[i].xo = newx;
	e->data.nodedata[i].yo = y;
	e->data.nodedata[i].x2 = 0;
	e->data.nodedata[i].y2 = y;
	e->data.nodedata[i].mode = 0;
	e->data.nodedata[i].slope = 32768;
	e->data.nodes++;
	return;
}

#ifdef WIN32

Envelope *InitEnvelope(Envelope *e)
{
	e->reset();

	return e;
}

void DestroyEnvelope(Envelope *e)
{
	assert(e);
	if(e->wnd) DestroyWindow(e->wnd);
}

void TraceEnvelopeCurve(HDC hdc, int x1, int y1, int x2, int y2, unsigned short slope)
{
	if(x2 < x1) return;
	if(x2 == x1 || slope == 0) {
		MoveToEx(hdc, x1, y1, 0);
		LineTo(hdc, x2, y2);
		return;
	}

	int n = x2 - x1;

	double b = (double)slope / 65.536;
	if(b < 2) b = 2;
	if(b > 980) b = 980;
	if(b == 500) b = 499;

	double y = 1;
//	double a = pow(pow(2.0 + pow(3.0, abs(b)), b), 1.0/(double)n);
	double a;

//	if(b < 500) a = 0.994957 - 0.531926 / b + 0.00610731;
//	else if(b > 500) a = 1.0 / (0.994957 - 0.531926 / (1000 - b) + 0.00610731);
	if(b < 500) a = slopefunction(b * 0.001);
	else if(b > 500) a = 1.0 / slopefunction(1 - (b * 0.001));

	a = pow(a, 1000.0 / (double)n);

	double y_n = pow(a, (double)n);
	double y_scale = (double)(y1 - y2) / (1.0 - y_n);
	double y_offset = y2 - y_scale * y_n;

	MoveToEx(hdc, x1, y1, 0);

	for(int i = 1; i < n + 1; i++) {
		y *= a;

		double y_i = y * y_scale + y_offset;

		LineTo(hdc, x1 + i, y_i);
	}
	

}

void EnvelopePaint(Envelope *e, HWND hwnd)
{
	PAINTSTRUCT ps;
	BeginPaint(hwnd, &ps);

	RECT r;
	GetClientRect(hwnd, &r);

	DeleteObject(SelectObject(ps.hdc, CreatePen(PS_SOLID, 1, RGB(0xff, 0xff, 0xff))));
	Rectangle(ps.hdc, r.left, r.top, r.right, r.bottom);
	
	DeleteObject(SelectObject(ps.hdc, CreatePen(PS_SOLID, 1, RGB(0x4f, 0x4f, 0x4f))));

	int startx = -e->viewx * (double)(r.right - r.left - 12) + 6;

	MoveToEx(ps.hdc, startx, r.top + 6, 0);
	LineTo(ps.hdc, r.right, r.top + 6);
	MoveToEx(ps.hdc, startx, r.bottom - 6, 0);
	LineTo(ps.hdc, r.right, r.bottom - 6);
	MoveToEx(ps.hdc, startx, r.top + 6, 0);
	LineTo(ps.hdc, startx, r.bottom - 6);
//	Rectangle(ps.hdc, r.left + 6, r.top + 6, r.right - 6, r.bottom - 6);
	DeleteObject(SelectObject(ps.hdc, CreatePen(PS_SOLID, 1, 0)));

	int i;

	bool prev_loopstart = false;
	double px, py;
	double px2, py2;
	int xx = 0;
	for(i = 0; i < e->data.nodes; i++) {
		xx += e->data.nodedata[i].x;
		double x = ((double)xx / e->scalex - e->viewx) * (double)(r.right - r.left - 12) + 6;
		double y = (1.f - (double)e->data.nodedata[i].y / 65536.0) * (double)(r.bottom - r.top - 12) + 6;

		double x2 = ((double)(xx + e->data.nodedata[i].x2) / e->scalex - e->viewx) * (double)(r.right - r.left - 12) + 6;
		double y2 = (1.f - (double)e->data.nodedata[i].y2 / 65536.0) * (double)(r.bottom - r.top - 12) + 6;

		if(e->data.nodedata[i].mode == 1 || e->data.nodedata[i].mode == 2) {
			MoveToEx(ps.hdc, x, 0, 0);
			LineTo(ps.hdc, x, r.bottom - r.top);
		}

		if(i > 0) {
			DeleteObject(SelectObject(ps.hdc, CreatePen(PS_DASH, 1, 0)));
			TraceEnvelopeCurve(ps.hdc, px2, py2, x2, y2, e->data.nodedata[i].slope);
			DeleteObject(SelectObject(ps.hdc, CreatePen(PS_SOLID, 1, 0)));
			TraceEnvelopeCurve(ps.hdc, px, py, x, y, e->data.nodedata[i].slope);
		}

		if(prev_loopstart == true) {
			prev_loopstart = false;

			for(int j = 0; j < e->data.nodes; j++) {
				if(e->data.nodedata[j].mode == 2) {
					py = (1.f - (double)e->data.nodedata[j].y / 65536.0) * (double)(r.bottom - r.top - 12) + 6;
					break;
				}
			}

			DeleteObject(SelectObject(ps.hdc, CreatePen(PS_DOT, 1, 0)));
			TraceEnvelopeCurve(ps.hdc, px, py, x, y, e->data.nodedata[i].slope);
			DeleteObject(SelectObject(ps.hdc, CreatePen(PS_SOLID, 1, 0)));
		}

		px = x;
		py = y;
		px2 = x2;
		py2 = y2;

		if(e->data.nodedata[i].mode == 1) prev_loopstart = true;
	}

	xx = 0;
	for(i = 0; i < e->data.nodes; i++) {
		xx += e->data.nodedata[i].x;
		double x = ((double)xx / e->scalex - e->viewx) * (double)(r.right - r.left - 12) + 6;
		double y = (1.f - (double)e->data.nodedata[i].y / 65536.0) * (double)(r.bottom - r.top - 12) + 6;

		double x2 = ((double)(xx + e->data.nodedata[i].x2) / e->scalex - e->viewx) * (double)(r.right - r.left - 12) + 6;
		double y2 = (1.f - (double)e->data.nodedata[i].y2 / 65536.0) * (double)(r.bottom - r.top - 12) + 6;

		if(e->selected.find(i) != e->selected.end()) {
			// selected
			DeleteObject(SelectObject(ps.hdc, GetSysColorBrush(COLOR_HIGHLIGHT)));
		} else {
			// not selected
			DeleteObject(SelectObject(ps.hdc, GetSysColorBrush(COLOR_MENU)));
		}

		Rectangle(ps.hdc, x - 3, y - 3, x + 4, y + 4);
		Rectangle(ps.hdc, x2 - 3, y2 - 3, x2 + 4, y2 + 4);

		DeleteObject(SelectObject(ps.hdc, CreatePen(PS_DASHDOT, 1, 0)));
		MoveToEx(ps.hdc, x, y, 0);
		LineTo(ps.hdc, x2, y2);
		DeleteObject(SelectObject(ps.hdc, CreatePen(PS_SOLID, 1, 0)));

		if(i > 0) {
			int ix = px + ((x - px) * e->data.nodedata[i].slope) / 65536;
			int iy = (py + y) * 0.5;
			RoundRect(ps.hdc, ix - 3, iy - 3, ix + 4, iy + 4, 3, 3);
		}

		px = x;
		py = y;
	}

	DeleteObject(SelectObject(ps.hdc, GetStockObject(BLACK_PEN)));
	DeleteObject(SelectObject(ps.hdc, GetStockObject(BLACK_BRUSH)));

	EndPaint(hwnd, &ps);
}

int EnvelopeNodeHit(Envelope *e, HWND hwnd, int x, int y, int &hitslope)
{
	int found = 1024;
	int foundslope = 0;
	double dist = 10000;

	hitslope = 0;

	RECT r;
	GetClientRect(hwnd, &r);

	double px, py;
	int xx = 0;
	for(int i = 0; i < e->data.nodes; i++) {
		xx += e->data.nodedata[i].x;
		double x2 = ((double)xx / e->scalex - e->viewx) * (double)(r.right - r.left - 12) + 6;
		double y2 = (1.f - (double)e->data.nodedata[i].y / 65536.0) * (double)(r.bottom - r.top - 12) + 6;

		double x3 = ((double)(xx + e->data.nodedata[i].x2) / e->scalex - e->viewx) * (double)(r.right - r.left - 12) + 6;
		double y3 = (1.f - (double)e->data.nodedata[i].y2 / 65536.0) * (double)(r.bottom - r.top - 12) + 6;

		if(i > 0) {
			int ix = px + ((x2 - px) * e->data.nodedata[i].slope) / 65536;
			int iy = (py + y2) * 0.5;

			double l = sqrt((double)((x-ix)*(x-ix)+(y-iy)*(y-iy)));
			if(l < dist) {
				found = i;
				dist = l;
				foundslope = 1;
			}
		}

		px = x2;
		py = y2;

		double l = sqrt((x-x3)*(x-x3)+(y-y3)*(y-y3));
		if(l < dist) {
			found = i;
			dist = l;
			foundslope = 2;
		}

		l = sqrt((x-x2)*(x-x2)+(y-y2)*(y-y2));
		if(l <= dist) {
			found = i;
			dist = l;
			foundslope = 0;
		}

	}

	hitslope = foundslope;
	if(dist < 8) return found;

	return -1;
}

static LRESULT CALLBACK MultiEnvelopeWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int result = 0;
	
    switch(message) {
	case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lpcs->lpCreateParams);
		}
		break;

	case WM_PAINT:
		{
			Envelope *e = (Envelope*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			EnvelopePaint(e, hwnd);
		}
		break;

	case WM_LBUTTONDOWN:
		{
			Envelope *e = (Envelope*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			SetCapture(hwnd);

			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);

			if(wParam & MK_SHIFT) {
				GetCursorPos(&e->drag_start);
				e->drag = 4;
				e->temp = e->viewx;
				e->temp2 = e->scalex;
				break;
			}

			int hitslope;
			int hitnode = EnvelopeNodeHit(e, hwnd, xPos, yPos, hitslope);

			if(hitnode != -1) {

				GetCursorPos(&e->drag_start);

				if(hitslope == 2 || ((hitslope == 0) && (wParam & MK_CONTROL))) {
					// morph
					e->drag = 3;

					e->data.nodedata[hitnode].t = e->data.nodedata[hitnode].slope;
					if((hitslope == 0) && (wParam & MK_CONTROL)) {
						e->data.nodedata[hitnode].x2 = 0;
						e->data.nodedata[hitnode].y2 = e->data.nodedata[hitnode].y;
						e->data.nodedata[hitnode].xo = 0;
						e->data.nodedata[hitnode].yo = e->data.nodedata[hitnode].y;
						InvalidateRect(hwnd, 0, FALSE);
					}

					e->data.nodedata[hitnode].xo = e->data.nodedata[hitnode].x2;
					e->data.nodedata[hitnode].yo = e->data.nodedata[hitnode].y2;

					if(e->selected.find(hitnode) == e->selected.end()) {
						e->selected.clear();
						e->selected.insert(hitnode);
						
						InvalidateRect(hwnd, 0, FALSE);
					}

					return 0;
				} if(hitslope == 1) {
					// slope
					e->drag = 2;

					e->data.nodedata[hitnode].t = e->data.nodedata[hitnode].slope;
					e->data.nodedata[hitnode].xo = e->data.nodedata[hitnode].x;
					e->data.nodedata[hitnode].yo = e->data.nodedata[hitnode].y;

					if(e->selected.find(hitnode) == e->selected.end()) {
						e->selected.clear();
						e->selected.insert(hitnode);
						
						InvalidateRect(hwnd, 0, FALSE);
					}

					return 0;
				} else if(hitnode != -1) {
					e->data.nodedata[hitnode].xo = e->data.nodedata[hitnode].x;
					e->data.nodedata[hitnode].yo = e->data.nodedata[hitnode].y;

	//				if(wParam & MK_CONTROL) {
	//					e->selected.insert(hitnode);
	//					InvalidateRect(hwnd, 0, FALSE);
	//				} else {
						if(e->selected.find(hitnode) == e->selected.end()) {
							e->selected.clear();
							e->selected.insert(hitnode);
							
							InvalidateRect(hwnd, 0, FALSE);
						}
	//				}

					e->drag = 1;

					return 0;
				}
			} else {
				e->drag = 0;
				e->selected.clear();
				InvalidateRect(hwnd, 0, FALSE);
			}
		}
		break;

	case WM_LBUTTONUP:
		{
			Envelope *e = (Envelope*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			ReleaseCapture();

			e->drag = 0;
		}
		break;

	case WM_RBUTTONDOWN:
		{
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);

			Envelope *e = (Envelope*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if(e->drag != 0) break;
			SetCapture(hwnd);

			int hitslope;
			int hitnode = EnvelopeNodeHit(e, hwnd, xPos, yPos, hitslope);

			RECT r;
			GetClientRect(hwnd, &r);


			if(hitnode > -1 && hitslope == 0) {
				if(EnvelopeCanDelete(e, hitnode)) {
					EnvelopeDeleteNode(e, hitnode);
					InvalidateRect(hwnd, 0, FALSE);
				}
			} else if(hitnode == -1) {
				double x = e->scalex * (e->viewx + (double)xPos / (double)(r.right - r.left - 12)) + 6;
				double y = 65536 * (1 - (double)yPos / (double)(r.bottom - r.top - 12)) + 6;

				EnvelopeAddNode(e, x, y);
				InvalidateRect(hwnd, 0, FALSE);
			}
		}
		break;

	case WM_RBUTTONUP:
		{
			Envelope *e = (Envelope*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			ReleaseCapture();
		}
		break;

	case WM_MOUSEMOVE:
		{
			Envelope *e = (Envelope*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);

			RECT r;
			GetClientRect(hwnd, &r);
			double dragscalex = e->scalex / (double)(r.right - r.left - 12);
			double dragscaley = 65535 / (double)(r.bottom - r.top - 12);

			if(e->drag == 4) {

				POINT p;
				GetCursorPos(&p);

				e->viewx = (e->temp - (p.x - e->drag_start.x) / (double)(r.right - r.left - 12)) / pow(2.0, (p.y - e->drag_start.y) / (double)(r.bottom - r.top - 12));
				if(e->viewx < 0) e->viewx = 0;
				
				e->scalex = e->temp2 * pow(2.0, (p.y - e->drag_start.y) / (double)(r.bottom - r.top - 12));

				InvalidateRect(hwnd, 0, FALSE);

			} else if(e->drag == 3) {

				POINT p;
				GetCursorPos(&p);

				for(int i = 0; i < e->data.nodes; i++) {
					
					if(e->selected.find(i) != e->selected.end()) {
						int new_x = e->data.nodedata[i].xo + (double)(p.x - e->drag_start.x) * dragscalex;
						if(new_x < -32767)
							new_x = -32767;
						if(new_x > 32767)
							new_x = 32767;

						if(i == 0) new_x = 0;

						e->data.nodedata[i].x2 = new_x;

						int new_y = e->data.nodedata[i].yo - (double)(p.y - e->drag_start.y) * dragscaley;
						if(new_y < 0)
							new_y = 0;
						if(new_y > 65535)
							new_y = 65535;
						e->data.nodedata[i].y2 = new_y;
					}
				}

				InvalidateRect(hwnd, 0, FALSE);

			} else if(e->drag == 2) {
				POINT p;
				GetCursorPos(&p);

				for(int i = 0; i < e->data.nodes; i++) {

					if(e->selected.find(i) != e->selected.end()) {
						int new_slope = e->data.nodedata[i].t + 65536 * (double)(p.x - e->drag_start.x) / (double)e->data.nodedata[i].x * e->scalex / (double)(r.right - r.left - 12);
						if(new_slope < 1) new_slope = 1;
						else if(new_slope > 65534) new_slope = 65534;
						e->data.nodedata[i].slope = new_slope;
					}
				}

				InvalidateRect(hwnd, 0, FALSE);

			} else if(e->drag == 1) {
				POINT p;
				GetCursorPos(&p);

				// move loop should be inverted if nodes are moved to the right.
				// otherwise they'll get stuck on each other...
				int ss = 0;
				int se = e->data.nodes;
				int sd = 1;
				if(p.x > e->drag_start.x) {
					ss = se - 1;
					se = -1;
					sd = -1;
				}

				for(int i = ss; i != se; i += sd) {
					
					if(e->selected.find(i) != e->selected.end()) {
						int new_x = e->data.nodedata[i].xo + (double)(p.x - e->drag_start.x) * dragscalex;
						if(new_x < 1)
							new_x = 1;
						if(new_x > 65535)
							new_x = 65535;

						if(i == 0) new_x = 0;

						e->data.nodedata[i].x = new_x;

						int new_y = e->data.nodedata[i].yo - (double)(p.y - e->drag_start.y) * dragscaley;
						if(new_y < 0)
							new_y = 0;
						if(new_y > 65535)
							new_y = 65535;

						if(e->data.nodedata[i].y == e->data.nodedata[i].y2) e->data.nodedata[i].y2 = new_y;
						e->data.nodedata[i].y = new_y;
					}
				}

				InvalidateRect(hwnd, 0, FALSE);
			}
		}
		break;

	case WM_CLOSE:
		{
			PostQuitMessage(0);
		}
        break;
		
	default:
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }
	
    return result;
}

static LRESULT CALLBACK FungusEnvWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    // result data
    int result = 0;

    // handle message
    switch (message)
    {
		case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)lpcs->lpCreateParams);
		}
		break;

        case WM_SIZE:
		{
			Envelope *e = (Envelope*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

			RECT r;
			GetClientRect(hWnd, &r);

			SetWindowPos(e->envwnd, 0, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER);
		}
		break;

        case WM_CLOSE:
        {
            ShowWindow(hWnd, SW_HIDE);
        }
        break;

        default:
        {
            result = DefWindowProc(hWnd,message,wParam,lParam);
        }
    }

    return result;
}

void CreateEnvelope(Envelope *e, char *name)
{
	assert(e);

    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = FungusEnvWndProc;
    wc.lpszClassName = "FUNGUSENVWINDOW";
    RegisterClass(&wc);

	RECT r;
    r.left = 0;
    r.right = 500;
    r.top = 0;
    r.bottom = 250;
    AdjustWindowRect(&r,WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,0);
	
    HWND parent = CreateWindow("FUNGUSENVWINDOW", name,
		(WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX), // & ~WS_THICKFRAME
		CW_USEDEFAULT,CW_USEDEFAULT,r.right - r.left,r.bottom - r.top,GetForegroundWindow(),0,0,(LPVOID)e);
	e->wnd = parent;
	
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = MultiEnvelopeWndProc;
    wc.lpszClassName = "MULTIENVELOPE";
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

	GetClientRect(parent, &r);

    e->envwnd = CreateWindow("MULTIENVELOPE","",WS_CHILD,r.left,r.top,r.right - r.left,r.bottom - r.top,parent,0,0,e);
	ShowWindow(e->envwnd, SW_NORMAL);
}

void ShowEnvelope(Envelope *e)
{
	ShowWindow(e->wnd, SW_SHOWNORMAL);
}

void RepaintEnvelope(Envelope *e)
{
	if(e->wnd)
		InvalidateRect(e->wnd, NULL, TRUE);
}

#else

void CreateEnvelope(Envelope *e, char *name)
{
}

Envelope *InitEnvelope(Envelope *e)
{
	e->reset();

	return e;
}

void DestroyEnvelope(Envelope *e)
{
}

void ShowEnvelope(Envelope *e)
{
}

void RepaintEnvelope(Envelope *e)
{
}

#endif
