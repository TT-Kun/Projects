#pragma once
#include"Common.h"

//����ÿ������ֻ�ܹ���һ��Central Cache�����ʹ�õ���ģʽ�������ö���ģʽ
class CentralCache
{
public:
	static CentralCache& GetInstance() {
		return _instance;
	}
	Span* GetOneSpan(SpanList& list, size_t byte_size);
	//��ȡһ���ǿ�span
	size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);
	//��centralcache�л�ȡһ�������Ķ���ռ���� threadcache

	void ReleaseListToSpans(void* start, size_t size);
	//���г�ȥ��һ��������С���ڴ���ջ�span
private:
	CentralCache(){}
	SpanList _spanlist[NFREE_LISTS];
	static CentralCache _instance;
};
