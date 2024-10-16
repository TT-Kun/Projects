#pragma once
#include"Common.h"
#include"ObjectPool.h"
#include"PageMap.h"

class PageHeap
{
public:
	static PageHeap& GetInstance()
	{
		return _instance;
	}
	Span* NewSpan(size_t k);
	//获取一个k页的span

	Span* MapObjToSpan(void* obj);

	void ReleaseSpanToPage(Span* span);

	std::mutex _pagemtx;
private:
	SpanList _spanlist[NPAGES];
	ObjectPool<Span>_spanPool;
	TCMalloc_PageMap1<32 - PAGE_SHIFT>_spanmapID;
	PageHeap(){}
	PageHeap(const PageHeap&) = delete;
	//若没有显式禁止拷贝构造，编译器会生成默认的私有拷贝构造函数，而如果代码不严谨出现问题时，
	//私有的拷贝构造函数可能会被类中的一些函数调用，出现拷贝构造的情况，破坏了单例模式
	static PageHeap _instance;
};