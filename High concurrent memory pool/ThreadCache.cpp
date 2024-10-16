#define _CRT_SECURE_NO_WARNINGS 1
#include"Common.h"
#include"ThreadCache.h"
#include"CentralCache.h"

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size) //threadcache��centralcache����ռ�
{
	//����ʼ���������㷨
	size_t batchnum = min(_freelist[index].GetMaxSize(), SizeClass::GetMemNum(size));
	//��ΪGetMenNum��size��������ܹ����뵽size��С�ĸ����������ٶ࣬������һ���Ƚϣ�����GetMaxSize���������Ƶ�����
	if (_freelist[index].GetMaxSize() == batchnum) {
		_freelist[index].GetMaxSize() += 1;
		//��ÿ������ռ�󣬽���+1�������´��ܹ�һ��������ȴ˴ζ�һ����λ�Ŀռ䣬ʵ��������
	}
	
	void* start = nullptr;
	void* end = nullptr;
	size_t actualnum = CentralCache::GetInstance().FetchRangeObj(start, end, batchnum, size);
	assert(actualnum > 0);

	if (actualnum == 1) {
		assert(start == end);
		return start;
	}
	else {
		_freelist[index].PushRange(NextObj(start), end, actualnum-1);
		//�����뵽���ڴ�ҵ�freelist����
		return start;
	}
}

void* ThreadCache::Allocate(size_t size) {
	assert(size <= MAX_BYTES);
	size_t align_size = SizeClass::RoundUp(size);
	//�ж�size��С�Ŀռ�Ӧ��ʹ�ö���λ�ֽڶ���
	size_t index = SizeClass::Index(size);
	//�ҵ�size��С�Ŀռ��Ӧ�Ĺ�ϣͰ������
	if (!_freelist[index].Empty()) 
		//�ж϶�Ӧ�������Ƿ�Ϊ�գ�����ǿգ��򵯳��ռ䣨��������ͷɾ��
	{
		return _freelist[index].Pop();
	}
	else //���Ϊ�գ�������һ�������ڴ�
	{
		return FetchFromCentralCache(index, align_size);
	}
}

void ThreadCache::Deallocate(void* ptr, size_t size)
{
	assert(ptr);
	assert(size <= MAX_BYTES);
	//ҪС��256kb������threadcache
	// �Ҷ�ӳ�����������Ͱ������������
	size_t index = SizeClass::Index(size);
	_freelist[index].Push(ptr);

	//�������ȴ���һ������������ڴ�ʱ��ժ��һ�ε�list��central cache
	if (_freelist[index].GetSize() >= _freelist[index].GetMaxSize()) {
		ListCentralRecycle(_freelist[index], size);
	}
}

void ThreadCache::ListCentralRecycle(FreeList& list, size_t size) 
//��threadcache�������������ڴ���յ�centralcache��
{
	void* start = nullptr;
	void* end = nullptr;
	list.PopRange(start, end,list.GetMaxSize());
	CentralCache::GetInstance().ReleaseListToSpans(start, size);
}
