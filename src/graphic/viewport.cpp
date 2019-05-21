
#include "viewport.h"

#include "../opengl/openglutils.h"

namespace ysl
{
	namespace vpl
	{
		void Viewport::Activate() const
		{
			GL(glViewport(xpos, ypos, width, height));
			GL(glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w));

			if(clearWhat)
			{
				GLboolean color_write_mask[4] = { 0,0,0,0 };
				GL(glGetBooleanv(GL_COLOR_WRITEMASK, color_write_mask));

				GLboolean depth_write_mask = 0;
				GL(glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_write_mask));

				GLboolean stencil_write_mask = 0;
				GL(glGetBooleanv(GL_STENCIL_WRITEMASK, &stencil_write_mask));

				
				GL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
				GL(glDepthMask(GL_TRUE));
				GL(glStencilMask(GL_TRUE));

				GL(glClearColor(clearColor.x,clearColor.y,clearColor.z,clearColor.w));
				GL(glClearDepth(clearDepth));
				GL(glClearStencil(clearStencil));
				GL(glClear(clearWhat));

				glColorMask(color_write_mask[0], color_write_mask[1], color_write_mask[2], color_write_mask[3]);
				glDepthMask(depth_write_mask);
				glStencilMask(stencil_write_mask);
			}
			
		}
	}
}