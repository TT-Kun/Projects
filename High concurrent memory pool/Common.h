#pragma once
//存放共有的头文件

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
static const size_t NFREE_LISTS = 208;	//freelist哈希桶中的桶的数量
static const size_t NPAGES = 129;		//PageCache中桶的数量
static const size_t PAGE_SHIFT = 13;	//除以8k相当于右移13位
typedef size_t PAGE_ID;

//#ifdef _WIN64
//typedef unsigned long long PAGE_ID;
//#elif _WIN32
//typedef size_t PAGE_ID;
//#else	//linux
//#endif
//条件编译，使得PAEG_ID可以在不同位数下变为不同的类型

#ifdef _WIN32
	#include<windows.h>
#else
#endif

//直接从系统上按页申请空间
inline static void* SystemAlloc(size_t kpage)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
	// linux下brk mmap等
#endif
	if (ptr == nullptr)
		throw std::bad_alloc();
	return ptr;
}

//释放掉从系统中申请的空间
inline static void SystemFree(void* ptr) 
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	// sbrk unmmap等
#endif
}

static void*& NextObj(void* obj) //封装一个取头上4/8个字节的值，便于用来存放下一个节点的地址
{
	return *(void**)obj;
}

//管理切分好的小对象自由链表：
class FreeList
{
public:
	void Push(void* obj) {
		//头插
		//*(void**)obj = _freelist;
		NextObj(obj) = _freelist;
		_freelist = obj;
		_size++;
	}

	void* Pop(void) {
		//头删
		assert(_freelist);
		void* obj = _freelist;
		_freelist = NextObj(obj);
		_size--;
		return obj;
	}

	void PushRange(void* start,void* end, size_t n)
		//push一段范围内的链表节点,start、end为起始和终止位置，n为个数
	{
		NextObj(end) = _freelist;
		_freelist = start;

		/*-----测试验证，打条件断点监视是不是真的有n个-----*/
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

	void PopRange(void*& start, void*& end, size_t n) //摘掉一段范围内的链表节点
	{
		assert(n <= _size);//检查调用该函数前是否出错
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

	size_t& GetSize() //获取链表节点个数
	{
		return _size;
	}

private:
	void* _freelist = nullptr;
	size_t _maxsize = 1;
	size_t _size = 0;			//记录链表节点个数
};

class SizeClass
{
public:

	static inline size_t _RoundUp(size_t size, size_t align_num)
// 整体控制在最多10%左右的内碎片浪费
// [1,128]					8byte对齐	    freelist[0,16)		16个桶
// [128+1,1024]				16byte对齐	    freelist[16,72)		56个桶
// [1024+1,8*1024]			128byte对齐	    freelist[72,128)	56个桶
// [8*1024+1,64*1024]		1024byte对齐    freelist[128,184)	56个桶
// [64*1024+1,256*1024]		8*1024byte对齐  freelist[184,208)	56个桶
// 小于等于256KB的内存就找ThreadCache要，大于256KB的内存就调用BigAlloc函数
	//子函数，给定一个大小，判断这个大小的内存使用AlignNum的位数对齐
	{
		return ((size + align_num - 1) & ~(align_num - 1));
	}
	static inline size_t RoundUp(size_t size) 
	//给定一个大小，判断这个大小的内存应该使用多少字节对齐
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
			//大于256KB的内存就按页来对齐
		}
	}

	static inline size_t _Index(size_t bytes, size_t align_shift) {
		return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
	}
	static inline size_t Index(size_t bytes)
		//找到对应内存大小的桶的索引
	{
		assert(bytes <= MAX_BYTES);
		// 每个区间有多少个链
		static int group_array[4] = { 16, 56, 56, 56 };//前四个区间对应的桶的个数
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
	//	找到对应区间的桶的索引
	//{
	//	assert(bytes <= MAX_BYTES);
	//	 每个区间有多少个链
	//	static int group_array[4] = { 16, 56, 56, 56 };//前四个区间对应的桶的个数
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

	static size_t GetMemNum(size_t size) //一次thread cache从central cache获取多少个小内存块
	{
		assert(size > 0);
		int num = MAX_BYTES / size;		//256kb/单个对象大小
		if (num < 2)	num = 2;
		if (num > 512)		num = 512;
		return num;
	}

	static size_t GetPageNum(size_t size) {
		size_t num = GetMemNum(size);
		size_t npage = num * size;
		//这里npage是总字节数
		npage >>= PAGE_SHIFT;
		//除以8k算出一共多少页
		if (npage == 0) {
			npage = 1;
		}
		//如果不够一页，就给一页
		return npage;
	}
	
private:

};

//管理多个连续页的大块内存跨度结构Span
struct Span
{
	PAGE_ID _pageId = 0;//页号
	size_t _n = 0;		//管理的页数

	Span* _next = nullptr;	//指向双向链表下一个节点
	Span* _prev = nullptr;	//指向双向链表上一个节点

	size_t _objSize = 0;  // 切好的小对象的大小
	size_t _use_count = 0;	//切好小块内存且被分配给thread cache的计数
	void* _freelist = nullptr;	//切好的小块内存的自由链表

	bool _isused = false;
	//对于页，在32位下，进程地址空间一般为4G，一页的大小一般为4k或者8k
	//如果以一页大小为8k，即2^13，那么一共有2^32 / 2^13 = 2^19个页
	//在64位下，一共有2^64 / 2^13 = 2^51个页
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

	void PushFront(Span* span) //头插
	{
		Insert(Begin(), span);
	}
	Span* PopFront()    //头删
	{
		Span* front = _head->_next;
		Erase(front);
		return front;
	}
	void Insert(Span* pos, Span* newSpan) //在pos位置的Span前面插入结点
	{
		assert(pos);
		assert(newSpan);
		Span* prev = pos->_prev;
		prev->_next = newSpan;
		newSpan->_next = pos;
		newSpan->_prev = prev;
		pos->_prev = newSpan;
	}
	void Erase(Span* pos)	//删除pos位置的结点
	{
		assert(pos);

		// 1.条件断点
		// 2.查看调试栈帧（查看函数调用先后）
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

	std::mutex _mtx;//加桶锁

private:
	Span* _head;	//头结点
};

