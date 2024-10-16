#define _CRT_SECURE_NO_WARNINGS 1
#include"Common.h"
#include"PageHeap.h"

PageHeap PageHeap::_instance;

//��ȡһ��kҳ��span
Span* PageHeap::NewSpan(size_t k)
{
	if (k > NPAGES - 1) //������ҳ������128��ʱ������pagecache�й��������ڴ���128ҳ����ʱ�޷�ʹ���ڴ�ؽ����ڴ������Ҫ��ϵͳ����ռ�
	{
		void* ptr = SystemAlloc(k);
		// Span* span = new Span;
		Span* span = _spanPool.New();
		span->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
		span->_n = k;

		//_spanmapID[span->_pageId] = span;
		_spanmapID.set(span->_pageId, span);
		//���ｫ��ϵͳ�����Span������pageid�浽map���У������ͷź����ж���Ӧ���ͷŸ�ϵͳ�����ǹ黹��pagecache

		return span;
	}
	assert(k > 0 && k < NPAGES);

	if (!_spanlist[k].Empty()) //����k��Ͱ��û��span
	{
		Span* kSpan =  _spanlist[k].PopFront();
		
		//����id��span��ӳ�䣬����central cache����С���ڴ棬���Ҷ�Ӧ��span
		for (PAGE_ID i = 0; i < kSpan->_n; i++) {
			//_spanmapID[kSpan->_pageId + i] = kSpan;
			_spanmapID.set(kSpan->_pageId + i, kSpan);
		}
		return kSpan;
	} 
	//���һ�º����Ͱ������û��span������У����԰��������з�
	for (size_t i = k + 1; i < NPAGES; i++) {
		if (!_spanlist[i].Empty()) //ֻҪ�и�Ͱ�ǿգ�����
		{
			Span* nSpan = _spanlist[i].PopFront();
			//Span* kSpan = new Span;
			Span* kSpan = _spanPool.New();
			
			//��span��ͷ����һ��kҳ����
			//kҳspan����
			//nspan�ٹҵ���Ӧӳ���λ��
			kSpan->_pageId = nSpan->_pageId;
			kSpan->_n = k;

			nSpan->_pageId += k;
			nSpan->_n -= k;

			_spanlist[nSpan->_n].PushFront(nSpan);
			//�洢nSpan����βҳ�Ÿ�nSpan��ӳ�䣬����page cache�����ڴ�ʱ���еĺϲ�����
			//_spanmapID[nSpan->_pageId] = nSpan;
			_spanmapID.set(nSpan->_pageId, nSpan);
			
			//_spanmapID[nSpan->_pageId + nSpan->_n - 1] = nSpan;
			_spanmapID.set(nSpan->_pageId + nSpan->_n - 1, nSpan);

			for (PAGE_ID i = 0; i < kSpan->_n; i++) {
				//_spanmapID[kSpan->_pageId + i] = kSpan;
				_spanmapID.set(kSpan->_pageId + i, kSpan);
			}
			//unordermapӳ�䣬����centralcache����С���ڴ�ʱ���Ҷ�Ӧ��span
			return kSpan;
		}
	}
	//�ߵ����λ��û�з��ؾ�˵������û�д�ҳ��Span����Ҫ��ϵͳ�����ռ�
	//Span* bigspan = new Span;
	Span* bigspan = _spanPool.New();
	void* ptr = SystemAlloc(NPAGES-1);//���ù��Ƿ�����ɹ���������ɹ��������ڲ������쳣
	bigspan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigspan->_n = NPAGES - 1;

	_spanlist[bigspan->_n].PushFront(bigspan);

	return NewSpan(k);//�ݹ�����Լ�

}

Span* PageHeap::MapObjToSpan(void* obj)
//����ҳ�ĵ�ַ���ҵ���Ӧ��ӳ��
{
	PAGE_ID id = (PAGE_ID)obj >> PAGE_SHIFT;

	//std::unique_lock<std::mutex>lock(_pagemtx);
	//�������������Զ�����
	//����ҳ�ĵ�ַ�ҵ�ҳ��
	//auto ret = _spanmapID.find(id);
	//if (ret != _spanmapID.end()) {
	//	return ret->second;
	//}
	//else {
	//	assert(false);
	//	return nullptr;
	//}
	//��������װ����PageMap�е�get����
	auto ret = (Span*)_spanmapID.get(id);
	assert(ret != nullptr);
	return ret;
}

void PageHeap::ReleaseSpanToPage(Span* span)
//��span����page,���Ҷ�spanǰ���ҳ���Խ��кϲ��������ڴ���Ƭ�����⡣
{
	if (span->_n > NPAGES - 1) //���Span�µ�_n��Ա����128��˵���Ǵ�ϵͳֱ������Ŀռ䣬ֱ���ͷŵ��ͺ���
	{
		void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
		SystemFree(ptr);
		//delete span;
		_spanPool.Delete(span);
		return;
	}

	while (1) //��ǰ�ϲ�
	{
		PAGE_ID prevID = span->_pageId - 1;

		//auto ret = _spanmapID.find(prevID);
		//if (ret == _spanmapID.end()) //ǰ��ҳ��û���ˣ����ϲ���
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
		if (prevspan->_isused) //ǰ������ҳ��span��ʹ���У����ϲ�
		{
			break;
		}
		if (prevspan->_n + span->_n > NPAGES - 1)//�ϲ�������128ҳ���Ͳ��ϲ���
		{
			break;
		}
		span->_pageId = prevspan->_pageId;
		span->_n += prevspan->_n;

		_spanlist[prevspan->_n].Erase(prevspan);//����Ϊ�ϲ��ɸ�����ڴ�飬Ҫ�����ӵ�ӳ���ϵ���µ�λ�ã�����ںϲ�ǰ��Ӧ�Ĺ�ϣͰ��ɾ��ԭ���Ľڵ�
		//delete prevspan;
		_spanPool.Delete(prevspan);
	}

	while (1)//���ϲ�
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

	//���ϲ���Ĵ��ڴ�ҵ���Ӧλ��
	_spanlist[span->_n].PushFront(span);
	span->_isused = false;	//�ϲ�������������״̬�ı��false�����page���Ժϲ����page
	//_spanmapID[span->_pageId] = span;
	_spanmapID.set(span->_pageId, span);
	//_spanmapID[span->_pageId + span->_n - 1] = span;
	_spanmapID.set(span->_pageId + span->_n - 1, span);

}
