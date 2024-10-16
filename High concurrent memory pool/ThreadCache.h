#pragma once
#include"Common.h"

class ThreadCache
{
public:
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);
	//������ͷ��ڴ����,��������

	void* FetchFromCentralCache(size_t index, size_t size);
	//�����Ļ����ȡ������������

	void ListCentralRecycle(FreeList& list, size_t size);
	//�ͷŶ���ʱ���������ʱ�������ڴ浽���Ļ���

private:
	FreeList _freelist[NFREE_LISTS];
};

static _declspec(thread)ThreadCache* pTLSThreadCache = nullptr;
//ÿ���̶߳���һ��pTLSThreadCacheָ��(����ΪThreadCache*)�������ڵ��߳�����ȫ�ֿɷ��ʵģ�
//���ǲ��ܹ��������̷߳��ʵ������������ݵ��̶߳�����