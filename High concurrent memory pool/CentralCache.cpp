#include"Common.h"
#include"CentralCache.h"
#include"PageHeap.h"

CentralCache CentralCache::_instance;
//饿汉模式实例化，类外声明

Span* CentralCache::GetOneSpan(SpanList& list, size_t size) {
	//查看当前spanlist中有没有未分配对象的span
	Span* it = list.Begin();
	while (it != list.End()) {
		if (it->_freelist != nullptr) {
			return it;
		}
		else {
			it = it->_next;
		}
	}

	list._mtx.unlock();
	//先把central cache的桶锁解开，如果其他线程释放内存给centralcache的时候不会阻塞，不过申请内存可能会被加锁的pagecache阻挡。
	//如果程序走到这里，说明没有空闲span，要找pagecache要空间
	PageHeap::GetInstance()._pagemtx.lock();
	Span* span = PageHeap::GetInstance().NewSpan(SizeClass::GetPageNum(size));
	span->_isused = true;
	span->_objSize = size;
	PageHeap::GetInstance()._pagemtx.unlock();
	//下面对获取到的span进行切分，不需要加锁，因为不存在线程竞争，其他线程访问不到这个span

	char* startadd = (char*)(span->_pageId << PAGE_SHIFT);
	//算出span的起始地址
	size_t bytes = span->_n << PAGE_SHIFT;
	//计算span的字节数大小

	char* end = startadd + bytes;

	span->_freelist = startadd;
	startadd += size;
	void* tail = span->_freelist;
	//1、先切一块下来做头节点，方便尾插
	while (startadd < end) {
		NextObj(tail) = startadd;
		tail = NextObj(tail);
		startadd += size;
	}
	NextObj(tail) = nullptr;
	//把大块内存切成自由链表链接起来

	list._mtx.lock();
	//当切好了span后，需要把切好的span挂到桶里面，再把锁加回去

	list.PushFront(span);//头插到链表里
	return span;
}
//获取一个非空的span

size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchnum, size_t size) 
//从centralcache中获取batchnum数量的对象空间给到 threadcache
{
	size_t index = SizeClass::Index(size);
	_spanlist[index]._mtx.lock();

	Span* span = GetOneSpan(_spanlist[index], size);
	assert(span);
	assert(span->_freelist);
	//从span中获取batchnum个对象
	//如果不够batchnum个，有多少个拿多少个
	start = span->_freelist;
	end = start;
	size_t i = 0;			//用作循环
	size_t actualnum = 1;	//实际申请的对象空间数量
	while (i < batchnum - 1 && NextObj(end) != nullptr) {
		end = NextObj(end);
		i++;
		actualnum++;
	}
	span->_freelist = NextObj(end);
	NextObj(end) = nullptr;
	span->_use_count += actualnum;

	_spanlist[index]._mtx.unlock();

	return actualnum;
}


void CentralCache::ReleaseListToSpans(void* start, size_t size)
{
	size_t index = SizeClass::Index(size);
	_spanlist[index]._mtx.lock();			//加锁
	
	while (start) {
		void* next = NextObj(start);//记录下一个对象

		Span* span = PageHeap::GetInstance().MapObjToSpan(start);
		//把对象头插到span中
		NextObj(start) = span->_freelist;
		span->_freelist = start;
		span->_use_count--;

		if (span->_use_count == 0)
			//若为0，则这个span所有切分出去的小块内存都回来了。
			//这个span就能回收给pagecache，pagecache能够尝试进行前后页的合并
		{
			_spanlist[index].Erase(span);
			span->_freelist = nullptr;
			span->_next = nullptr;
			span->_prev = nullptr;

			_spanlist[index]._mtx.unlock();			
			//解桶锁，这时候已经将需要回收的部分处理好了，不需要再变动桶了，
			//因此为了提高threadcache向span获取或者归还空间的效率，可以把桶锁解开
			PageHeap::GetInstance()._pagemtx.lock();			//还空间给pagecache，加大锁
			PageHeap::GetInstance().ReleaseSpanToPage(span);
			PageHeap::GetInstance()._pagemtx.unlock();			//解大锁

			_spanlist[index]._mtx.lock();	//加桶锁

		}
		start = next;
	}
	_spanlist[index]._mtx.unlock();			//解锁
	
}