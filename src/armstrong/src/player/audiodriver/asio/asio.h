/*
Hello Steinberg. Your ASIO SDK is horrible, the Windows parts are filled
with broken use of the registry and COM.
Here's what's really needed to use ASIO on Windows, ruthlessly copied from
your headers. All the rest seems quite useless.

Well okay, I stripped out the DSD stuff.
 */


/* from asio.h */

typedef double ASIOSampleRate;
typedef long ASIOBool;
typedef long ASIOError;
typedef __int64 ASIOSamples;
typedef __int64 ASIOTimeStamp;
typedef long ASIOSampleType;

enum {
	ASE_OK = 0,             // This value will be returned whenever the call succeeded
	ASE_SUCCESS = 0x3f4847a0,	// unique success return value for ASIOFuture calls
	ASE_NotPresent = -1000, // hardware input or output is not present or available
	ASE_HWMalfunction,      // hardware is malfunctioning (can be returned by any ASIO function)
	ASE_InvalidParameter,   // input parameter invalid
	ASE_InvalidMode,        // hardware is in a bad mode or used in a bad mode
	ASE_SPNotAdvancing,     // hardware is not running when sample position is inquired
	ASE_NoClock,            // sample clock or rate cannot be determined or is not present
	ASE_NoMemory            // not enough memory for completing the request
};

enum {
	ASIOFalse = 0,
	ASIOTrue = 1
};

enum {
	ASIOSTInt16MSB   = 0,
	ASIOSTInt24MSB   = 1,		// used for 20 bits as well
	ASIOSTInt32MSB   = 2,
	ASIOSTFloat32MSB = 3,		// IEEE 754 32 bit float
	ASIOSTFloat64MSB = 4,		// IEEE 754 64 bit double float

	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can be more easily used with these
	ASIOSTInt32MSB16 = 8,		// 32 bit data with 16 bit alignment
	ASIOSTInt32MSB18 = 9,		// 32 bit data with 18 bit alignment
	ASIOSTInt32MSB20 = 10,		// 32 bit data with 20 bit alignment
	ASIOSTInt32MSB24 = 11,		// 32 bit data with 24 bit alignment
	
	ASIOSTInt16LSB   = 16,
	ASIOSTInt24LSB   = 17,		// used for 20 bits as well
	ASIOSTInt32LSB   = 18,
	ASIOSTFloat32LSB = 19,		// IEEE 754 32 bit float, as found on Intel x86 architecture
	ASIOSTFloat64LSB = 20, 		// IEEE 754 64 bit double float, as found on Intel x86 architecture

	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can more easily used with these
	ASIOSTInt32LSB16 = 24,		// 32 bit data with 18 bit alignment
	ASIOSTInt32LSB18 = 25,		// 32 bit data with 18 bit alignment
	ASIOSTInt32LSB20 = 26,		// 32 bit data with 20 bit alignment
	ASIOSTInt32LSB24 = 27,		// 32 bit data with 24 bit alignment

	ASIOSTLastEntry
};

typedef struct ASIOClockSource
{
	long index;					// as used for ASIOSetClockSource()
	long associatedChannel;		// for instance, S/PDIF or AES/EBU
	long associatedGroup;		// see channel groups (ASIOGetChannelInfo())
	ASIOBool isCurrentSource;	// ASIOTrue if this is the current clock source
	char name[32];				// for user selection
} ASIOClockSource;

typedef struct ASIOChannelInfo
{
	long channel;			// on input, channel index
	ASIOBool isInput;		// on input
	ASIOBool isActive;		// on exit
	long channelGroup;		// dto
	ASIOSampleType type;	// dto
	char name[32];			// dto
} ASIOChannelInfo;

typedef struct ASIOBufferInfo
{
	ASIOBool isInput;			// on input:  ASIOTrue: input, else output
	long channelNum;			// on input:  channel index
	void *buffers[2];			// on output: double buffer addresses
} ASIOBufferInfo;

typedef struct ASIOTimeCode
{       
	double          speed;                  // speed relation (fraction of nominal speed)
	                                        // optional; set to 0. or 1. if not supported
	ASIOSamples     timeCodeSamples;        // time in samples
	unsigned long   flags;                  // some information flags (see below)
	char future[64];
} ASIOTimeCode;

typedef enum ASIOTimeCodeFlags
{
	kTcValid                = 1,
	kTcRunning              = 1 << 1,
	kTcReverse              = 1 << 2,
	kTcOnspeed              = 1 << 3,
	kTcStill                = 1 << 4,
	
	kTcSpeedValid           = 1 << 8
}  ASIOTimeCodeFlags;

