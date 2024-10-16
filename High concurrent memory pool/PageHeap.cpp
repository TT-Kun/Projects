#define _CRT_SECURE_NO_WARNINGS 1
#include"Common.h"
#include"PageHeap.h"

PageHeap PageHeap::_instance;

//获取一个k页的span
Span* PageHeap::NewSpan(size_t k)
{
	if (k > NPAGES - 1) //当申请页数大于128的时候，由于pagecache中管理的最大内存是128页，此时无法使用内存池进行内存管理，需要向系统申请空间
	{
		void* ptr = SystemAlloc(k);
		// Span* span = new Span;
		Span* span = _spanPool.New();
		span->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
		span->_n = k;

		//_spanmapID[span->_pageId] = span;
		_spanmapID.set(span->_pageId, span);
		//这里将向系统申请的Span和他的pageid存到map当中，方便释放函数判断是应该释放给系统，还是归还给pagecache

		return span;
	}
	assert(k > 0 && k < NPAGES);

	if (!_spanlist[k].Empty()) //检查第k个桶有没有span
	{
		Span* kSpan =  _spanlist[k].PopFront();
		
		//建立id和span的映射，方便central cache回收小块内存，查找对应的span
		for (PAGE_ID i = 0; i < kSpan->_n; i++) {
			//_spanmapID[kSpan->_pageId + i] = kSpan;
			_spanmapID.set(kSpan->_pageId + i, kSpan);
		}
		return kSpan;
	} 
	//检查一下后面的桶里面有没有span，如果有，可以把它进行切分
	for (size_t i = k + 1; i < NPAGES; i++) {
		if (!_spanlist[i].Empty()) //只要有个桶非空，就切
		{
			Span* nSpan = _spanlist[i].PopFront();
			//Span* kSpan = new Span;
			Span* kSpan = _spanPool.New();
			
			//在span的头部切一个k页下来
			//k页span返回
			//nspan再挂到对应映射的位置
			kSpan->_pageId = nSpan->_pageId;
			kSpan->_n = k;

			nSpan->_pageId += k;
			nSpan->_n -= k;

			_spanlist[nSpan->_n].PushFront(nSpan);
			//存储nSpan的首尾页号跟nSpan的映射，方便page cache回收内存时进行的合并查找
			//_spanmapID[nSpan->_pageId] = nSpan;
			_spanmapID.set(nSpan->_pageId, nSpan);
			
			//_spanmapID[nSpan->_pageId + nSpan->_n - 1] = nSpan;
			_spanmapID.set(nSpan->_pageId + nSpan->_n - 1, nSpan);

			for (PAGE_ID i = 0; i < kSpan->_n; i++) {
				//_spanmapID[kSpan->_pageId + i] = kSpan;
				_spanmapID.set(kSpan->_pageId + i, kSpan);
			}
			//unordermap映射，方便centralcache回收小块内存时查找对应的span
			return kSpan;
		}
	}
	//走到这个位置没有返回就说明后面没有大页的Span，需要向系统申请大空间
	//Span* bigspan = new Span;
	Span* bigspan = _spanPool.New();
	void* ptr = SystemAlloc(NPAGES-1);//不用管是否申请成功，如果不成功，函数内部会抛异常
	bigspan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigspan->_n = NPAGES - 1;

	_spanlist[bigspan->_n].PushFront(bigspan);

	return NewSpan(k);//递归调用自己

}

Span* PageHeap::MapObjToSpan(void* obj)
//根据页的地址，找到对应的映射
{
	PAGE_ID id = (PAGE_ID)obj >> PAGE_SHIFT;

	//std::unique_lock<std::mutex>lock(_pagemtx);
	//除函数作用域自动解锁
	//根据页的地址找到页号
	//auto ret = _spanmapID.find(id);
	//if (ret != _spanmapID.end()) {
	//	return ret->second;
	//}
	//else {
	//	assert(false);
	//	return nullptr;
	//}
	//上面代码封装到技PageMap中的get函数
	auto ret = (Span*)_spanmapID.get(id);
	assert(ret != nullptr);
	return ret;
}

void PageHeap::ReleaseSpanToPage(Span* span)
//将span还给page,并且对span前后的页尝试进行合并，缓解内存碎片的问题。
{
	if (span->_n > NPAGES - 1) //如果Span下的_n成员大于128，说明是从系统直接申请的空间，直接释放掉就好了
	{
		void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
		SystemFree(ptr);
		//delete span;
		_spanPool.Delete(span);
		return;
	}

	while (1) //向前合并
	{
		PAGE_ID prevID = span->_pageId - 1;

		//auto ret = _spanmapID.find(prevID);
		//if (ret == _spanmapID.end()) //前面页号没有了，不合并了
		//{
		//	break;
		//}
		
		auto ret = (Span*)_spanmapID.get(prevID);
		if (ret == nullptr)
		{
			break;
		}
		
		//Span* prevspan = ret->second;
		Span* prevspan = ret;
		if (prevspan->_isused) //前面相邻页的span在使用中，不合并
		{
			break;
		}
		if (prevspan->_n + span->_n > NPAGES - 1)//合并完会大于128页，就不合并了
		{
			break;
		}
		span->_pageId = prevspan->_pageId;
		span->_n += prevspan->_n;

		_spanlist[prevspan->_n].Erase(prevspan);//，因为合并成更大的内存块，要把它接到映射关系下新的位置，因此在合并前对应的哈希桶中删除原来的节点
		//delete prevspan;
		_spanPool.Delete(prevspan);
	}

	while (1)//向后合并
	{
		PAGE_ID nextID = span->_pageId + span->_n;
		//auto ret = _spanmapID.find(prevID);
		//if (ret == _spanmapID.end()) {
		//	break;
		//}
		auto ret = (Span*)_spanmapID.get(nextID);
		if (ret == nullptr)
		{
			break;
		}
		//Span* nextspan = ret->second;
		Span* nextspan = ret;

		if (nextspan->_isused) {
			break;
		}
		if (nextspan->_n + span->_n > NPAGES - 1) {
			break;
		}

		span->_n += nextspan->_n;

		_spanlist[nextspan->_n].Erase(nextspan);
		//delete nextspan;
		_spanPool.Delete(nextspan);
	}

	//将合并后的大内存挂到相应位置
	_spanlist[span->_n].PushFront(span);
	span->_isused = false;	//合并操作结束，将状态改编成false，别的page可以合并这个page
	//_spanmapID[span->_pageId] = span;
	_spanmapID.set(span->_pageId, span);
	//_spanmapID[span->_pageId + span->_n - 1] = span;
	_spanmapID.set(span->_pageId + span->_n - 1, span);

}
