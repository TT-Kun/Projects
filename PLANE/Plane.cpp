#include "plane.h"

void transparentimage(int x, int y, IMAGE img) //便于直接使用透明背景贴图的函数，可用于功能拓展
{
    IMAGE img1;
    DWORD* d1;
    img1 = img;
    d1 = GetImageBuffer(&img1);
    float h, s, l;
    for (int i = 0; i < img1.getheight() * img1.getwidth(); i++) {
        RGBtoHSL(BGR(d1[i]), &h, &s, &l);
        if (l < 0.03) {
            d1[i] = BGR(WHITE);
        }
        if (d1[i] != BGR(WHITE)) {
            d1[i] = 0;
        }
    }
    putimage(x, y, &img1, SRCAND);
    putimage(x, y, &img, SRCPAINT);
}

bool is_click(int x, int y, RECT& r) //判断是否点击指定区域
{
	return (r.left <= x && r.right >= x && r.top <= y && r.bottom >= y);
}

bool RectCrashRect(RECT& r1, RECT& r2) //判断两个贴图的碰撞
{
	RECT r;
	r.left = r1.left - (r2.right - r2.left);
	r.right = r1.right;
	r.top = r1.top - (r2.bottom - r2.top);
	r.bottom = r1.bottom;

	return (r.left < r2.left&& r2.left <= r.right && r.top <= r2.top && r2.top <= r.bottom);
}

void Welcome()		//初始化界面
{
	LPCTSTR title = _T("打飞机");
	LPCTSTR title_play = _T("开始游戏");
	LPCTSTR title_exit = _T("退出游戏");

	IMAGE background;	
	RECT title_playr, title_exitr;
	BeginBatchDraw();
	loadimage(&background, "./images/bk.png");
	putimage(0, 0, &background);
	setbkcolor(WHITE);
	//cleardevice();
	settextstyle(40, 0, _T("黑体"));
	settextcolor(BLACK);

	outtextxy(WIN_WIDTH / 2 - textwidth(title) / 2,WIN_HEIGHT/5,title);

	outtextxy(WIN_WIDTH / 2 - textwidth(title_play) / 2, WIN_HEIGHT *2 / 5, title_play);
	title_playr.left = WIN_WIDTH / 2 - textwidth(title_play) / 2;
	title_playr.right = title_playr.left + textwidth(title_play);
	title_playr.top = WIN_HEIGHT *2/ 5;
	title_playr.bottom = title_playr.top + textheight(title_play);
	
	outtextxy(WIN_WIDTH / 2 - textwidth(title_exit) / 2, WIN_HEIGHT *3 / 5, title_exit);
	title_exitr.left = WIN_WIDTH / 2 - textwidth(title_exit) / 2;
	title_exitr.right = title_exitr.left + textwidth(title_exit);
	title_exitr.top = WIN_HEIGHT *3/ 5;
	title_exitr.bottom = title_exitr.top + textheight(title_exit);

	EndBatchDraw();
	
	//判断鼠标是否点击了开始或结束
	while (true) {
		ExMessage mouse_mess;
		getmessage(&mouse_mess, EM_MOUSE);
		if (mouse_mess.lbutton) //判断鼠标左键是否按下
		{
			if (is_click(mouse_mess.x, mouse_mess.y, title_playr)) {
				return;
				//如果在title_playr范围内点击，则返回,return后接着主函数继续运行游戏。
			}
			else if (is_click(mouse_mess.x, mouse_mess.y, title_exitr)) {
				exit(0);
				//如果如果在title_exitr范围内点击，直接退出程序
			}
		}
	}
}

void Over(long long& kill)//游戏结束结算
{
	printf_s("o");
	TCHAR* str = new TCHAR[128];
	_stprintf_s(str, 128, _T("击杀数：%llu"), kill);

	settextcolor(RED);
	outtextxy(WIN_WIDTH / 2 - textwidth(str) / 2, WIN_HEIGHT / 5, str);

	// 键盘事件 （按Enter返回）
	LPCTSTR info = _T("按Enter返回");
	settextstyle(20, 0, _T("黑体"));
	outtextxy(WIN_WIDTH - textwidth(info), WIN_HEIGHT - textheight(info), info);

	while (true)
	{
		ExMessage mess;
		getmessage(&mess, EM_KEY);
		if (mess.vkcode == 0x0D)
		{
			return;
		}
	}
}

void CreateEnemy(vector<enemy_plane>& ep_vec, IMAGE& enemy1y, IMAGE& enemy1b,IMAGE& enemy1_down1y, IMAGE& enemy1_down1b, IMAGE& enemy1_down2y, IMAGE& enemy1_down2b) 
//创建敌机
{
	ep_vec.push_back(enemy_plane(&enemy1y, &enemy1b, &enemy1_down1y, &enemy1_down1b, &enemy1_down2y, &enemy1_down2b, abs(rand()) % (WIN_WIDTH - enemy1y.getwidth())));
}