typedef struct AsioTimeInfo
{
	double          speed;                  // absolute speed (1. = nominal)
	ASIOTimeStamp   systemTime;             // system time related to samplePosition, in nanoseconds
	                                        // on mac, must be derived from Microseconds() (not UpTime()!)
	                                        // on windows, must be derived from timeGetTime()
	ASIOSamples     samplePosition;
	ASIOSampleRate  sampleRate;             // current rate
	unsigned long flags;                    // (see below)
	char reserved[12];
} AsioTimeInfo;

typedef enum AsioTimeInfoFlags
{
	kSystemTimeValid        = 1,            // must always be valid
	kSamplePositionValid    = 1 << 1,       // must always be valid
	kSampleRateValid        = 1 << 2,
	kSpeedValid             = 1 << 3,
	
	kSampleRateChanged      = 1 << 4,
	kClockSourceChanged     = 1 << 5
} AsioTimeInfoFlags;

typedef struct ASIOTime                          // both input/output
{
	long reserved[4];                       // must be 0
	struct AsioTimeInfo     timeInfo;       // required
	struct ASIOTimeCode     timeCode;       // optional, evaluated if (timeCode.flags & kTcValid)
} ASIOTime;

enum
{
	kAsioSelectorSupported = 1,	// selector in <value>, returns 1L if supported,
								// 0 otherwise
    kAsioEngineVersion,			// returns engine (host) asio implementation version,
								// 2 or higher
	kAsioResetRequest,			// request driver reset. if accepted, this
								// will close the driver (ASIO_Exit() ) and
								// re-open it again (ASIO_Init() etc). some
								// drivers need to reconfigure for instance
								// when the sample rate changes, or some basic
								// changes have been made in ASIO_ControlPanel().
								// returns 1L; note the request is merely passed
								// to the application, there is no way to determine
								// if it gets accepted at this time (but it usually
								// will be).
	kAsioBufferSizeChange,		// not yet supported, will currently always return 0L.
								// for now, use kAsioResetRequest instead.
								// once implemented, the new buffer size is expected
								// in <value>, and on success returns 1L
	kAsioResyncRequest,			// the driver went out of sync, such that
								// the timestamp is no longer valid. this
								// is a request to re-start the engine and
								// slave devices (sequencer). returns 1 for ok,
								// 0 if not supported.
	kAsioLatenciesChanged, 		// the drivers latencies have changed. The engine
								// will refetch the latencies.
	kAsioSupportsTimeInfo,		// if host returns true here, it will expect the
								// callback bufferSwitchTimeInfo to be called instead
								// of bufferSwitch
	kAsioSupportsTimeCode,		// 
	kAsioMMCCommand,			// unused - value: number of commands, message points to mmc commands
	kAsioSupportsInputMonitor,	// kAsioSupportsXXX return 1 if host supports this
	kAsioSupportsInputGain,     // unused and undefined
	kAsioSupportsInputMeter,    // unused and undefined
	kAsioSupportsOutputGain,    // unused and undefined
	kAsioSupportsOutputMeter,   // unused and undefined
	kAsioOverload,              // driver detected an overload

	kAsioNumMessageSelectors
} /*AsioMessageSelector*/;

