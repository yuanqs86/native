/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_NULLABLE_HPP__
#define __INC_YXC_SYS_BASE_NULLABLE_HPP__

#ifdef __cplusplus
#include <new>
#include <YXC_Sys/YXC_Sys.h>

namespace YXCLib
{
	template <typename T>
	class Nullable
	{
	private:
		T* pObj;
		ybyte_t byObj[sizeof(T)];

	public:
		inline Nullable() : pObj(NULL)
		{

		}

		inline Nullable(const T& tObj) : pObj((T*)byObj)
		{
			new (this->pObj) T(tObj);
		}

		inline Nullable(const Nullable& rhs) : pObj(NULL)
		{
			if (rhs.pObj)
			{
				this->pObj = (T*)&byObj;
				new (this->pObj) T(*rhs.pObj);
			}
		}

		inline Nullable& operator =(const T& tObj)
		{
			this->Set(tObj);
			return *this;
		}

		inline Nullable& operator =(const Nullable& rhs)
		{
			if (this != &rhs)
			{
				this->SetPtr(rhs.pObj);
			}
			return *this;
		}

		inline ~Nullable()
		{
			this->SetToNull();
		}

		inline void Set(const T& tObj)
		{
			if (this->pObj)
			{
				*this->pObj = tObj;
			}
			else
			{
				this->pObj = (T*)&byObj;
				new (this->pObj) T(tObj);
			}
		}

		inline void SetPtr(const T* pObjPtr)
		{
			if (pObjPtr)
			{
				this->Set(*pObjPtr);
			}
			else
			{
				this->SetToNull();
			}
		}

		inline ybool_t IsNull() const
		{
			return pObj == NULL;
		}

		inline ybool_t IsNotNull() const
		{
			return !this->IsNull();
		}

		inline const T& Get() const
		{
			return *pObj;
		}

		inline const T* GetPtr() const
		{
			return pObj;
		}

		inline operator ybool_t() const
		{
			return this->IsNotNull();
		}

		inline void SetToNull()
		{
			if (this->pObj)
			{
				this->pObj->~T();
				this->pObj = NULL;
			}
		}
	};
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_NULLABLE_HPP__ */
