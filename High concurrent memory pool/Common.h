#pragma once
//��Ź��е�ͷ�ļ�

#include<iostream>
#include<algorithm>
#include<vector>
#include<time.h>
#include<assert.h>
#include<unordered_map>
#include<thread>
#include<mutex>
//#include<Windows.h>
using std::cout;
using std::endl;

static const size_t MAX_BYTES = 256 * 1024;		//256kb	
static const size_t NFREE_LISTS = 208;	//freelist��ϣͰ�е�Ͱ������
static const size_t NPAGES = 129;		//PageCache��Ͱ������
static const size_t PAGE_SHIFT = 13;	//����8k�൱������13λ
typedef size_t PAGE_ID;

//#ifdef _WIN64
//typedef unsigned long long PAGE_ID;
//#elif _WIN32
//typedef size_t PAGE_ID;
//#else	//linux
//#endif
//�������룬ʹ��PAEG_ID�����ڲ�ͬλ���±�Ϊ��ͬ������

#ifdef _WIN32
	#include<windows.h>
#else
#endif

//ֱ�Ӵ�ϵͳ�ϰ�ҳ����ռ�
inline static void* SystemAlloc(size_t kpage)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
	// linux��brk mmap��
#endif
	if (ptr == nullptr)
		throw std::bad_alloc();
	return ptr;
}

//�ͷŵ���ϵͳ������Ŀռ�
inline static void SystemFree(void* ptr) 
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	// sbrk unmmap��
#endif
}

static void*& NextObj(void* obj) //��װһ��ȡͷ��4/8���ֽڵ�ֵ���������������һ���ڵ�ĵ�ַ
{
	return *(void**)obj;
}

//�����зֺõ�С������������
class FreeList
{
public:
	void Push(void* obj) {
		//ͷ��
		//*(void**)obj = _freelist;
		NextObj(obj) = _freelist;
		_freelist = obj;
		_size++;
	}

	void* Pop(void) {
		//ͷɾ
		assert(_freelist);
		void* obj = _freelist;
		_freelist = NextObj(obj);
		_size--;
		return obj;
	}

	void PushRange(void* start,void* end, size_t n)
		//pushһ�η�Χ�ڵ�����ڵ�,start��endΪ��ʼ����ֹλ�ã�nΪ����
	{
		NextObj(end) = _freelist;
		_freelist = start;

		/*-----������֤���������ϵ�����ǲ��������n��-----*/
		//void* cur = start;
		//int i = 0;
		//while (cur) {
		//	cur = NextObj(cur);
		//	i++;
		//}
		//if (n != i) {
		//	int x = 0;
		//}
		/*-------------------------------------------*/

		_size += n;
	}

	void PopRange(void*& start, void*& end, size_t n) //ժ��һ�η�Χ�ڵ�����ڵ�
	{
		assert(n <= _size);//�����øú���ǰ�Ƿ����
		start = _freelist;
		end = start;

		for (size_t i = 0; i < n - 1; ++i) {
			end = NextObj(end);
		}
		_freelist = NextObj(end);

		NextObj(end) = nullptr;
		_size -= n;
	}

	bool Empty()
	{
		return _freelist == nullptr;
	}

	size_t& GetMaxSize(){
		return _maxsize;
	}

	size_t& GetSize() //��ȡ����ڵ����
	{
		return _size;
	}

private:
	void* _freelist = nullptr;
	size_t _maxsize = 1;
	size_t _size = 0;			//��¼����ڵ����
};

class SizeClass
{
public:

	static inline size_t _RoundUp(size_t size, size_t align_num)
// ������������10%���ҵ�����Ƭ�˷�
// [1,128]					8byte����	    freelist[0,16)		16��Ͱ
// [128+1,1024]				16byte����	    freelist[16,72)		56��Ͱ
// [1024+1,8*1024]			128byte����	    freelist[72,128)	56��Ͱ
// [8*1024+1,64*1024]		1024byte����    freelist[128,184)	56��Ͱ
// [64*1024+1,256*1024]		8*1024byte����  freelist[184,208)	56��Ͱ
// С�ڵ���256KB���ڴ����ThreadCacheҪ������256KB���ڴ�͵���BigAlloc����
	//�Ӻ���������һ����С���ж������С���ڴ�ʹ��AlignNum��λ������
	{
		return ((size + align_num - 1) & ~(align_num - 1));
	}
	static inline size_t RoundUp(size_t size) 
	//����һ����С���ж������С���ڴ�Ӧ��ʹ�ö����ֽڶ���
	{
		if (size <= 128)
		{
			return _RoundUp(size, 8);
		}
		else if (size <= 1024)
		{
			return _RoundUp(size, 16);
		}
		else if (size <= 8 * 1024)
		{
			return _RoundUp(size, 128);
		}
		else if (size <= 64 * 1024)
		{
			return _RoundUp(size, 1024);
		}
		else if (size <= 256 * 1024)
		{
			return _RoundUp(size, 8 * 1024);
		}
		else
		{
			return _RoundUp(size, 1 << PAGE_SHIFT);
			//����256KB���ڴ�Ͱ�ҳ������
		}
	}

