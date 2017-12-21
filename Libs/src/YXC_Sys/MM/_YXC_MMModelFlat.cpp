#define __MODULE__ "EK.MMBase.FlatModel"

#include <YXC_Sys/MM/_YXC_MMModelFlat.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>

namespace YXC_Inner
{
	_MMModelFlat::_MMModelFlat() : m_FreeQSection(4000)
	{
		m_data = NULL;
		m_len = 0;
		m_FreeLen = 0;
		m_state = FALSE;
		m_free.clear();
		m_used.clear();
	}

	_MMModelFlat::~_MMModelFlat()
	{
		if(m_len && m_data)
			Destroy();
	}

	void* _MMModelFlat::Alloc(ysize_t len)
	{
		len = YXC_ALIGN_PTR_LEN(len);
		YXCLib::Locker<YXX_Crit> L(m_FreeQSection);

		_YXC_CHECK_REPORT_NEW_RET2(m_state, YXC_ERC_MMBASE_NOT_INIALIZED, NULL, YC("The memory isn't initialized yet"));

		map<ybyte_t*,ysize_t>::iterator iter =  m_free.begin();
		map<ybyte_t*,ysize_t>::iterator iter2;
		if(iter == m_free.end()) return NULL;

		ysize_t MinNum = 0;
		for(;iter != m_free.end();iter++)//查找最合适的块
		{
			if(iter->second == len)
			{
				MinNum = iter->second;
				iter2 = iter;
				break;
			}
			if(iter->second >= len)
			{
				if(MinNum == 0)
				{
					MinNum = iter->second;
					iter2 = iter;
					continue;
				}
				if(MinNum > iter->second)
				{
					MinNum = iter->second;
					iter2 = iter;
				}
			}
		}

		if(MinNum)//存在合适的块
		{
			m_FreeLen -= len;
			if(iter2->second == len)//大小正好
			{
				m_used.insert(make_pair(iter2->first,iter2->second));
				ybyte_t* data = iter2->first;
				m_free.erase(iter2);
				return (void*)(data);
			}
			else //切掉多余部分
			{
				ysize_t len2 = iter2->second - len;
				iter2->second = len;
				m_used.insert(make_pair(iter2->first,len));
				m_free.insert(make_pair(iter2->first+len,len2));
				ybyte_t*  data = iter2->first;
				m_free.erase(iter2);
				return (void*)(data);
			}
		}
		else if(m_FreeLen >= len)//没有合适的块 但剩余空间足够
		{
		}

		_YXC_REPORT_ERR(YXC_ERC_MMBASE_NO_FIT_BLOCK, YC("No fit block can be allocated"));
		return NULL;
	}

	void _MMModelFlat::Free(void* data)
	{
		YXCLib::Locker<YXX_Crit> L(m_FreeQSection);
		ybyte_t* POS = (ybyte_t*)data;
		map<ybyte_t*,ysize_t>::iterator iter = m_used.find(POS);
		if(iter == m_used.end())
			return;

		ysize_t lenth = iter->second;
		m_free.insert(make_pair(POS,lenth));
		m_FreeLen += lenth;
		m_used.erase(iter);

		iter = m_free.find(POS);
		if(iter == m_free.end()) return;

		if(iter == m_free.begin()) {
			map<ybyte_t*,ysize_t>::iterator iter2 =  iter;
			iter2++;
			if(iter2 != m_free.end()){
				if(iter->first + iter->second == iter2->first){
					iter->second += iter2->second;
					m_free.erase(iter2);
				}
			}
		}
		else {
			map<ybyte_t*,ysize_t>::iterator iter2 = iter;
			iter2--;
			if(iter2->first+iter2->second == iter->first){
				iter2->second += iter->second;
				m_free.erase(iter);
                iter = iter2;
			}
			map<ybyte_t*,ysize_t>::iterator iter3 = iter;
			iter3++;
			if(iter3 != m_free.end()){
				if(iter->first + iter->second == iter3->first){
					iter->second += iter3->second;
					m_free.erase(iter3);
				}
			}
		}
	}

	YXC_Status _MMModelFlat::Init(ysize_t size)
	{
		size = YXC_ALIGN_PTR_LEN(size);

		m_data = (ybyte_t*)malloc(size);
		_YXC_CHECK_REPORT_NEW_RET(m_data != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc new flat block failed"));

		m_len = size;
		m_state = TRUE;
		m_FreeLen = size;
		m_free.insert(make_pair(m_data,size));
		return YXC_ERC_SUCCESS;
	}

	void  _MMModelFlat::Destroy()
	{
		if(m_data)
			free(m_data);
		m_data = NULL;
		m_len = 0;
		m_state = FALSE;

		m_free.clear();
		m_used.clear();
	}
}
