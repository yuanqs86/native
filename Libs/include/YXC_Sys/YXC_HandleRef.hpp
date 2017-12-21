/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_HANDLE_REF_HPP__
#define __INC_YXC_SYS_BASE_HANDLE_REF_HPP__

#include <YXC_Sys/YXC_Sys.h>

namespace YXCLib
{
	template <typename T, ybool_t bStdcall>
	struct DestroyFuncImpl
	{
	public:
		typedef void (*DestroyFunc)(T rawObject); /* must not be NULL! */
	};

	template <typename T>
	struct DestroyFuncImpl<T, TRUE>
	{
	public:
		typedef void (__stdcall *DestroyFunc)(T rawObject); /* must not be NULL! */
	};

	template <typename T, ybool_t bStdcall = FALSE>
	class HandleRef
	{
	public:
		typedef typename DestroyFuncImpl<T, bStdcall>::DestroyFunc DestroyFunc;
	public:
		inline HandleRef(DestroyFunc func) : _rawObject(), _destroyFunc(func), _bHasObject(FALSE)
		{

		}

		inline HandleRef(T rawObject, DestroyFunc destoryFunc)
		{
			this->Attach(rawObject, destoryFunc);
		}

		inline void Attach(T rawObject)
		{
			this->_bHasObject = true;
			this->_rawObject = rawObject;
		}

		inline operator T()
		{
			return this->_rawObject;
		}

		inline void Attach(T rawObject, DestroyFunc destoryFunc)
		{
			this->_bHasObject = TRUE;
			this->_rawObject = rawObject;
			this->_destroyFunc = destoryFunc;
		}

		inline T Detach()
		{
			this->_bHasObject = FALSE;
			return this->_rawObject;
		}

		inline ~HandleRef()
		{
			this->Destroy();
		}

		inline void Destroy()
		{
			if (this->_bHasObject)
			{
				this->_destroyFunc(this->_rawObject);
				this->_bHasObject = FALSE;
			}
		}
	private:
		ybool_t _bHasObject;
		T _rawObject;
		DestroyFunc _destroyFunc;
	};

	template <typename T, typename C>
	class HandleRefEx
	{
	public:
		typedef void (*DestroyFunc)(T rawObject, C controllor); /* must not be NULL! */

	public:
		inline HandleRefEx(DestroyFunc func) : _rawObject(), _controllor(), _destroyFunc(func), _bHasObject(FALSE)
		{

		}

		inline HandleRefEx(T rawObject, C controllor, DestroyFunc destoryFunc)
		{
			this->Attach(rawObject, controllor, destoryFunc);
		}

		inline void Attach(T rawObject, C controllor)
		{
			this->_bHasObject = TRUE;
			this->_rawObject = rawObject;
			this->_controllor = controllor;
		}

		inline operator T()
		{
			return this->_rawObject;
		}

		inline void Attach(T rawObject, C controllor, DestroyFunc destoryFunc)
		{
			this->_bHasObject = TRUE;
			this->_rawObject = rawObject;
			this->_controllor = controllor;
			this->_destroyFunc = destoryFunc;
		}

		inline void Detach()
		{
			this->_bHasObject = FALSE;
		}

		inline ~HandleRefEx()
		{
			this->Destroy();
		}

		inline void Destroy()
		{
			if (this->_bHasObject)
			{
				this->_destroyFunc(this->_rawObject, this->_controllor);
				this->_bHasObject = FALSE;
			}
		}
	private:
		ybool_t _bHasObject;
		T _rawObject;
		C _controllor;
		DestroyFunc _destroyFunc;
	};
}

#endif /* __INC_YXC_SYS_BASE_HANDLE_REF_HPP__ */
