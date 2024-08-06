#define _CRT_SECURE_NO_WARNINGS 1
#include"snake.h"

void test()/*完成的是游戏的测试逻辑*/
{
	Snake snake = { 0 };
	//创建贪吃蛇
	InitGame(&snake);
	/*初始化游戏：欢迎界面、游戏介绍、地图初始化*/
 	RunGame(&snake);
	//运行游戏
} 

int main() 
{
	setlocale(LC_ALL, "");
	srand((unsigned int)time(NULL));
	test();
	return 0;
}
//几个特殊符号■(蛇身)★（食物）□（墙壁）