void EnemyShow(vector<enemy_plane>&ep_vec) //敌机移动显示
{
	for (int i = 0; i < ep_vec.size(); i++) {
		ep_vec[i].show();
	}
}

void CreatePlaneBullet(vector<plane_bullet>& plane_bul_vec,hero_plane& hp,IMAGE& img) //创建玩家飞机子弹
{
	plane_bul_vec.push_back(plane_bullet(&img, &hp));
}

void BulletShow(vector<plane_bullet>& bul_vec) //玩家飞机子弹移动显示
{
	for (int i = 0; i < bul_vec.size();i++) {
		bul_vec[i].show();
	}
}

void DeleteEnemy(vector<enemy_plane>& ep_vec) //删除敌方出界的飞机，提高程序效率
{
	if (ep_vec.empty())	return;
	if (ep_vec[0].GetRect().top >= WIN_HEIGHT) {
		ep_vec.erase(ep_vec.begin());
	}
}

void DeleteBullet(vector<plane_bullet>& bul_vec) //删除玩家飞机出界的子弹，提高程序效率
{
	auto it = bul_vec.begin();
	while (it != bul_vec.end()) {
		if (it->move > WIN_HEIGHT) {
			it = bul_vec.erase(it);
		}
		else {
			++it;
		}
	}
}

void DeleteEnemyBullet(vector<enemy_bullet>& enemy_bul_vec) //删除敌方飞机出界的子弹，提高程序效率
{
	auto it = enemy_bul_vec.begin();
	while (it != enemy_bul_vec.end()) {
		if (it->move > WIN_HEIGHT) {
			it = enemy_bul_vec.erase(it);
		}
		else {
			++it;
		}
	}
}

void DestroyEnemy(vector<plane_bullet>& bul_vec, vector<enemy_plane>& ep_vec, hero_plane& hp) //成功击中敌机后的操作
{
	// 处理敌机与我方飞机的碰撞
	for (auto ep = ep_vec.begin(); ep != ep_vec.end();ep++) {
		if (RectCrashRect((*ep).GetRect(), hp.GetRect())) {
			ep = ep_vec.erase(ep);
			hp.HP -= 3;
			break;
		}
	}
	// 处理子弹与敌机的碰撞
	auto bul = bul_vec.begin();
	while (bul != bul_vec.end()) {
		auto ep = ep_vec.begin();
		while (ep != ep_vec.end()) {
			if (RectCrashRect((*bul).GetRect(), (*ep).GetRect())) {
				bul = bul_vec.erase(bul);
				(*ep).enemy1_hp--;
				if ((*ep).enemy1_hp <= 0) {
					ep = ep_vec.erase(ep);
					hp.point++;
					if (ep == ep_vec.end()) {
						break;
					}
					//当删除一个敌机后，ep迭代器可能已经失效，后续的循环可能会出现未定义的行为。
					//解决方法可以是在删除敌机后，正确地更新迭代器，确保循环的正确性
				}
				break;
			}
			else {
				++ep;
			}
		}
		if (bul != bul_vec.end()) {
			++bul;
		}
	}
}

// 实现 enemy_bullet 的成员函数
enemy_bullet::enemy_bullet(IMAGE* img, enemy_plane* ep)//初始化
	: bullet_ptr(img), move(0), plane_ref(ep)
{
	bullet_rect.left = (ep->width() - img->getwidth()) / 2 + ep->GetRect().left;
	bullet_rect.right = bullet_rect.left + img->getwidth();
	bullet_rect.top = ep->GetRect().bottom;
	bullet_rect.bottom = bullet_rect.top + img->getheight();
}

bool enemy_bullet::show() //敌方飞机子弹的移动显示
{
	if (bullet_rect.top <= 0) return false;
	bullet_rect.top += 2;
	bullet_rect.bottom += 2;
	move += 2;
	putimage(bullet_rect.left, bullet_rect.top, bullet_ptr);
	return true;
}

RECT& enemy_bullet::GetRect() //获取敌方飞机发射的子弹的贴图相关信息
{
	return bullet_rect;
}

// 实现 enemy_plane 的成员函数
enemy_plane::enemy_plane(IMAGE* enemy1y, IMAGE* enemy1b, IMAGE* enemy1_down1y, IMAGE* enemy1_down1b, IMAGE* enemy1_down2y, IMAGE* enemy1_down2b, int x)
	//初始化
	: imgenemy1y(enemy1y), imgenemy1b(enemy1b), imgenemy1_down1y(enemy1_down1y), imgenemy1_down1b(enemy1_down1b),
	imgenemy1_down2y(enemy1_down2y), imgenemy1_down2b(enemy1_down2b), enemy1_hp(ENEMY_HP)
{
	rect.left = x;
	rect.right = x + imgenemy1b->getwidth();
	rect.top = -imgenemy1b->getheight();
	rect.bottom = 0;
}

