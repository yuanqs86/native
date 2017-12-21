/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_REF_OBJECT_HPP__
#define __INC_YXC_SYS_BASE_REF_OBJECT_HPP__

#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_HandleRef.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <new>

namespace YXCLib
{
	template <typename T>
	class RefObject;

	template <typename T>
	static inline void _CloseRefObject(RefObject<T>* obj);

	template <typename T>
	class RefObject
	{
	protected:
		inline RefObject() : _refCount(1) {}

		virtual ~RefObject() {}

	public:
		static inline T* NewObject()
		{
			return new T();
		}

		template <typename C1>
		static inline T* NewObject(C1 c1)
		{
			return new T(c1);
		}

		template <typename C1, typename C2>
		static inline T* NewObject(C1 c1, C2 c2)
		{
			return new T(c1, c2);
		}

		template <typename C1, typename C2, typename C3>
		static inline T* NewObject(C1 c1, C2 c2, C3 c3)
		{
			return new T(c1, c2, c3);
		}

	public:
		static inline YXC_Status RNew(T** ppObj)
		{
			_YCHK_MAL_R1(pObj, T);
			YXCLib::HandleRef<void*> pObj_res(pObj, free);

			_YCHK_REPLACEMENT_NEW_RET(pObj, T);
			*ppObj = (T*)pObj_res.Detach();
			return YXC_ERC_SUCCESS;
		}

		template <typename C1>
		static inline YXC_Status RNew(T** ppObj, C1 c1)
		{
			_YCHK_MAL_R1(pObj, T);
			YXCLib::HandleRef<void*> pObj_res(pObj, free);

			_YCHK_REPLACEMENT_NEW_RET1(pObj, T, c1);
			*ppObj = (T*)pObj_res.Detach();
			return YXC_ERC_SUCCESS;
		}

		template <typename C1, typename C2>
		static inline YXC_Status RNew(T** ppObj, C1 c1, C2 c2)
		{
			_YCHK_MAL_R1(pObj, T);
			YXCLib::HandleRef<void*> pObj_res(pObj, free);

			_YCHK_REPLACEMENT_NEW_RET1(pObj, T, c1, c2);
			*ppObj = (T*)pObj_res.Detach();
			return YXC_ERC_SUCCESS;
		}

		template <typename C1, typename C2, typename C3>
		static inline YXC_Status RNew(T** ppObj, C1 c1, C2 c2, C3 c3)
		{
			_YCHK_MAL_R1(pObj, T);
			YXCLib::HandleRef<void*> pObj_res(pObj, free);

			_YCHK_REPLACEMENT_NEW_RET1(pObj, T, c1, c2, c3);
			*ppObj = (T*)pObj_res.Detach();
			return YXC_ERC_SUCCESS;
		}

	protected:
		RefObject(const RefObject& rhs) : _refCount(1) {}

		RefObject& operator =(const RefObject& rhs)
		{
			this->Unreference();

			this->_refCount = 1;
		}

	public:
		inline yuint32_t Reference()
		{
			yuint32_t refCount = YXCLib::Interlocked::Increment(&this->_refCount);
			return refCount;
		}

		inline yuint32_t Unreference()
		{
			yuint32_t uDecVal = YXCLib::Interlocked::Decrement(&this->_refCount);
			if (uDecVal == 0)
			{
				this->_OnNoReference();
			}

			return uDecVal;
		}

	protected:
		virtual void _OnNoReference() = 0;

	private:
		volatile yuint32_t _refCount;

	public:
		struct HRef : public YXCLib::HandleRef< RefObject<T>* >
		{
		public:
			inline HRef() : YXCLib::HandleRef< RefObject<T>* >(_CloseRefObject<T>) {}

			inline HRef(RefObject<T>* obj) : YXCLib::HandleRef< RefObject<T>* >(obj, _CloseRefObject<T>) {}
		};
	};

	template <typename T>
	static inline void _CloseRefObject(RefObject<T>* obj)
	{
		obj->Unreference();
	}
}

#endif /* __INC_YXC_SYS_BASE_REF_OBJECT_HPP__ */
