
#ifndef _RENDERSTATESET_H_
#define _RENDERSTATESET_H_
#include "graphictype.h"
#include "abstrarenderstate.h"

namespace ysl
{
	namespace graphics
	{
		class GLSLProgram;

		class RenderStateSet
		{
		public:
			RenderStateSet() = default;
			void SetRenderState(Ref<RenderState> state, int index);
			Ref<RenderState> GetRenderState(RenderStateType type, int index);
			void RemoveRenderState(RenderStateType type, int index);
			void SetProgram(Ref<GLSLProgram> program);
			Ref<GLSLProgram> CreateGetProgram();
		private:
			std::vector<RenderStateBox> renderStates;
			Ref<GLSLProgram> program;
		};

	}
}
#endif