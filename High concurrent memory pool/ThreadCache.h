#pragma once
#include"Common.h"

class ThreadCache
{
public:
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);
	//申请和释放内存对象,类外声明

	void* FetchFromCentralCache(size_t index, size_t size);
	//从中心缓存获取对象，类外声明

	void ListCentralRecycle(FreeList& list, size_t size);
	//释放对象时，链表过长时，回收内存到中心缓存

private:
	FreeList _freelist[NFREE_LISTS];
};

static _declspec(thread)ThreadCache* pTLSThreadCache = nullptr;
//每个线程都有一个pTLSThreadCache指针(类型为ThreadCache*)，在所在的线程内是全局可访问的，
//但是不能够被其他线程访问到，保持了数据的线程独立性