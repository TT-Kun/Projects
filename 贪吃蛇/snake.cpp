#define _CRT_SECURE_NO_WARNINGS 1
#include"snake.h"

void SetPos(short x, short y) /*定位光标位置*/ {
	HANDLE houtput = NULL;
	//获得标准输出设备的句柄
	houtput = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = { x,y };
	//定位光标位置
	SetConsoleCursorPosition(houtput, pos);
}

void Pause() {
	while (1) {
		Sleep(300);
		if (KEY_PRESS(VK_SPACE)) {
			SetPos(0, 28);
			printf("                                                          ");
			break;
		}
	}
}

void HideCursor() {/*隐藏光标*/
	HANDLE houtput = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursor_info = { 0 };
	GetConsoleCursorInfo(houtput, &cursor_info);
	cursor_info.bVisible = false;
	SetConsoleCursorInfo(houtput, &cursor_info);
}
 
void CreateWelcome()/*创建欢迎界面*/
{
	SetPos(16, 12);
	printf("欢迎来到贪吃蛇小游戏！");
	SetPos(0, 28);
}

void CreateSpeci() {/*打印说明页*/
	SetPos(10, 10);
	printf("请使用↑、↓、←、→键来控制蛇的移动。");
	SetPos(19, 12);
	printf("F1为加速，F2为减速");
	SetPos(15, 14);
	printf("ESC退出游戏，space暂停游戏");
	SetPos(0, 28);
}

void CreateHelp() {/*打印游戏窗口中的说明信息*/
	SetPos(66, 10);
	printf("不允许碰墙，不允许碰蛇身");
	SetPos(61, 14);
	printf("请使用↑、↓、←、→键来控制蛇的移动");
	SetPos(70, 16);
	printf("F1为加速，F2为减速");
	SetPos(60, 18);
	printf("加速食物分值会变大，减速食物分值会变小");
	SetPos(65, 20);
	printf("ESC退出游戏，space暂停游戏");
}

void CreateMap()//打印地图（周围的墙）
{
	setlocale(LC_ALL, "");//切换到本地地区
	SetPos(0, 0);
	for (short i = 0; i < 58; i += 2) {
		wprintf(L"% lc", WALL);
	}//打印上面的墙体
	SetPos(0, 26);
	for (short i = 0; i < 58; i += 2) {
		wprintf(L"% lc", WALL);
	}//打印下面的墙体
	for (short i = 1; i < 26; i++) {
		SetPos(0, i);
		wprintf(L"% lc", WALL);
		SetPos(56, i);
		wprintf(L"% lc", WALL);
	}//打印左右墙体
	SetPos(0, 28);//打印完后定位到地图下方，避免提示语挤兑地图
}

void CreateFood(Snake* ps) {/*创建食物*/
	//x取值：2-54(且为偶数)	y取值：1-25
	int x, y;
again:
	x = (rand() % 27 + 1) * 2;
	y = rand() % 25 + 1;
	
	//食物的生成位置不能够和蛇身的位置冲突
	SnakeNode* cur = ps->pSnake;
	while (cur) {
		if (x == cur->x && y == cur->y) {
			goto again;
		}
		cur = cur->next;
	}//利用goto语句，如果生成食物的地址和蛇身链表某个位置地址重合，则返回重新生成
	SnakeNode* pFood = (SnakeNode*)malloc(sizeof(SnakeNode));	
	if (pFood == NULL) {
		perror("CreateFood()::malloc()");	return;
	}//判断是否申请空间成功
	pFood->front = pFood->next = NULL;
	pFood->x = x;	pFood->y = y;
	ps->pfood = pFood;
	//将创建的食物和Snake结构体联系起来
	SetPos(pFood->x, pFood->y);
	wprintf(L"%lc", FOOD);
	SetPos(0, 28);
}

int NextIsFood(Snake* ps) {//判断要前进的下一个点有没有食物
	if (ps->dir == UP && ps->pSnake->x == ps->pfood->x && ps->pSnake->y - 1 == ps->pfood->y)
		return 1;
	else if (ps->dir == DOWN && ps->pSnake->x == ps->pfood->x && ps->pSnake->y + 1 == ps->pfood->y)
		return 1;
	else if (ps->dir == LEFT && ps->pSnake->y == ps->pfood->y && ps->pSnake->x - 2 == ps->pfood->x)
		return 1;
	else if (ps->dir == RIGHT && ps->pSnake->y == ps->pfood->y && ps->pSnake->x + 2 == ps->pfood->x)
		return 1;
	return 0;
}

void HeadAppear(Snake* ps) {/*在蛇将要移动到的下一个位置打印出图案,并头插一个节点*/
	SnakeNode* NewHead = (SnakeNode*)malloc(sizeof(SnakeNode));
	//接下来判断蛇的运动方向来找到下一个位置
	if (ps->dir == UP) {
		NewHead->x = ps->pSnake->x;
		NewHead->y = ps->pSnake->y - 1;
	}
	else if (ps->dir == DOWN) {
		NewHead->x = ps->pSnake->x;
		NewHead->y = ps->pSnake->y + 1;
	}
	else if (ps->dir == LEFT) {
		NewHead->x = ps->pSnake->x - 2;
		NewHead->y = ps->pSnake->y;
	}
	else if (ps->dir == RIGHT) {
		NewHead->x = ps->pSnake->x + 2;
		NewHead->y = ps->pSnake->y;
	}

	NewHead->front = NULL;
	NewHead->next = ps->pSnake;
	ps->pSnake->front = NewHead;
	ps->pSnake = NewHead;
	//头插新的节点
	SetPos(NewHead->x, NewHead->y);
	wprintf(L"%lc", BODY);
}