	static inline size_t _Index(size_t bytes, size_t align_shift) {
		return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
	}
	static inline size_t Index(size_t bytes)
		//�ҵ���Ӧ�ڴ��С��Ͱ������
	{
		assert(bytes <= MAX_BYTES);
		// ÿ�������ж��ٸ���
		static int group_array[4] = { 16, 56, 56, 56 };//ǰ�ĸ������Ӧ��Ͱ�ĸ���
		if (bytes <= 128) {
			return _Index(bytes, 3);
		}//8
		else if (bytes <= 1024) {
			return _Index(bytes - 128, 4) + group_array[0];
		}//16
		else if (bytes <= 8 * 1024) {
			return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
		}//128
		else if (bytes <= 64 * 1024) {
			return _Index(bytes - 8 * 1024, 10) + group_array[2] + group_array[1] + group_array[0];
		}//1024
		else if (bytes <= 256 * 1024) {
			return _Index(bytes - 64 * 1024, 13) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
		}//8*1024
		else {
			assert(false);
		}
		return -1;
	}
	//static inline size_t _Index(size_t bytes, size_t align_num) {
	//	if (!bytes % align_num) {
	//		return bytes / align_num - 1;
	//	}
	//	else {
	//		return bytes / align_num;
	//	}
	//}
	//static inline size_t Index(size_t bytes)
	//	�ҵ���Ӧ�����Ͱ������
	//{
	//	assert(bytes <= MAX_BYTES);
	//	 ÿ�������ж��ٸ���
	//	static int group_array[4] = { 16, 56, 56, 56 };//ǰ�ĸ������Ӧ��Ͱ�ĸ���
	//	if (bytes <= 128) {
	//		return _Index(bytes, 8);
	//	}//8
	//	else if (bytes <= 1024) {
	//		return _Index(bytes - 128, 16) + group_array[0];
	//	}//16
	//	else if (bytes <= 8 * 1024) {
	//		return _Index(bytes - 1024, 128) + group_array[1] + group_array[0];
	//	}//128
	//	else if (bytes <= 64 * 1024) {
	//		return _Index(bytes - 8 * 1024, 1024) + group_array[2] + group_array[1] + group_array[0];
	//	}//1024
	//	else if (bytes <= 256 * 1024) {
	//		return _Index(bytes - 64 * 1024, 8*1024) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
	//	}//8*1024
	//	else {
	//		assert(false);
	//	}
	//	return -1;
	//}

	static size_t GetMemNum(size_t size) //һ��thread cache��central cache��ȡ���ٸ�С�ڴ��
	{
		assert(size > 0);
		int num = MAX_BYTES / size;		//256kb/���������С
		if (num < 2)	num = 2;
		if (num > 512)		num = 512;
		return num;
	}

	static size_t GetPageNum(size_t size) {
		size_t num = GetMemNum(size);
		size_t npage = num * size;
		//����npage�����ֽ���
		npage >>= PAGE_SHIFT;
		//����8k���һ������ҳ
		if (npage == 0) {
			npage = 1;
		}
		//�������һҳ���͸�һҳ
		return npage;
	}
	
private:

};

//����������ҳ�Ĵ���ڴ��ȽṹSpan
struct Span
{
	PAGE_ID _pageId = 0;//ҳ��
	size_t _n = 0;		//�����ҳ��

	Span* _next = nullptr;	//ָ��˫��������һ���ڵ�
	Span* _prev = nullptr;	//ָ��˫��������һ���ڵ�

	size_t _objSize = 0;  // �кõ�С����Ĵ�С
	size_t _use_count = 0;	//�к�С���ڴ��ұ������thread cache�ļ���
	void* _freelist = nullptr;	//�кõ�С���ڴ����������

	bool _isused = false;
	//����ҳ����32λ�£����̵�ַ�ռ�һ��Ϊ4G��һҳ�Ĵ�Сһ��Ϊ4k����8k
	//�����һҳ��СΪ8k����2^13����ôһ����2^32 / 2^13 = 2^19��ҳ
	//��64λ�£�һ����2^64 / 2^13 = 2^51��ҳ
};

class SpanList
{
public:
	SpanList()
	{
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	void PushFront(Span* span) //ͷ��
	{
		Insert(Begin(), span);
	}
	Span* PopFront()    //ͷɾ
	{
		Span* front = _head->_next;
		Erase(front);
		return front;
	}
	void Insert(Span* pos, Span* newSpan) //��posλ�õ�Spanǰ�������
	{
		assert(pos);
		assert(newSpan);
		Span* prev = pos->_prev;
		prev->_next = newSpan;
		newSpan->_next = pos;
		newSpan->_prev = prev;
		pos->_prev = newSpan;
	}
	void Erase(Span* pos)	//ɾ��posλ�õĽ��
	{
		assert(pos);

		// 1.�����ϵ�
		// 2.�鿴����ջ֡���鿴���������Ⱥ�
		//if (pos == _head);
		//{
		//	int x = 0;
		//}

		assert(pos != _head);
		Span* prev = pos->_prev;
		Span* next = pos->_next;
		prev->_next = next;
		next->_prev = prev;
	}
	Span* Begin()
	{
		return _head->_next;
	}
	Span* End()
	{
		return _head;
	}
	bool Empty()
	{
		return _head->_next == _head;
	}

	std::mutex _mtx;//��Ͱ��

private:
	Span* _head;	//ͷ���
};

