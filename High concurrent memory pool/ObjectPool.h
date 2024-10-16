#pragma once
#include"Common.h"

//定长内存池，一次申请N大小的空间
//template<size_t N>
//class ObjectPool {
//};
//
//#ifdef _WIN32
//#include<windows.h>
//#else
//#endif

//// 直接去堆上按页申请空间
//inline static void* SystemAlloc(size_t kpage)
//{ 
//#ifdef _WIN32
//	void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
//#else
//	// linux下brk mmap等
//#endif
//	if (ptr == nullptr)
//		throw std::bad_alloc();
//	return ptr;
//}

template<class T>
class ObjectPool {
public:
	T* New() {
		T* obj = nullptr;
		if (_freelist) 
		//_freelist非空，说明有还回来的内存，优先使用还回来的内存
		//可以使用类似无头结点链表的头删
		{
			void* next = *(void**)_freelist;
			obj = (T*)_freelist;
			_freelist = next;
		}
		else
		{
			if (_remainbytes < sizeof(T)) {
				_remainbytes = 128 * 1024;
				/*如果空间无法被T整除，可能会出现会有一块内存不够一个T对象的空间，
				所以会导致_remainBytes的结果非零，而空间又不够用了，
				因此if（）中的判定应该是_remainBytes < sizeof(T),
				即剩余内存不够一个对象大小时，则重开大块空间，以避免越界访问*/
				//_memory = (char*)malloc(128 * 1024);
				_memory = (char*)SystemAlloc(_remainbytes >> 13);
				if (_memory == nullptr) {
					throw std::bad_alloc();
				}
			}
			obj = (T*)_memory;		//申请出去的新的空间的起始位置
			//_memory += sizeof(T);		//新的空间被申请后，剩余空间的起始位置
			size_t objsize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			_memory += objsize;
			/*由于指针变量的大小是4个字节（32位）或者8个字节（64位），而回收内存的时候是通过
			链表回收的，需要存储加一个节点的地址，因此如果当T类型的类型大小小于指针大小的时候
			无法存储一个地址，例如32位下指针大小4个字节，而char类型1个字节，这时候回收链表中
			只有一个字节的大小的话显然无法存放一个地址，因此需要作比较，如果类型大小小于指针变量
			类型大小，则返回一个指针变量的大小。*/
			_remainbytes -= objsize;	//剩余的空间
		}
		//定位new，显式调用T的构造函数初始化
		new(obj)T;
		assert(obj);
		return obj;
	}
	void Delete(T* obj) {
		//显式调用析构函数清理对象
		obj->~T();

		//就相当于无头结点链表的头插
			*(void**)obj = _freelist;
			_freelist = *(void**)obj;
		/*之所以使用void类型来强制转换，只因为在不同位机器下，地址的字节位数不同
		比如在32位下，指针类型的大小是四个字节，而在64位下，指针类型的大小是八个字节
		因此如果想要通过二级指针来修改obj的指向，同时兼顾不同机器，使用void类型
		来进行强制类型转换。*/
	}
	std::mutex _poolMtx; // 防止ThreadCache申请时申请到空指针
private:
	//void* _memory;
	//为了方便地址上的加减运算，使用char类型，char类型占一个字节
	char* _memory = nullptr;	//指向大块内存的指针
	void* _freelist = nullptr;	//指向待释放的空间
	size_t _remainbytes = 0;    // 大块内存在切分过程中剩余字节数
};

struct TreeNode
	//二叉树节点
{
	int _val;
	TreeNode* _left;
	TreeNode* _right;

	TreeNode()
		:_val(0)
		, _left(nullptr)
		, _right(nullptr)
	{}
};

//void TestObjectPool()
//{
//	// 申请释放的轮次
//	const size_t Rounds = 5;
//	// 每轮申请释放多少次
//	const size_t N = 100000;
//	std::vector<TreeNode*> v1;
//	v1.reserve(N);
//	size_t begin1 = clock();
//	for (size_t j = 0; j < Rounds; ++j)
//	{
//		for (int i = 0; i < N; ++i)
//		{
//			v1.push_back(new TreeNode);
//		}
//		for (int i = 0; i < N; ++i)
//		{
//			delete v1[i];
//		}
//		v1.clear();
//	}
//	size_t end1 = clock();
//	std::vector<TreeNode*> v2;
//	v2.reserve(N);
//	ObjectPool<TreeNode> TNPool;
//	size_t begin2 = clock();
//	for (size_t j = 0; j < Rounds; ++j)
//	{
//		for (int i = 0; i < N; ++i)
//		{
//			v2.push_back(TNPool.New());
//		}
//		for (int i = 0; i < N; ++i)
//		{
//			TNPool.Delete(v2[i]);
//		}
//		v2.clear();
//	}
//	size_t end2 = clock();
//	cout << "new cost time:" << end1 - begin1 << endl;
//	cout << "object pool cost time:" << end2 - begin2 << endl;
//}