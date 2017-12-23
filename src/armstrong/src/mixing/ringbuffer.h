#pragma once
#pragma warning(push)
#pragma warning(disable:4311) // warning C4312: 'type cast' : conversion from 'long' to 'void *' of greater size
#pragma warning(disable:4312) // warning C4311: 'type cast' : pointer truncation from 'void *' to 'long'
#pragma warning(disable:4244) // warning C4244: 'argument' : conversion from 'intptr_t' to 'long', possible loss of data
#pragma warning(disable:4267) // warning C4267: 'argument' : conversion from 'size_t' to 'unsigned int', possible loss of data
#include <boost/lockfree/spsc_queue.hpp>
#pragma warning(pop)
