#ifndef _COMMON_H_
#define _COMMON_H_

#if defined(_WIN32) && defined(VM_SHARED_LIBRARY)
	#ifdef common_EXPORTS
		#define COMMON_EXPORT_IMPORT __declspec(dllexport)
	#else
		#define COMMON_EXPORT_IMPORT __declspec(dllimport)
	#endif
#else
	#define COMMON_EXPORT_IMPORT
#endif

#include <memory>


namespace ysl
{

	class Object;

	template<typename Ty>
	Ty* Object_Dynamic_Cast(Object * obj)
	{
		if (obj == nullptr)
			return nullptr;
		return Ty::_ms_RttiType.DerivedFrom(obj->GetRtti()) || obj->GetRtti().DerivedFrom(Ty::_ms_RttiType) ? static_cast<Ty*>(obj) : nullptr;
	}

	template<typename Ty>
	const Ty* Object_Dynamic_Cast(const Object * obj)
	{
		if (obj == nullptr)
			return nullptr;
		return Ty::_ms_RttiType == (obj->GetRtti()) ? (Ty*)(obj) : nullptr;
	}

	template<typename Ty>
	Ty * Object_Static_Cast(Object * obj)
	{
		return static_cast<Ty>(obj);
	}

	template<typename Ty>
	const Ty * Object_Static_Cast(const Object * obj)
	{
		return static_cast<Ty>(obj);
	}

	//template<typename Ty>

	template<typename Ty>
	std::shared_ptr<Ty> Shared_Object_Dynamic_Cast(const std::shared_ptr<Object> &obj)
	{
		if (obj == nullptr)
			return nullptr;
		return Ty::_ms_RttiType == obj->GetRtti() ? std::static_pointer_cast<Ty>(obj) : nullptr;
	}

	template<typename Ty>
	std::shared_ptr<Ty> Shared_Object_Static_Cast(const std::shared_ptr<Object> & obj)
	{
		return std::static_pointer_cast<Ty>(obj);
	}


}
#endif