#pragma once
//��������ռ�

#include"Common.h"
#include"ThreadCache.h"
#include"PageHeap.h"
#include"ObjectPool.h"

static void* ConcurrentAlloc(size_t size)
//����һ���߳��ڷ����ڴ棬����size��С�Ŀռ䣬����ֵΪ����Ŀռ�ĵ�ַ
{
	if (size > MAX_BYTES) {
		size_t alignSize = SizeClass::RoundUp(size);
		//����256KB���ڴ����RoundUp������ҳ����
		//����256KB��С��128*8KB����128ҳ��С����Ȼ����ʹ�ôδ��ڴ�أ��������������128/8KB������Ҫ��ϵͳ�������ڴ棬��һ����NewSpan��ʵ��
		size_t k = alignSize >> PAGE_SHIFT;

		PageHeap::GetInstance()._pagemtx.lock();
		Span* span = PageHeap::GetInstance().NewSpan(k);
		span->_objSize = size;
		PageHeap::GetInstance()._pagemtx.unlock();
		
		void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
		//��ҳ��*8k�õ���ַ
		return ptr;
	}
	else {
		if (!pTLSThreadCache) {
			//���pTLSThreadCacheΪ�գ���˵�����̻߳�û���Լ���ThreadCache���������ڴ����
			//pTLSThreadCache = new ThreadCache;
			static ObjectPool<ThreadCache>tcPool;
			tcPool._poolMtx.lock();
			pTLSThreadCache = tcPool.New();
			tcPool._poolMtx.unlock();

			//�����µ�ThreadCache����
		}
		//cout << std::this_thread::get_id() << ":" << pTLSThreadCache << endl;
		return pTLSThreadCache->Allocate(size);
	}
}

static void ConcurrentFree(void* ptr)
//����һ���߳��ڴ���ͷŻ���
{
	Span* span = PageHeap::GetInstance().MapObjToSpan(ptr);
	size_t size = span->_objSize;
	//�Ż�������ֻ�ô���ָ�벻�ô���size
	if (size > MAX_BYTES) //�������256KB�����п����Ǵ�PageCache�õģ��п�������ϵͳ�õ�
	{
		//Span* span = PageCache::GetInstance().MapObjToSpan(ptr);
		//ͨ����ַmap���ҵ�span

		PageHeap::GetInstance()._pagemtx.lock();
		PageHeap::GetInstance().ReleaseSpanToPage(span);
		PageHeap::GetInstance()._pagemtx.unlock();

	}
	else {
		assert(pTLSThreadCache);
		pTLSThreadCache->Deallocate(ptr, size);
	}
	//���ڴ���ӵ���������������
}