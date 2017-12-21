/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_STL_ALLOCATOR_HPP__
#define __INC_YXC_SYS_BASE_STL_ALLOCATOR_HPP__

#include <memory>
#include <YXC_Sys/YXC_Sys.h>

#include <YXC_Sys/YXC_LocalMM.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>

#include <vector>
#include <map>
#include <deque>

namespace YXCLib
{
	template <typename MP, typename T>
	class StlAlloc
	{
	public:
		typedef ysize_t size_type;
		typedef yintptr_t difference_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;

		template <typename U>
		class rebind
		{
		public:
			typedef StlAlloc<MP, U> other;
		};

		StlAlloc(MP* pEmp, size_type stMaxSize)
		{
			this->pPool = pEmp;
			this->stMaxSize = stMaxSize;
		}

		StlAlloc(const StlAlloc& rhs)
		{
			this->pPool = rhs.pPool;
			this->stMaxSize = rhs.stMaxSize;
		}

		StlAlloc& operator =(const StlAlloc& rhs)
		{
			this->pPool = rhs.pPool;
			this->stMaxSize = rhs.stMaxSize;
		}

		template <typename U>
		StlAlloc(const StlAlloc<MP, U>& rhs) // for stl vector / deque
		{
			this->pPool = rhs.pPool;
			this->stMaxSize = 1;
		}

		T* allocate(size_type count)
		{
			void* pMemory = pPool->Alloc(count * sizeof(T));
			if (pMemory == NULL)
			{
				throw std::bad_alloc();
				return NULL;
			}

			return (T*)pMemory;
		}

		void deallocate(void* ptr, size_type count)
		{
			pPool->Free(ptr);
		}

		size_type max_size() const
		{
			return this->stMaxSize;
		}

		void construct(pointer ptr, const_reference val)
		{
			new (ptr) T(val);
		}

		void destroy(pointer ptr)
		{
			ptr->~T();
		}

	private:
		MP* pPool;

		ysize_t stMaxSize;

		// template <typename T>
		friend class StlAlloc; // <T>;
	};
}

namespace YXC_Inner
{
	template <typename T, typename Alloc>
	class _VecWrap
	{
		typedef std::vector<T, Alloc> _Vec;
		static YXC_Status push_back(_Vec& vec, const T& data)
		{
			try
			{
				vec.push_back(data);
				return YXC_ERC_SUCCESS;
			}
			catch (std::bad_alloc& e)
			{
				return YXC_ERC_OUT_OF_MEMORY;
			}
		}
	};
}

#endif /* __INC_YXC_SYS_BASE_STL_ALLOCATOR_HPP__ */
