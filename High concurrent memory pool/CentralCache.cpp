#include"Common.h"
#include"CentralCache.h"
#include"PageHeap.h"

CentralCache CentralCache::_instance;
//����ģʽʵ��������������

Span* CentralCache::GetOneSpan(SpanList& list, size_t size) {
	//�鿴��ǰspanlist����û��δ��������span
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
	//�Ȱ�central cache��Ͱ���⿪����������߳��ͷ��ڴ��centralcache��ʱ�򲻻����������������ڴ���ܻᱻ������pagecache�赲��
	//��������ߵ����˵��û�п���span��Ҫ��pagecacheҪ�ռ�
	PageHeap::GetInstance()._pagemtx.lock();
	Span* span = PageHeap::GetInstance().NewSpan(SizeClass::GetPageNum(size));
	span->_isused = true;
	span->_objSize = size;
	PageHeap::GetInstance()._pagemtx.unlock();
	//����Ի�ȡ����span�����з֣�����Ҫ��������Ϊ�������߳̾����������̷߳��ʲ������span

	char* startadd = (char*)(span->_pageId << PAGE_SHIFT);
	//���span����ʼ��ַ
	size_t bytes = span->_n << PAGE_SHIFT;
	//����span���ֽ�����С

	char* end = startadd + bytes;

	span->_freelist = startadd;
	startadd += size;
	void* tail = span->_freelist;
	//1������һ��������ͷ�ڵ㣬����β��
	while (startadd < end) {
		NextObj(tail) = startadd;
		tail = NextObj(tail);
		startadd += size;
	}
	NextObj(tail) = nullptr;
	//�Ѵ���ڴ��г�����������������

	list._mtx.lock();
	//���к���span����Ҫ���кõ�span�ҵ�Ͱ���棬�ٰ����ӻ�ȥ

	list.PushFront(span);//ͷ�嵽������
	return span;
}
//��ȡһ���ǿյ�span

size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchnum, size_t size) 
//��centralcache�л�ȡbatchnum�����Ķ���ռ���� threadcache
{
	size_t index = SizeClass::Index(size);
	_spanlist[index]._mtx.lock();

	Span* span = GetOneSpan(_spanlist[index], size);
	assert(span);
	assert(span->_freelist);
	//��span�л�ȡbatchnum������
	//�������batchnum�����ж��ٸ��ö��ٸ�
	start = span->_freelist;
	end = start;
	size_t i = 0;			//����ѭ��
	size_t actualnum = 1;	//ʵ������Ķ���ռ�����
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
	_spanlist[index]._mtx.lock();			//����
	
	while (start) {
		void* next = NextObj(start);//��¼��һ������

		Span* span = PageHeap::GetInstance().MapObjToSpan(start);
		//�Ѷ���ͷ�嵽span��
		NextObj(start) = span->_freelist;
		span->_freelist = start;
		span->_use_count--;

		if (span->_use_count == 0)
			//��Ϊ0�������span�����зֳ�ȥ��С���ڴ涼�����ˡ�
			//���span���ܻ��ո�pagecache��pagecache�ܹ����Խ���ǰ��ҳ�ĺϲ�
		{
			_spanlist[index].Erase(span);
			span->_freelist = nullptr;
			span->_next = nullptr;
			span->_prev = nullptr;

			_spanlist[index]._mtx.unlock();			
			//��Ͱ������ʱ���Ѿ�����Ҫ���յĲ��ִ�����ˣ�����Ҫ�ٱ䶯Ͱ�ˣ�
			//���Ϊ�����threadcache��span��ȡ���߹黹�ռ��Ч�ʣ����԰�Ͱ���⿪
			PageHeap::GetInstance()._pagemtx.lock();			//���ռ��pagecache���Ӵ���
			PageHeap::GetInstance().ReleaseSpanToPage(span);
			PageHeap::GetInstance()._pagemtx.unlock();			//�����

			_spanlist[index]._mtx.lock();	//��Ͱ��

		}
		start = next;
	}
	_spanlist[index]._mtx.unlock();			//����
	
}