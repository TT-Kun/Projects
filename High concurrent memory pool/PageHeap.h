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
	//��ȡһ��kҳ��span

	Span* MapObjToSpan(void* obj);

	void ReleaseSpanToPage(Span* span);

	std::mutex _pagemtx;
private:
	SpanList _spanlist[NPAGES];
	ObjectPool<Span>_spanPool;
	TCMalloc_PageMap1<32 - PAGE_SHIFT>_spanmapID;
	PageHeap(){}
	PageHeap(const PageHeap&) = delete;
	//��û����ʽ��ֹ�������죬������������Ĭ�ϵ�˽�п������캯������������벻�Ͻ���������ʱ��
	//˽�еĿ������캯�����ܻᱻ���е�һЩ�������ã����ֿ��������������ƻ��˵���ģʽ
	static PageHeap _instance;
};