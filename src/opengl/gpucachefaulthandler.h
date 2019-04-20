
#ifndef _GPUCACHEFAULTHANDLER_H_
#define _GPUCACHEFAULTHANDLER_H_
#include "../io/largevolumecache.h"
#include "openglbuffer.h"
#include "abstrgpuobject/abstrgpucachefaulthandler.h"

namespace ysl
{
	class HashBasedGPUCacheFaultHandler :public AbstrGPUCacheFaultHandler
	{
		std::shared_ptr<OpenGLBuffer> bufMissedHash;
		std::shared_ptr<OpenGLBuffer> bufMissedTable;
		std::shared_ptr<OpenGLBuffer> atomicCounter;
		void InitHashBuffer(int blockCount);
		void InitMissTableBuffer();
		void InitGPUAtomicCounter();
		void ResetBlockExistsHash();
		void ResetMissedBlockVector();
		const int capacity;
		const Size3 dim;
	public:
		HashBasedGPUCacheFaultHandler(int capacity, const Size3 & dim);
		std::vector<VirtualMemoryBlockIndex> CaptureCacheFault() override;
		void BindHashTableTo(int index);
		void BindFaultTableTo(int index);
		void BindAtomicCounterTo(int index);
		void Reset() override;
		~HashBasedGPUCacheFaultHandler();
	};
}

#endif