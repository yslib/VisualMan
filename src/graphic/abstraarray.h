
#ifndef _ABSTRAARRAY_H_
#define _ABSTRAARRAY_H_

#include "graphictype.h"
#include "bufferobject.h"
#include "../mathematics/transformation.h"

namespace ysl
{
	namespace graphics
	{
		class /*GRAPHICS_EXPORT_IMPORT*/ AbstraArray
		{
		public:
			AbstraArray();

			AbstraArray(const AbstraArray& other) = default;

			AbstraArray& operator=(const AbstraArray& other) = default;

			AbstraArray(AbstraArray&& other) noexcept = default;

			AbstraArray& operator=(AbstraArray&& other) noexcept = default;

			Ref<BufferObject> GetBufferObject() { return bufferObject; }

			Ref<const BufferObject> GetBufferObject()const { return bufferObject; }

			char * RawData() { return bufferObject ? bufferObject->Data() : nullptr; }

			const char * RawData()const { return bufferObject ? bufferObject->Data() : nullptr; }

			void Clear() { if (bufferObject)bufferObject->Clear(); }

			std::size_t Bytes()const { return bufferObject ? bufferObject->Size() : 0; }

			virtual Bound3f GetBoundingBox()const = 0;

			virtual void SetTransform(const Transform & transform) = 0;

			// The below 3 function is used to given the enough information for
			// glVertexAttribPointer 

			virtual size_t ComponentNum()const = 0;		

			virtual unsigned Type()const = 0;

			virtual size_t CountOfVector()const = 0;
			

			virtual ~AbstraArray() {}
		private:
			Ref<BufferObject> bufferObject;
		};

		template<typename Vec_Ty, typename Sca_Ty, size_t CompNum, unsigned TypeFlag>
		class Array :public AbstraArray
		{
		public:
			using ScalarType = Sca_Ty;
			using VectorType = Vec_Ty;

			size_t ComponentNum()const override { return CompNum; }

			unsigned Type()const override { return TypeFlag; }

			virtual size_t SizeOfVector()const { return sizeof(VectorType); }

			size_t CountOfVector()const override{ return Bytes() / SizeOfVector(); }

			const VectorType * Data()const { return reinterpret_cast<const VectorType*>(RawData()); }

			VectorType * Data() { return reinterpret_cast<VectorType*>(RawData()); }

			VectorType & operator[](size_t i) { assert(i < CountOfVector()); return *(RawData() + i); }

			const VectorType & operator[](size_t i)const { assert(i < CountOfVector()); return *(RawData() + i); }

			//size_t Size()const { return Bytes(); }

			Bound3f GetBoundingBox() const override { assert(false); return Bound3f{}; }

			void SetTransform(const Transform & transform)override;

			virtual ~Array() = default;
		};

		template <typename Vec_Ty, typename Sca_Ty, size_t ScalarNum, unsigned TypeFlag>
		void Array<Vec_Ty, Sca_Ty, ScalarNum, TypeFlag>::SetTransform(const Transform& transform)
		{
			assert(false);
		}

		class ArrayFloat1 :public Array<float, float, 1, GL_FLOAT> {};
		class ArrayFloat2 :public Array<Vec2f, float, 2, GL_FLOAT> {};
		class ArrayFloat3 :public Array<Vec3f, float, 3, GL_FLOAT> {};
	}
}

#endif