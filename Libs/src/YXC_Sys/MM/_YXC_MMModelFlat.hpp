#ifndef __INNER_INC_YXC_SYS_BASE_MM_MODEL_FLAT_HPP__
#define __INNER_INC_YXC_SYS_BASE_MM_MODEL_FLAT_HPP__

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

namespace YXC_Inner
{
	class _MMModelFlat : public _MMModelBase
	{
	public:
		_MMModelFlat();

		~_MMModelFlat();

	public:
		virtual void* Alloc(ysize_t stNumBytes); //����һ����С���ڴ�

		virtual void Free(void* pData);//�ͷ��ڴ�

		YXC_Status Init(ysize_t stPoolSize); //��ʼ��

		void Destroy();//�ͷ�

	private:
		_MMModelFlat(_MMModelFlat& rhs);

		_MMModelFlat& operator =(_MMModelFlat& rhs);

	private:
		map<ybyte_t*, ysize_t> m_free; //δ��������ڴ� ��ʼλ�úͳ���

		map<ybyte_t*, ysize_t> m_used;//�Ѿ������ȥ���ڴ� void* Ϊ��ʼλ�ã�esize_t Ϊռ�ó���

		ybyte_t* m_data;//�ڴ�������ڴ�Ŀ�ʼλ��

		ysize_t m_len;//�ڴ���е��ڴ��С

		ybool_t m_state;//�Ƿ��ʼ��

		ysize_t m_FreeLen;//δ���䳤��

		YXX_Crit m_FreeQSection;
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_MM_MODEL_FLAT_HPP__ */
