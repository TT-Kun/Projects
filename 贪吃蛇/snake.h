#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include<locale.h>
#include<stdbool.h>
#include<time.h>

#define WALL L'□'
#define BODY L'■'
#define FOOD L'★'
#define POS_X 24
#define POS_Y 8

#define KEY_PRESS(VK) (( GetAsyncKeyState (VK) & 0x1 ) ? 1:0)

enum DIRECTION {/*对蛇移动状态的枚举，给第一个赋值1，接下来的每个变量都会以此＋1*/
	UP = 1,
	DOWN,
	LEFT,
	RIGHT
};

enum GAME_STATUS {/*枚举游戏运行状态*/
	OK,//正常运行
	KILL_BY_WALL,//撞墙死亡
	KILL_BY_SELF,//撞蛇身死亡
	END//正常退出/结束
};

typedef struct SnakeNode /*蛇链表节点*/ {
	int x;
	int y; 
	//坐标
	struct SnakeNode* next;//下一个节点
	struct SnakeNode* front;//上一个节点
}SnakeNode,*pSnakeNode;

typedef	struct Snake {
	SnakeNode* pSnake;//维护整条蛇的指针，也是指向头节点的指针
	SnakeNode* prear;//指向尾节点的指针
	SnakeNode* pfood;//维护食物的指针
	enum  DIRECTION dir;//蛇头的⽅向
	enum GAME_STATUS game_status;//游戏状态
	int socre;//当前获得分数
	int food_weight;//默认每个⻝物10分
	int sleep_time;//每⾛⼀步休眠时间
}Snake;

void SetPos(short x, short y);//定位地图

void HideCursor();/*隐藏光标*/

void CreateWelcome();//打印欢迎界面

void CreateSpeci();//打印说明界面

void CreateHelp();//创建游戏窗口旁的信息提示

void CreateMap();//初始化地图

void CreateFood(Snake* ps);//创建食物

int NextIsFood(Snake* ps);//判断前进的下一个点是不是食物

void HeadAppear(Snake* ps);//打印出前进下一个点的图案并头插一个节点

void TailDisapp(Snake* ps);//尾删最后一个节点并用空格覆盖BODY图案

void InitSnake(Snake *ps);//初始化蛇

void InitGame(Snake* ps);//初始化游戏

void RunGame(Snake* ps);//运行游戏

void KillByWall(Snake* ps);//判定有无撞墙

void KillBySelf(Snake* ps);//判定有无撞自己