void TailDisapp(Snake* ps) {//尾删最后一个节点并用空格覆盖BODY图案
	SnakeNode* tail = ps->prear;
	SetPos(ps->prear->x, ps->prear->y);
	printf("  ");
	ps->prear = ps->prear->front;
	ps->prear->next = NULL;
	free(tail);
}

void InitSnake(Snake* ps) {/*初始化贪吃蛇*/
	SetPos(32, 5);
	SnakeNode* cur = NULL;
	for (int i = 0; i < 5; i++) {//创建初始化贪吃蛇的蛇身链表
		cur = (SnakeNode*)malloc(sizeof(SnakeNode));
		if (cur == NULL) {/*判断申请空间是否成功*/
			perror("InitSnake()::malloc()");
			return;
		}
		cur->next = NULL;	cur->front = NULL;
		cur->x = POS_X + i * 2;//提前预设好初始化蛇头所在的位置POS_X、POS_X
		cur->y = POS_Y;//设置每个节点的坐标
		//使用头插法创建双向链表
		if (ps->pSnake == NULL) {
			ps->pSnake = cur;
			ps->prear = cur;
		}
		else {
			cur->next = ps->pSnake;
			ps->pSnake->front = cur;
			ps->pSnake = cur;
		}
	}
	cur = ps->pSnake;
	while (cur) {
		SetPos(cur->x, cur->y);
		wprintf(L"%lc", BODY);
		cur = cur->next;
	}
	ps->dir = RIGHT;//设置蛇运动的方向是向右。
	ps->socre = 0;//设置目前得分
	ps->food_weight = 20;//设置食物重量
	ps->game_status = OK;//设置游戏状态
	ps->sleep_time = 250;//设置每次运动间隔是两百五十毫秒
	SetPos(0, 28);
}

void KillByWall(Snake* ps) {
	if (ps->pSnake->x == 0 || ps->pSnake->x == 56 || ps->pSnake->y == 0 || ps->pSnake->y == 26) {
		ps->game_status = KILL_BY_WALL;
		SetPos(68, 5);
		printf(" 你死亡了！游戏结束！");
		SetPos(62, 7);
		printf("-------- 你的得分是：%3d -------- ",ps->socre);
		SetPos(ps->pSnake->x, ps->pSnake->y);
		wprintf(L"%lc", L'×');
	}
}

void KillBySelf(Snake* ps) {
	SnakeNode* pcur = ps->pSnake->next;
	while (pcur) {
		if (pcur->x == ps->pSnake->x && pcur->y == ps->pSnake->y) {
			ps->game_status = KILL_BY_SELF;
			SetPos(68, 5);
			printf(" 你死亡了！游戏结束！");
			SetPos(62, 7);
			printf("-------- 你的得分是：%3d -------- ", ps->socre);
			SetPos(ps->pSnake->x, ps->pSnake->y);
			wprintf(L"%lc", L'×');
			break;
		}
		pcur = pcur->next;
	}
}

void InitGame(Snake* ps) {/*初始化游戏*/
	system("mode con cols=105 lines=35");//设置窗口大小
	system("title 贪吃蛇");//命名程序
	HideCursor();//隐藏光标函数
	CreateWelcome();//创建欢迎界面
	system("echo ------------------ 请按 → 继续 ------------------&pause>nul");
	//是程序暂停直到键盘输入信息
	system("cls");//清空屏幕

	CreateSpeci();//创建说明界面
	system("echo ------------------ 请按 → 继续 ------------------&pause>nul");
	system("cls");

	CreateMap();
	CreateHelp();//创建游戏准备界面
	InitSnake(ps);//初始化蛇
	CreateFood(ps);//创建食物

	SetPos(0, 28);
	system("echo ------------------- 请按任意键开始游戏 -------------------&pause>nul");
}

void RunGame(Snake* ps)  {
	SetPos(0, 28);
	printf("                                                               ");
	do {
		SetPos(62, 7);
		printf("当前的得分：%3d", ps->socre);
		SetPos(78, 7);
		printf("当前食物的分数：%2d", ps->food_weight);
		if (KEY_PRESS(VK_UP) && ps->dir != DOWN) {
			ps->dir = UP;
		}
		else if (KEY_PRESS(VK_DOWN) && ps->dir != UP) {
			ps->dir = DOWN;
		}
		else if (KEY_PRESS(VK_RIGHT) && ps->dir != LEFT) {
			ps->dir = RIGHT;
		}
		else if (KEY_PRESS(VK_LEFT) && ps->dir != RIGHT) {
			ps->dir = LEFT;
		}
		else if (KEY_PRESS(VK_SPACE)) {//暂停/继续
			SetPos(0, 28);
			printf("------------------- 请按空格键继续游戏 -------------------");
			Pause();
		}
		else if (KEY_PRESS(VK_ESCAPE)) {//退出
			ps->game_status = END;
			break;
		}
		else if (KEY_PRESS(VK_F1)) {//加速
			if (ps->sleep_time > 50) {
				ps->sleep_time -= 50;
				ps->food_weight += 5;
			}
		}
		else if (KEY_PRESS(VK_F2)) {//减速
			if (ps->sleep_time < 400) {
				ps->sleep_time += 50;
				ps->food_weight -= 5;
			}
		}

		if (NextIsFood(ps)) {
			free(ps->pfood);
			HeadAppear(ps);
			CreateFood(ps);
			ps->socre += ps->food_weight;
		}
		else {
			TailDisapp(ps);
			HeadAppear(ps);
		}
		KillByWall(ps);
		KillBySelf(ps);
		Sleep(ps->sleep_time);
   	} while (ps->game_status == OK);
		SetPos(0, 28);

}