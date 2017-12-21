#define __MODULE__ "EK.MMBase.FixedModel"

#include <YXC_Sys/MM/_YXC_MMModelFixed.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>

namespace YXC_Inner
{
	_MMModelFixed::_MMModelFixed() : m_FreeQSection(4000)
	{
		m_Num = 0;
		m_MaxNum = 0;
		m_len = 0;
		m_state = FALSE;
		m_free.clear();
		m_used.clear();
	}

	_MMModelFixed::~_MMModelFixed()
	{
		if(m_len && m_data.size())
			Destroy();
	}

	void* _MMModelFixed::Alloc(ysize_t len)
	{
		len = YXC_ALIGN_PTR_LEN(len);
		if(len > m_len) return NULL;

		YXCLib::Locker<YXX_Crit> locker(m_FreeQSection);

		_YXC_CHECK_REPORT_NEW_RET2(m_state, YXC_ERC_MMBASE_NOT_INIALIZED, NULL, YC("The memory isn't initialized yet"));

		map<ybyte_t*,ysize_t>::iterator iter =  m_free.begin();
		map<ybyte_t*,ysize_t>::iterator iter2;

		_YXC_CHECK_REPORT_NEW_RET2(iter != m_free.end() || m_Num < m_MaxNum, YXC_ERC_MMBASE_NO_FIT_BLOCK, NULL, YC("Free list is empty"));

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
		else if(m_Num < m_MaxNum)
		{
			ybyte_t* data = (ybyte_t*)malloc(m_len);
			if (data == NULL)
			{
				_YXC_REPORT_ERR(YXC_ERC_C_RUNTIME, YC("Alloc new fixed block failed"));
				return NULL;
			}

			m_data.push_back(data);
			m_Num++;
			m_free.insert(make_pair(data,m_len));
			m_FreeLen += m_len;
			return Alloc(len);
		}

		_YXC_REPORT_ERR(YXC_ERC_MMBASE_NO_FIT_BLOCK, YC("No fit block can be allocated"));
		return NULL;
	}

	void _MMModelFixed::Free(void* data)
	{
		YXCLib::Locker<YXX_Crit> locker(m_FreeQSection);

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

	YXC_Status _MMModelFixed::Init(ysize_t size, ysize_t MaxNum)
	{
		size = YXC_ALIGN_PTR_LEN(size);
		_YXC_CHECK_REPORT_NEW_RET(MaxNum >= 1, YXC_ERC_INVALID_PARAMETER, YC("Parameter MaxNum must be bigger than 1"));

		YXCLib::Locker<YXX_Crit> locker(m_FreeQSection);

		ybyte_t* data = (ybyte_t*)malloc(size);
		_YXC_CHECK_REPORT_NEW_RET(data != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc new fixed block failed"));

		m_data.push_back(data);
		m_len = size;
		m_state = TRUE;
		m_FreeLen = size;
		m_free.insert(make_pair(data,size));
		m_Num = 1;
		m_MaxNum = MaxNum;
		return YXC_ERC_SUCCESS;
	}

	void _MMModelFixed::Destroy()
	{
		YXCLib::Locker<YXX_Crit> locker(m_FreeQSection);
		for (size_t i = 0; i < m_data.size();i++)
		{
			ybyte_t* data = m_data[i];
			free(data);
		}
		m_data.clear();
		m_FreeLen = 0;
		m_len = 0;
		m_state = FALSE;

		m_free.clear();
		m_used.clear();
	}
}