bool enemy_plane::show() //敌方飞机的移动显示
{
	if (rect.top >= WIN_HEIGHT) return false;
	rect.top += 1;
	rect.bottom += 1;
	if (enemy1_hp == 3) {
		putimage(rect.left, rect.top, imgenemy1y, SRCAND);
		putimage(rect.left, rect.top, imgenemy1b, SRCPAINT);
	}
	else if (enemy1_hp == 2) {
		putimage(rect.left, rect.top, imgenemy1_down1y, SRCAND);
		putimage(rect.left, rect.top, imgenemy1_down1b, SRCPAINT);
	}
	else if (enemy1_hp == 1) {
		putimage(rect.left, rect.top, imgenemy1_down2y, SRCAND);
		putimage(rect.left, rect.top, imgenemy1_down2b, SRCPAINT);
	}
	return true;
}

RECT& enemy_plane::GetRect() //获取敌方飞机贴图相关信息
{
	return rect;
}

void RunGame() //主体运行
{
	setbkcolor(WHITE);
	cleardevice();
	IMAGE background;
	IMAGE my_plane[2];
	IMAGE hpd1[2];
	IMAGE hpd2[2];
	IMAGE enemy1[2];
	IMAGE enemy1_down1[2];
	IMAGE enemy1_down2[2];
	IMAGE enemy2[2];
	IMAGE bullet1;
	IMAGE bullet2;

	loadimage(&background, _T("./images/bk2.png"),WIN_WIDTH);
	putimage(0, 0, &background);
	
	loadimage(&my_plane[0], _T("./images/me1y.png"),80,88);
	loadimage(&my_plane[1], _T("./images/me1b.png"), 80, 88);
	loadimage(&hpd1[0], _T("./images/hpd1y.png"), 80, 88);
	loadimage(&hpd1[1], _T("./images/hpd1b.png"), 80, 88);
	loadimage(&hpd2[0], _T("./images/hpd2y.png"), 80, 88);
	loadimage(&hpd2[1], _T("./images/hpd2b.png"),80,88);

	loadimage(&bullet1, _T("./images/bullet1.png"));
	loadimage(&bullet2, _T("./images/bullet2.png"));

	loadimage(&enemy1[0], _T("./images/enemy1y.png"));
	loadimage(&enemy1[1], _T("./images/enemy1b.png"));
	loadimage(&enemy1_down1[0], _T("./images/enemy1_down1y.png"));
	loadimage(&enemy1_down1[1], _T("./images/enemy1_down1b.png"));
	loadimage(&enemy1_down2[0], _T("./images/enemy1_down2y.png"));
	loadimage(&enemy1_down2[1], _T("./images/enemy1_down2b.png"));


	BK bk = BK(background);
	hero_plane hp(&my_plane[0], &my_plane[1],&hpd1[0],&hpd1[1],&hpd2[0],&hpd2[1]);


	vector<enemy_plane>ep_vec;
	vector<plane_bullet>plane_bul_vec;	//我方飞机子弹
	vector<enemy_bullet>enemy_bul_vec;	//敌方飞机子弹
	ep_vec.push_back(enemy_plane(&enemy1[0], &enemy1[1], &enemy1_down1[0], &enemy1_down1[1], &enemy1_down2[0], &enemy1_down2[1], 50));

	int count = 0;
	while (hp.HP>0) {
		count++;
		BeginBatchDraw();
		flushmessage();

		//开始准备
		bk.show();
		Sleep(8);
		hp.MouseControl();
		hp.show();

		//生成敌方飞机
		if (ep_vec.empty() || ep_vec[ep_vec.size() - 1].GetRect().top>=50) {
			CreateEnemy(ep_vec, enemy1[0], enemy1[1], enemy1_down1[0], enemy1_down1[1], enemy1_down2[0], enemy1_down2[1]);
		}
		EnemyShow(ep_vec);
		DeleteEnemy(ep_vec);
		
		//生成我方飞机子弹
		if (!plane_bul_vec.size()) {
			CreatePlaneBullet(plane_bul_vec, hp, bullet1);
		}
		else if (plane_bul_vec[plane_bul_vec.size() - 1].move >= 36) {
			CreatePlaneBullet(plane_bul_vec, hp, bullet1);
		}

		//生成敌机子弹
		auto ep = ep_vec.begin();
		while (ep != ep_vec.end()) {
			if ((*ep).GetRect().bottom % 150 == 20) {
				//enemy_plane& epn = *ep;
				enemy_bullet eb(&bullet2, &(*ep));
				enemy_bul_vec.push_back(eb);
			}
			ep++;
		}
		//敌机子弹数组移动和击中判定
		auto eb = enemy_bul_vec.begin();
		while (eb != enemy_bul_vec.end()) {
			if (RectCrashRect((*eb).GetRect(), hp.GetRect())) {
				hp.HP -= 1;
				eb = enemy_bul_vec.erase(eb);
			}
			(*eb).show();
			eb++;
		}


		BulletShow(plane_bul_vec);
		DeleteBullet(plane_bul_vec);
		DeleteEnemyBullet(enemy_bul_vec);
		DestroyEnemy(plane_bul_vec, ep_vec, hp);
		EndBatchDraw();
	}
	Over(hp.point);
}


