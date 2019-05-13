
#ifndef _ABSTRARENDERSTATE_H_
#define _ABSTRARENDERSTATE_H_
#include "graphictype.h"
#include "camera.h"
#include "program.h"

namespace ysl
{
	namespace gpu
	{
		class RenderContext;

		enum RenderStateType
		{
			RS_VertexAttrib,
			RS_VertexAttrib0 = RS_VertexAttrib,
			RS_VertexAttrib1,
			RS_VertexAttrib2,
			RS_VertexAttrib3,
			RS_VertexAttrib4,
			RS_VertexAttrib5,
			RS_VertexAttrib6,
			RS_VertexAttrib7,

			RS_AlphaFunc,
			RS_BlendEquation,
			RS_BlendFunc,
			RS_CullFace,
			RS_DepthFunc,
			RS_FrontFace,
			RS_PolygonMode,
			RS_LineWidth,
			RS_GLSLProgram,

			RS_NONE,
		};

		class RenderState
		{
		public:
			RenderState() = delete;
			RenderState(RenderStateType type) :type(type) {}
			virtual void Apply(int index, Ref<Camera> camera, RenderContext * context)const = 0;
			RenderStateType Type()const { return type; }
		private:
			RenderStateType type;
		};


		class RenderStateNonIndexed :public RenderState
		{
		public:
			RenderStateNonIndexed(RenderStateType type) :RenderState(type) {}
		};

		class RenderStateSet
		{
		public:
			RenderStateSet() = default;
			void SetRenderState(Ref<RenderState> state);
			void AddRenderState(Ref<RenderState> state);

			Ref<GLSLProgram> Program()const { return program; }
		private:
			std::vector<Ref<RenderState>> renderStates;
			Ref<GLSLProgram> program;

		};

	}
}

#endif