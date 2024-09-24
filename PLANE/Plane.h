#pragma once
#include<iostream>
#include<graphics.h>
#include<vector>
#include<queue>
#include<mmsystem.h>  
#include <tchar.h>

#define WIN_WIDTH 460	//窗口宽度
#define WIN_HEIGHT 700	//窗口高度
#define HERO_HP	6		//英雄飞机血量
#define ENEMY_HP 3		//敌机血量
using namespace std;

class BK //背景（方便实现背景滚动效果）
{
public:
	BK(IMAGE& img) 
		:img(img),
		y(-WIN_HEIGHT)
	{
	}
	void show() {
		if (!y)	y = -WIN_HEIGHT;
		y += 2;
		putimage(0, y, &img);

	}
private:
	IMAGE& img;
	int y;
};

class hero_plane //玩家飞机
{
public:
	hero_plane(IMAGE* hero_planey, IMAGE* hero_planeb, IMAGE* hp_down1y, IMAGE* hp_down1b, IMAGE* hp_down2y, IMAGE* hp_down2b)
		:point(0),hpy(hero_planey), hpb(hero_planeb), hpd1y(hp_down1y), hpd1b(hp_down1b), hpd2y(hp_down2y), hpd2b(hp_down2b)
		, HP(HERO_HP)
	{
		rect.left = WIN_WIDTH / 2 - (*hpy).getwidth() / 2;
		rect.top = WIN_HEIGHT - (*hpy).getheight();
		rect.right = rect.left + (*hpy).getwidth();
		rect.bottom = WIN_HEIGHT;
	}
	void MouseControl() {
		ExMessage mouse_mess;
		if (peekmessage(&mouse_mess, EM_MOUSE))
		{
			if (mouse_mess.y <= 450) {
				mouse_mess.y = 450;
			}
			rect.left = mouse_mess.x - (*hpy).getwidth() / 2;
			rect.top = mouse_mess.y - (*hpy).getheight() / 2;
			rect.right = rect.right = rect.left + (*hpy).getwidth();
			rect.bottom = rect.top + (*hpy).getheight();
		}
	}
	void show() {
		//putimage(185, 550, &imgy, SRCAND);
		//putimage(185, 550, &imgb, SRCPAINT);
		if (HP >= 5) {
			putimage(rect.left, rect.top, hpy, SRCAND);
			putimage(rect.left, rect.top, hpb, SRCPAINT);
		}
		else if (HP >= 3 && HP <= 4) {
			putimage(rect.left, rect.top, hpd1y, SRCAND);
			putimage(rect.left, rect.top, hpd1b, SRCPAINT);
		}
		else if (HP >= 1 && HP <= 2) {
			putimage(rect.left, rect.top, hpd2y, SRCAND);
			putimage(rect.left, rect.top, hpd2b, SRCPAINT);
		}
	}
	RECT& GetRect() { return rect; }

	long long point;
	int HP;
private:
	IMAGE* hpy;
	IMAGE* hpb;
	IMAGE* hpd1y;
	IMAGE* hpd1b;
	IMAGE* hpd2y;
	IMAGE* hpd2b;
	RECT rect;
	//int HP;
};

class plane_bullet //玩家飞机发射的子弹
{
public:
	plane_bullet(IMAGE* img, hero_plane* hp)
		: imgPtr(img), move(0)
	{
		rect.left = hp->GetRect().left + (hp->GetRect().right - hp->GetRect().left - imgPtr->getwidth()) / 2;
		rect.right = rect.left + imgPtr->getwidth();
		rect.top = hp->GetRect().top;
		rect.bottom = rect.top + imgPtr->getheight();
	}
	bool show() {
		if (rect.top <= 0) return false;
		rect.top -= 3;
		rect.bottom -= 3;
		move += 3;
		putimage(rect.left, rect.top, imgPtr);
		return true;
	}
	LONG getleft() {
		return rect.left;
	}
	RECT& GetRect() { return rect; }
	int move;
private:
	IMAGE* imgPtr;
	RECT rect;
};

// 前向声明 enemy_plane 类
class enemy_plane;

class enemy_bullet //敌方飞机发射的子弹
{
public:
	enemy_bullet(IMAGE* img, enemy_plane* ep);
	bool show();
	RECT& GetRect();
	int move;
private:
	IMAGE* bullet_ptr;
	RECT bullet_rect;
	enemy_plane* plane_ref;
};

class enemy_plane //敌方飞机
{
public:
	enemy_plane(IMAGE* enemy1y, IMAGE* enemy1b, IMAGE* enemy1_down1y, IMAGE* enemy1_down1b, IMAGE* enemy1_down2y, IMAGE* enemy1_down2b, int x);
	bool show();
	void CreateBullet(IMAGE* enemy_bullet);
	RECT& GetRect();
	int enemy1_hp;
	int width() {
		return imgenemy1b->getwidth();
	}
	vector<enemy_bullet> enemy_bul_vec;
private:
	IMAGE* imgenemy1y;
	IMAGE* imgenemy1b;
	IMAGE* imgenemy1_down1y;
	IMAGE* imgenemy1_down1b;
	IMAGE* imgenemy1_down2y;
	IMAGE* imgenemy1_down2b;
	IMAGE* imgenemy_bullet;
	RECT rect;
};

void transparentimage(int x, int y, IMAGE img);//便于直接使用透明背景贴图的函数

bool is_click(int x, int y, RECT& r); //判断是否点击指定区域

bool RectCrashRect(RECT& r1, RECT& r2); //判断两个贴图的碰撞

void Welcome();	//初始化界面

void Over(long long& kill);//游戏结束结算

void CreateEnemy(vector<enemy_plane>& ep_vec, IMAGE& enemy1y, IMAGE& enemy1b, IMAGE& enemy1_down1y, IMAGE& enemy1_down1b, IMAGE& enemy1_down2y, IMAGE& enemy1_down2b);

void CreatePlaneBullet(vector<plane_bullet>& bul_vec, hero_plane& hp, IMAGE& img); //创建弹药

void EnemyShow(vector<enemy_plane>& ep_vec);//敌机运动

void BulletShow(vector<plane_bullet>& bul_vec);//弹药运动

void DeleteEnemy(vector<enemy_plane>& ep_vec);//删除出界后的敌机

void DeleteBullet(std::vector<plane_bullet>& bul_vec);//删除出界弹药

void DeleteEnemyBullet(vector<enemy_bullet>& enemy_bul_vec);//删除敌方出界子弹，使程序更高效

void DestroyEnemy(vector<plane_bullet>& bul_vec, vector<enemy_plane>& ep_vec, hero_plane& hp);//成功击中敌机后的操作

void RunGame(); //游戏主体运行


/*
技术要点/实现功能：
1、地图的绘制（移动）
2、主角战机的移动和绘制
3、敌机的生成、绘制和移动
4、主角战机发射的弹药的移动和绘制
5、弹药和敌机的碰撞判定
6、敌机/主角战机坠毁
7、开始、结束界面以及得分栏等
*/