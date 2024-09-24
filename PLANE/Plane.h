#pragma once
#include<iostream>
#include<graphics.h>
#include<vector>
#include<queue>
#include<mmsystem.h>  
#include <tchar.h>

#define WIN_WIDTH 460	//���ڿ��
#define WIN_HEIGHT 700	//���ڸ߶�
#define HERO_HP	6		//Ӣ�۷ɻ�Ѫ��
#define ENEMY_HP 3		//�л�Ѫ��
using namespace std;

class BK //����������ʵ�ֱ�������Ч����
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

class hero_plane //��ҷɻ�
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

class plane_bullet //��ҷɻ�������ӵ�
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

// ǰ������ enemy_plane ��
class enemy_plane;

class enemy_bullet //�з��ɻ�������ӵ�
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

class enemy_plane //�з��ɻ�
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

void transparentimage(int x, int y, IMAGE img);//����ֱ��ʹ��͸��������ͼ�ĺ���

bool is_click(int x, int y, RECT& r); //�ж��Ƿ���ָ������

bool RectCrashRect(RECT& r1, RECT& r2); //�ж�������ͼ����ײ

void Welcome();	//��ʼ������

void Over(long long& kill);//��Ϸ��������

void CreateEnemy(vector<enemy_plane>& ep_vec, IMAGE& enemy1y, IMAGE& enemy1b, IMAGE& enemy1_down1y, IMAGE& enemy1_down1b, IMAGE& enemy1_down2y, IMAGE& enemy1_down2b);

void CreatePlaneBullet(vector<plane_bullet>& bul_vec, hero_plane& hp, IMAGE& img); //������ҩ

void EnemyShow(vector<enemy_plane>& ep_vec);//�л��˶�

void BulletShow(vector<plane_bullet>& bul_vec);//��ҩ�˶�

void DeleteEnemy(vector<enemy_plane>& ep_vec);//ɾ�������ĵл�

void DeleteBullet(std::vector<plane_bullet>& bul_vec);//ɾ�����絯ҩ

void DeleteEnemyBullet(vector<enemy_bullet>& enemy_bul_vec);//ɾ���з������ӵ���ʹ�������Ч

void DestroyEnemy(vector<plane_bullet>& bul_vec, vector<enemy_plane>& ep_vec, hero_plane& hp);//�ɹ����ел���Ĳ���

void RunGame(); //��Ϸ��������


/*
����Ҫ��/ʵ�ֹ��ܣ�
1����ͼ�Ļ��ƣ��ƶ���
2������ս�����ƶ��ͻ���
3���л������ɡ����ƺ��ƶ�
4������ս������ĵ�ҩ���ƶ��ͻ���
5����ҩ�͵л�����ײ�ж�
6���л�/����ս��׹��
7����ʼ�����������Լ��÷�����
*/