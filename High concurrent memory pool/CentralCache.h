#pragma once
#include"Common.h"

//由于每个进程只能够有一个Central Cache，因此使用单例模式，这里用饿汉模式
class CentralCache
{
public:
	static CentralCache& GetInstance() {
		return _instance;
	}
	Span* GetOneSpan(SpanList& list, size_t byte_size);
	//获取一个非空span
	size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);
	//从centralcache中获取一定数量的对象空间给到 threadcache

	void ReleaseListToSpans(void* start, size_t size);
	//将切出去的一定数量的小块内存回收回span
private:
	CentralCache(){}
	SpanList _spanlist[NFREE_LISTS];
	static CentralCache _instance;
};
