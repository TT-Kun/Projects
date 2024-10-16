#define _CRT_SECURE_NO_WARNINGS 1
#include"Common.h"
#include"ThreadCache.h"
#include"CentralCache.h"

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size) //threadcache向centralcache申请空间
{
	//慢开始反馈调节算法
	size_t batchnum = min(_freelist[index].GetMaxSize(), SizeClass::GetMemNum(size));
	//因为GetMenNum（size）是最大能够申请到size大小的个数，不能再多，所以做一个比较，避免GetMaxSize（）无限制的增长
	if (_freelist[index].GetMaxSize() == batchnum) {
		_freelist[index].GetMaxSize() += 1;
		//当每次申请空间后，进行+1操作，下次能够一次性申请比此次多一个单位的空间，实现慢增长
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
		//将申请到的内存挂到freelist当中
		return start;
	}
}

void* ThreadCache::Allocate(size_t size) {
	assert(size <= MAX_BYTES);
	size_t align_size = SizeClass::RoundUp(size);
	//判断size大小的空间应该使用多少位字节对齐
	size_t index = SizeClass::Index(size);
	//找到size大小的空间对应的哈希桶的索引
	if (!_freelist[index].Empty()) 
		//判断对应的链表是否为空，如果非空，则弹出空间（自由链表头删）
	{
		return _freelist[index].Pop();
	}
	else //如果为空，则向下一层申请内存
	{
		return FetchFromCentralCache(index, align_size);
	}
}

void ThreadCache::Deallocate(void* ptr, size_t size)
{
	assert(ptr);
	assert(size <= MAX_BYTES);
	//要小于256kb才能走threadcache
	// 找对映射的自由链表桶，对象插入进入
	size_t index = SizeClass::Index(size);
	_freelist[index].Push(ptr);

	//当链表长度大于一次批量申请的内存时就摘掉一段的list给central cache
	if (_freelist[index].GetSize() >= _freelist[index].GetMaxSize()) {
		ListCentralRecycle(_freelist[index], size);
	}
}

void ThreadCache::ListCentralRecycle(FreeList& list, size_t size) 
//从threadcache中拿走批量的内存回收到centralcache中
{
	void* start = nullptr;
	void* end = nullptr;
	list.PopRange(start, end,list.GetMaxSize());
	CentralCache::GetInstance().ReleaseListToSpans(start, size);
}
