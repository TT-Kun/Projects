#define _CRT_SECURE_NO_WARNINGS 1
#include"snake.h"

void SetPos(short x, short y) /*��λ���λ��*/ {
	HANDLE houtput = NULL;
	//��ñ�׼����豸�ľ��
	houtput = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = { x,y };
	//��λ���λ��
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

void HideCursor() {/*���ع��*/
	HANDLE houtput = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursor_info = { 0 };
	GetConsoleCursorInfo(houtput, &cursor_info);
	cursor_info.bVisible = false;
	SetConsoleCursorInfo(houtput, &cursor_info);
}
 
void CreateWelcome()/*������ӭ����*/
{
	SetPos(16, 12);
	printf("��ӭ����̰����С��Ϸ��");
	SetPos(0, 28);
}

void CreateSpeci() {/*��ӡ˵��ҳ*/
	SetPos(10, 10);
	printf("��ʹ�á����������������������ߵ��ƶ���");
	SetPos(19, 12);
	printf("F1Ϊ���٣�F2Ϊ����");
	SetPos(15, 14);
	printf("ESC�˳���Ϸ��space��ͣ��Ϸ");
	SetPos(0, 28);
}

void CreateHelp() {/*��ӡ��Ϸ�����е�˵����Ϣ*/
	SetPos(66, 10);
	printf("��������ǽ��������������");
	SetPos(61, 14);
	printf("��ʹ�á����������������������ߵ��ƶ�");
	SetPos(70, 16);
	printf("F1Ϊ���٣�F2Ϊ����");
	SetPos(60, 18);
	printf("����ʳ���ֵ���󣬼���ʳ���ֵ���С");
	SetPos(65, 20);
	printf("ESC�˳���Ϸ��space��ͣ��Ϸ");
}

void CreateMap()//��ӡ��ͼ����Χ��ǽ��
{
	setlocale(LC_ALL, "");//�л������ص���
	SetPos(0, 0);
	for (short i = 0; i < 58; i += 2) {
		wprintf(L"% lc", WALL);
	}//��ӡ�����ǽ��
	SetPos(0, 26);
	for (short i = 0; i < 58; i += 2) {
		wprintf(L"% lc", WALL);
	}//��ӡ�����ǽ��
	for (short i = 1; i < 26; i++) {
		SetPos(0, i);
		wprintf(L"% lc", WALL);
		SetPos(56, i);
		wprintf(L"% lc", WALL);
	}//��ӡ����ǽ��
	SetPos(0, 28);//��ӡ���λ����ͼ�·���������ʾ�Ｗ�ҵ�ͼ
}

void CreateFood(Snake* ps) {/*����ʳ��*/
	//xȡֵ��2-54(��Ϊż��)	yȡֵ��1-25
	int x, y;
again:
	x = (rand() % 27 + 1) * 2;
	y = rand() % 25 + 1;
	
	//ʳ�������λ�ò��ܹ��������λ�ó�ͻ
	SnakeNode* cur = ps->pSnake;
	while (cur) {
		if (x == cur->x && y == cur->y) {
			goto again;
		}
		cur = cur->next;
	}//����goto��䣬�������ʳ��ĵ�ַ����������ĳ��λ�õ�ַ�غϣ��򷵻���������
	SnakeNode* pFood = (SnakeNode*)malloc(sizeof(SnakeNode));	
	if (pFood == NULL) {
		perror("CreateFood()::malloc()");	return;
	}//�ж��Ƿ�����ռ�ɹ�
	pFood->front = pFood->next = NULL;
	pFood->x = x;	pFood->y = y;
	ps->pfood = pFood;
	//��������ʳ���Snake�ṹ����ϵ����
	SetPos(pFood->x, pFood->y);
	wprintf(L"%lc", FOOD);
	SetPos(0, 28);
}

int NextIsFood(Snake* ps) {//�ж�Ҫǰ������һ������û��ʳ��
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

void HeadAppear(Snake* ps) {/*���߽�Ҫ�ƶ�������һ��λ�ô�ӡ��ͼ��,��ͷ��һ���ڵ�*/
	SnakeNode* NewHead = (SnakeNode*)malloc(sizeof(SnakeNode));
	//�������ж��ߵ��˶��������ҵ���һ��λ��
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
	//ͷ���µĽڵ�
	SetPos(NewHead->x, NewHead->y);
	wprintf(L"%lc", BODY);
}

void TailDisapp(Snake* ps) {//βɾ���һ���ڵ㲢�ÿո񸲸�BODYͼ��
	SnakeNode* tail = ps->prear;
	SetPos(ps->prear->x, ps->prear->y);
	printf("  ");
	ps->prear = ps->prear->front;
	ps->prear->next = NULL;
	free(tail);
}

void InitSnake(Snake* ps) {/*��ʼ��̰����*/
	SetPos(32, 5);
	SnakeNode* cur = NULL;
	for (int i = 0; i < 5; i++) {//������ʼ��̰���ߵ���������
		cur = (SnakeNode*)malloc(sizeof(SnakeNode));
		if (cur == NULL) {/*�ж�����ռ��Ƿ�ɹ�*/
			perror("InitSnake()::malloc()");
			return;
		}
		cur->next = NULL;	cur->front = NULL;
		cur->x = POS_X + i * 2;//��ǰԤ��ó�ʼ����ͷ���ڵ�λ��POS_X��POS_X
		cur->y = POS_Y;//����ÿ���ڵ������
		//ʹ��ͷ�巨����˫������
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
	ps->dir = RIGHT;//�������˶��ķ��������ҡ�
	ps->socre = 0;//����Ŀǰ�÷�
	ps->food_weight = 20;//����ʳ������
	ps->game_status = OK;//������Ϸ״̬
	ps->sleep_time = 250;//����ÿ���˶������������ʮ����
	SetPos(0, 28);
}

void KillByWall(Snake* ps) {
	if (ps->pSnake->x == 0 || ps->pSnake->x == 56 || ps->pSnake->y == 0 || ps->pSnake->y == 26) {
		ps->game_status = KILL_BY_WALL;
		SetPos(68, 5);
		printf(" �������ˣ���Ϸ������");
		SetPos(62, 7);
		printf("-------- ��ĵ÷��ǣ�%3d -------- ",ps->socre);
		SetPos(ps->pSnake->x, ps->pSnake->y);
		wprintf(L"%lc", L'��');
	}
}

void KillBySelf(Snake* ps) {
	SnakeNode* pcur = ps->pSnake->next;
	while (pcur) {
		if (pcur->x == ps->pSnake->x && pcur->y == ps->pSnake->y) {
			ps->game_status = KILL_BY_SELF;
			SetPos(68, 5);
			printf(" �������ˣ���Ϸ������");
			SetPos(62, 7);
			printf("-------- ��ĵ÷��ǣ�%3d -------- ", ps->socre);
			SetPos(ps->pSnake->x, ps->pSnake->y);
			wprintf(L"%lc", L'��');
			break;
		}
		pcur = pcur->next;
	}
}

void InitGame(Snake* ps) {/*��ʼ����Ϸ*/
	system("mode con cols=105 lines=35");//���ô��ڴ�С
	system("title ̰����");//��������
	HideCursor();//���ع�꺯��
	CreateWelcome();//������ӭ����
	system("echo ------------------ �밴 �� ���� ------------------&pause>nul");
	//�ǳ�����ֱͣ������������Ϣ
	system("cls");//�����Ļ

	CreateSpeci();//����˵������
	system("echo ------------------ �밴 �� ���� ------------------&pause>nul");
	system("cls");

	CreateMap();
	CreateHelp();//������Ϸ׼������
	InitSnake(ps);//��ʼ����
	CreateFood(ps);//����ʳ��

	SetPos(0, 28);
	system("echo ------------------- �밴�������ʼ��Ϸ -------------------&pause>nul");
}

void RunGame(Snake* ps)  {
	SetPos(0, 28);
	printf("                                                               ");
	do {
		SetPos(62, 7);
		printf("��ǰ�ĵ÷֣�%3d", ps->socre);
		SetPos(78, 7);
		printf("��ǰʳ��ķ�����%2d", ps->food_weight);
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
		else if (KEY_PRESS(VK_SPACE)) {//��ͣ/����
			SetPos(0, 28);
			printf("------------------- �밴�ո��������Ϸ -------------------");
			Pause();
		}
		else if (KEY_PRESS(VK_ESCAPE)) {//�˳�
			ps->game_status = END;
			break;
		}
		else if (KEY_PRESS(VK_F1)) {//����
			if (ps->sleep_time > 50) {
				ps->sleep_time -= 50;
				ps->food_weight += 5;
			}
		}
		else if (KEY_PRESS(VK_F2)) {//����
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