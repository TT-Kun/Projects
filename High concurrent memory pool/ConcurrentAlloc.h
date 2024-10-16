#pragma once
//并发申请空间

#include"Common.h"
#include"ThreadCache.h"
#include"PageHeap.h"
#include"ObjectPool.h"

static void* ConcurrentAlloc(size_t size)
//用于一个线程内分配内存，分配size大小的空间，返回值为申请的空间的地址
{
	if (size > MAX_BYTES) {
		size_t alignSize = SizeClass::RoundUp(size);
		//大于256KB的内存调用RoundUp函数按页对齐
		//大于256KB但小于128*8KB（即128页大小）仍然可以使用次此内存池，但是如果超过了128/8KB，就需要向系统中申请内存，这一点再NewSpan中实现
		size_t k = alignSize >> PAGE_SHIFT;

		PageHeap::GetInstance()._pagemtx.lock();
		Span* span = PageHeap::GetInstance().NewSpan(k);
		span->_objSize = size;
		PageHeap::GetInstance()._pagemtx.unlock();
		
		void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
		//将页号*8k得到地址
		return ptr;
	}
	else {
		if (!pTLSThreadCache) {
			//如果pTLSThreadCache为空，则说明此线程还没有自己的ThreadCache对象用于内存管理
			//pTLSThreadCache = new ThreadCache;
			static ObjectPool<ThreadCache>tcPool;
			tcPool._poolMtx.lock();
			pTLSThreadCache = tcPool.New();
			tcPool._poolMtx.unlock();

			//创建新的ThreadCache对象
		}
		//cout << std::this_thread::get_id() << ":" << pTLSThreadCache << endl;
		return pTLSThreadCache->Allocate(size);
	}
}

static void ConcurrentFree(void* ptr)
//用于一个线程内存的释放回收
{
	Span* span = PageHeap::GetInstance().MapObjToSpan(ptr);
	size_t size = span->_objSize;
	//优化函数，只用传入指针不用传入size
	if (size > MAX_BYTES) //如果大于256KB，则有可能是从PageCache拿的，有可能是向系统拿的
	{
		//Span* span = PageCache::GetInstance().MapObjToSpan(ptr);
		//通过地址map，找到span

		PageHeap::GetInstance()._pagemtx.lock();
		PageHeap::GetInstance().ReleaseSpanToPage(span);
		PageHeap::GetInstance()._pagemtx.unlock();

	}
	else {
		assert(pTLSThreadCache);
		pTLSThreadCache->Deallocate(ptr, size);
	}
	//将内存添加到回收自由链表中
}