#pragma once
#include"Common.h"

//�����ڴ�أ�һ������N��С�Ŀռ�
//template<size_t N>
//class ObjectPool {
//};
//
//#ifdef _WIN32
//#include<windows.h>
//#else
//#endif

//// ֱ��ȥ���ϰ�ҳ����ռ�
//inline static void* SystemAlloc(size_t kpage)
//{ 
//#ifdef _WIN32
//	void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
//#else
//	// linux��brk mmap��
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
		//_freelist�ǿգ�˵���л��������ڴ棬����ʹ�û��������ڴ�
		//����ʹ��������ͷ��������ͷɾ
		{
			void* next = *(void**)_freelist;
			obj = (T*)_freelist;
			_freelist = next;
		}
		else
		{
			if (_remainbytes < sizeof(T)) {
				_remainbytes = 128 * 1024;
				/*����ռ��޷���T���������ܻ���ֻ���һ���ڴ治��һ��T����Ŀռ䣬
				���Իᵼ��_remainBytes�Ľ�����㣬���ռ��ֲ������ˣ�
				���if�����е��ж�Ӧ����_remainBytes < sizeof(T),
				��ʣ���ڴ治��һ�������Сʱ�����ؿ����ռ䣬�Ա���Խ�����*/
				//_memory = (char*)malloc(128 * 1024);
				_memory = (char*)SystemAlloc(_remainbytes >> 13);
				if (_memory == nullptr) {
					throw std::bad_alloc();
				}
			}
			obj = (T*)_memory;		//�����ȥ���µĿռ����ʼλ��
			//_memory += sizeof(T);		//�µĿռ䱻�����ʣ��ռ����ʼλ��
			size_t objsize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			_memory += objsize;
			/*����ָ������Ĵ�С��4���ֽڣ�32λ������8���ֽڣ�64λ�����������ڴ��ʱ����ͨ��
			������յģ���Ҫ�洢��һ���ڵ�ĵ�ַ����������T���͵����ʹ�СС��ָ���С��ʱ��
			�޷��洢һ����ַ������32λ��ָ���С4���ֽڣ���char����1���ֽڣ���ʱ�����������
			ֻ��һ���ֽڵĴ�С�Ļ���Ȼ�޷����һ����ַ�������Ҫ���Ƚϣ�������ʹ�СС��ָ�����
			���ʹ�С���򷵻�һ��ָ������Ĵ�С��*/
			_remainbytes -= objsize;	//ʣ��Ŀռ�
		}
		//��λnew����ʽ����T�Ĺ��캯����ʼ��
		new(obj)T;
		assert(obj);
		return obj;
	}
	void Delete(T* obj) {
		//��ʽ�������������������
		obj->~T();

		//���൱����ͷ��������ͷ��
			*(void**)obj = _freelist;
			_freelist = *(void**)obj;
		/*֮����ʹ��void������ǿ��ת����ֻ��Ϊ�ڲ�ͬλ�����£���ַ���ֽ�λ����ͬ
		������32λ�£�ָ�����͵Ĵ�С���ĸ��ֽڣ�����64λ�£�ָ�����͵Ĵ�С�ǰ˸��ֽ�
		��������Ҫͨ������ָ�����޸�obj��ָ��ͬʱ��˲�ͬ������ʹ��void����
		������ǿ������ת����*/
	}
	std::mutex _poolMtx; // ��ֹThreadCache����ʱ���뵽��ָ��
private:
	//void* _memory;
	//Ϊ�˷����ַ�ϵļӼ����㣬ʹ��char���ͣ�char����ռһ���ֽ�
	char* _memory = nullptr;	//ָ�����ڴ��ָ��
	void* _freelist = nullptr;	//ָ����ͷŵĿռ�
	size_t _remainbytes = 0;    // ����ڴ����зֹ�����ʣ���ֽ���
};

struct TreeNode
	//�������ڵ�
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
//	// �����ͷŵ��ִ�
//	const size_t Rounds = 5;
//	// ÿ�������ͷŶ��ٴ�
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