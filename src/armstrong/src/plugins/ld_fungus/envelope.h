#ifdef _WIN32
#include<windows.h>
#endif
#include<set>

typedef struct {
	unsigned short x, y;	// delta x, absolute y
	short x2; unsigned short y2;	// delta x, absolute y, for the envmorphing
	unsigned short slope;	// slope for the previous part of the envelope
	short mode;	// 0=normal, 1=loopstart, 2=loopend

	int t, xo, yo;	// temporary, delta x, absolute y
	
	// for realtime processing
	int n;
	double target;
	double mul;
} Node;

typedef struct {
	int nodes;
	Node nodedata[200];
} EnvelopeData;

class EnvelopeState {
public:
	EnvelopeData d;

	EnvelopeState() { lastvalue = 0; curnode = 0xff; isover = true;}

	int curnode;
	int curtime;
	double value;
	double mul, mul64;
	double scale, offset;

	int loopstart, loopend;
	float lastvalue;
	bool isover;

	void End(void);
	float get(int ns, int fill);
	bool over() { return isover; }
	float last() { return lastvalue; }

	float shape[64];

};

class Envelope {
public:
	std::set<int> selected;

#ifdef WIN32
	HWND wnd;
	HWND envwnd;

	// envelope edit state
	int drag;
	POINT drag_start;
#endif

	double scalex, viewx;

	double temp, temp2;

	EnvelopeData data;

	void reset(void);
};

void TriggerEnvelope(Envelope *e, EnvelopeState *state, double morph, bool reset, int samplerate);
Envelope *InitEnvelope(Envelope *e);

void CreateEnvelope(Envelope *e, char *name);
void DestroyEnvelope(Envelope *e);
void ShowEnvelope(Envelope *e);
void RepaintEnvelope(Envelope *e);


