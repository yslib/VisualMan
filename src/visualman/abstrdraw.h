
#ifndef _ABSTRADRAW_H_
#define _ABSTRADRAW_H_

#include "graphictype.h"
#include <vector>

namespace ysl
{
	namespace vm
	{
		class VISUALMAN_EXPORT_IMPORT AbstrDrawCall
		{
		public:
			AbstrDrawCall():drawType(PT_TRIANGLES){}
			virtual void Render()const = 0;
			virtual void UpdateDirtyBufferObject(BufferObjectUpdateMode mode) = 0;
			virtual void DestroyBufferObject() = 0;
			void SetPrimitiveType(PrimitiveType type) { drawType = type; }
			PrimitiveType GetPrimitiveType()const { return drawType; }
			virtual ~AbstrDrawCall() = default;
		private:
			PrimitiveType drawType;
			//std::vector<Ref<IDrawCallEvent>> events;
		};
	}
}
#endif