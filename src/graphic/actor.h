
#ifndef _ACTOR_H_
#define _ACTOR_H_
#include "graphictype.h"
#include "renderable.h"
#include "../mathematics/transformation.h"

namespace ysl
{
	namespace gpu 
	{

		class Actor 
		{
		public:
			Actor() = default;
			void SetTransform(const Transform & transform){this->transform = transform;}
			const Transform & GetTransform()const { return transform; }
		protected:
			Bound3f bound;
			Ref(Renderable) renderbles;
			Transform transform;
		};

		using ActorRefArray = std::vector<Ref(Actor)>;
	}
}

#endif