typedef struct ASIOCallbacks
{
	void (*bufferSwitch) (long doubleBufferIndex, ASIOBool directProcess);
		// bufferSwitch indicates that both input and output are to be processed.
		// the current buffer half index (0 for A, 1 for B) determines
		// - the output buffer that the host should start to fill. the other buffer
		//   will be passed to output hardware regardless of whether it got filled
		//   in time or not.
		// - the input buffer that is now filled with incoming data. Note that
		//   because of the synchronicity of i/o, the input always has at
		//   least one buffer latency in relation to the output.
		// directProcess suggests to the host whether it should immedeately
		// start processing (directProcess == ASIOTrue), or whether its process
		// should be deferred because the call comes from a very low level
		// (for instance, a high level priority interrupt), and direct processing
		// would cause timing instabilities for the rest of the system. If in doubt,
		// directProcess should be set to ASIOFalse.
		// Note: bufferSwitch may be called at interrupt time for highest efficiency.

	void (*sampleRateDidChange) (ASIOSampleRate sRate);
		// gets called when the AudioStreamIO detects a sample rate change
		// If sample rate is unknown, 0 is passed (for instance, clock loss
		// when externally synchronized).

	long (*asioMessage) (long selector, long value, void* message, double* opt);
		// generic callback for various purposes, see selectors below.
		// note this is only present if the asio version is 2 or higher

	ASIOTime* (*bufferSwitchTimeInfo) (ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
		// new callback with time info. makes ASIOGetSamplePosition() and various
		// calls to ASIOGetSampleRate obsolete,
		// and allows for timecode sync etc. to be preferred; will be used if
		// the driver calls asioMessage with selector kAsioSupportsTimeInfo.
} ASIOCallbacks;

enum
{
	kAsioEnableTimeCodeRead = 1,	// no arguments
	kAsioDisableTimeCodeRead,		// no arguments
	kAsioSetInputMonitor,			// ASIOInputMonitor* in params
	kAsioTransport,					// ASIOTransportParameters* in params
	kAsioSetInputGain,				// ASIOChannelControls* in params, apply gain
	kAsioGetInputMeter,				// ASIOChannelControls* in params, fill meter
	kAsioSetOutputGain,				// ASIOChannelControls* in params, apply gain
	kAsioGetOutputMeter,			// ASIOChannelControls* in params, fill meter
	kAsioCanInputMonitor,			// no arguments for kAsioCanXXX selectors
	kAsioCanTimeInfo,
	kAsioCanTimeCode,
	kAsioCanTransport,
	kAsioCanInputGain,
	kAsioCanInputMeter,
	kAsioCanOutputGain,
	kAsioCanOutputMeter,
};

typedef struct ASIOInputMonitor
{
	long input;		// this input was set to monitor (or off), -1: all
	long output;	// suggested output for monitoring the input (if so)
	long gain;		// suggested gain, ranging 0 - 0x7fffffffL (-inf to +12 dB)
	ASIOBool state;	// ASIOTrue => on, ASIOFalse => off
	long pan;		// suggested pan, 0 => all left, 0x7fffffff => right
} ASIOInputMonitor;

typedef struct ASIOChannelControls
{
	long channel;			// on input, channel index
	ASIOBool isInput;		// on input
	long gain;				// on input,  ranges 0 thru 0x7fffffff
	long meter;				// on return, ranges 0 thru 0x7fffffff
	char future[32];
} ASIOChannelControls;

typedef struct ASIOTransportParameters
{
	long command;		// see enum below
	ASIOSamples samplePosition;
	long track;
	long trackSwitches[16];		// 512 tracks on/off
	char future[64];
} ASIOTransportParameters;

enum
{
	kTransStart = 1,
	kTransStop,
	kTransLocate,		// to samplePosition
	kTransPunchIn,
	kTransPunchOut,
	kTransArmOn,		// track
	kTransArmOff,		// track
	kTransMonitorOn,	// track
	kTransMonitorOff,	// track
	kTransArm,			// trackSwitches
	kTransMonitor		// trackSwitches
};



/* from iasiodrv.h */

interface IASIO : public IUnknown
{
	virtual ASIOBool init(HWND sysHandle) = 0;
	virtual void getDriverName(char *name) = 0;	
	virtual long getDriverVersion() = 0;
	virtual void getErrorMessage(char *string) = 0;	
	virtual ASIOError start() = 0;
	virtual ASIOError stop() = 0;
	virtual ASIOError getChannels(long *numInputChannels, long *numOutputChannels) = 0;
	virtual ASIOError getLatencies(long *inputLatency, long *outputLatency) = 0;
	virtual ASIOError getBufferSize(long *minSize, long *maxSize,
		long *preferredSize, long *granularity) = 0;
	virtual ASIOError canSampleRate(ASIOSampleRate sampleRate) = 0;
	virtual ASIOError getSampleRate(ASIOSampleRate *sampleRate) = 0;
	virtual ASIOError setSampleRate(ASIOSampleRate sampleRate) = 0;
	virtual ASIOError getClockSources(ASIOClockSource *clocks, long *numSources) = 0;
	virtual ASIOError setClockSource(long reference) = 0;
	virtual ASIOError getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp) = 0;
	virtual ASIOError getChannelInfo(ASIOChannelInfo *info) = 0;
	virtual ASIOError createBuffers(ASIOBufferInfo *bufferInfos, long numChannels,
		long bufferSize, ASIOCallbacks *callbacks) = 0;
	virtual ASIOError disposeBuffers() = 0;
	virtual ASIOError controlPanel() = 0;
	virtual ASIOError future(long selector,void *opt) = 0;
	virtual ASIOError outputReady() = 0;
};

/*
ASIO's use of COM is broken.
Look up installed drivers in registry, HKLM\SOFTWARE\ASIO.
The CLSID given for each driver is both the class-ID you will be passing to
CoCreateInstance() as well as the interface-ID you will QueryInterface() the
returned IUnknown-object for to get its IASIO interface.
For proper COM use, IASIO should have a fixed IID, but what's done is done.

Lastly, the ASIO SDK code pretends that you can only use one ASIO driver at a
time in a process. That's obviously an implementation limitation of Steinberg's
code and not a limitation of ASIO itself, the drivers are independent objects
sharing no data whatsoever, if you just use the COM interface directly instead
of relying on Steinberg's broken code, you should be able to use as many ASIO
drivers in one process as you wish.
How lame is that?
 */
