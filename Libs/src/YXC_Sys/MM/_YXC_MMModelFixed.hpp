#ifndef __INNER_INC_YXC_SYS_BASE_MM_MODEL_FIXED_HPP__
#define __INNER_INC_YXC_SYS_BASE_MM_MODEL_FIXED_HPP__

#include <vector>
#include <list>
#include <deque>
#include <map>

#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/MM/_YXC_MMModelBase.hpp>

using std::deque;
using std::list;
using std::map;
using std::make_pair;
using std::vector;

namespace YXC_Inner
{
	class _MMModelFixed : public _MMModelBase
	{
	public:
		_MMModelFixed();

		~_MMModelFixed();

	public:
		virtual void* Alloc(ysize_t stNumBytes); //申请一个大小的内存

		virtual void Free(void* pData);//释放内存

		YXC_Status Init(ysize_t stNumBlockSize, ysize_t stMaxBlockNum); //初始化

		void Destroy();//释放

	private:
		_MMModelFixed(_MMModelFixed& rhs);

		_MMModelFixed& operator =(_MMModelFixed& rhs);

	private:
		map<ybyte_t*, ysize_t> m_free; //未被分配的内存 起始位置和长度

		map<ybyte_t*, ysize_t> m_used; //已经分配出去的内存 void* 为起始位置，esize_t 为占用长度

		vector<ybyte_t*> m_data; //内存池申请内存的开始位置

		ysize_t m_len; //内存池中的内存大小

		ysize_t m_MaxNum;

		ysize_t m_FreeLen;//未分配长度

		ysize_t m_Num;

		ybool_t m_state; //是否初始化

		YXX_Crit m_FreeQSection;
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_MM_MODEL_FIXED_HPP__ */
