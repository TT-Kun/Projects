#include "plane.h"

void transparentimage(int x, int y, IMAGE img) //����ֱ��ʹ��͸��������ͼ�ĺ����������ڹ�����չ
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

bool is_click(int x, int y, RECT& r) //�ж��Ƿ���ָ������
{
	return (r.left <= x && r.right >= x && r.top <= y && r.bottom >= y);
}

bool RectCrashRect(RECT& r1, RECT& r2) //�ж�������ͼ����ײ
{
	RECT r;
	r.left = r1.left - (r2.right - r2.left);
	r.right = r1.right;
	r.top = r1.top - (r2.bottom - r2.top);
	r.bottom = r1.bottom;

	return (r.left < r2.left&& r2.left <= r.right && r.top <= r2.top && r2.top <= r.bottom);
}

void Welcome()		//��ʼ������
{
	LPCTSTR title = _T("��ɻ�");
	LPCTSTR title_play = _T("��ʼ��Ϸ");
	LPCTSTR title_exit = _T("�˳���Ϸ");

	IMAGE background;	
	RECT title_playr, title_exitr;
	BeginBatchDraw();
	loadimage(&background, "./images/bk.png");
	putimage(0, 0, &background);
	setbkcolor(WHITE);
	//cleardevice();
	settextstyle(40, 0, _T("����"));
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
	
	//�ж�����Ƿ����˿�ʼ�����
	while (true) {
		ExMessage mouse_mess;
		getmessage(&mouse_mess, EM_MOUSE);
		if (mouse_mess.lbutton) //�ж��������Ƿ���
		{
			if (is_click(mouse_mess.x, mouse_mess.y, title_playr)) {
				return;
				//�����title_playr��Χ�ڵ�����򷵻�,return���������������������Ϸ��
			}
			else if (is_click(mouse_mess.x, mouse_mess.y, title_exitr)) {
				exit(0);
				//��������title_exitr��Χ�ڵ����ֱ���˳�����
			}
		}
	}
}

void Over(long long& kill)//��Ϸ��������
{
	printf_s("o");
	TCHAR* str = new TCHAR[128];
	_stprintf_s(str, 128, _T("��ɱ����%llu"), kill);

	settextcolor(RED);
	outtextxy(WIN_WIDTH / 2 - textwidth(str) / 2, WIN_HEIGHT / 5, str);

	// �����¼� ����Enter���أ�
	LPCTSTR info = _T("��Enter����");
	settextstyle(20, 0, _T("����"));
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
//�����л�
{
	ep_vec.push_back(enemy_plane(&enemy1y, &enemy1b, &enemy1_down1y, &enemy1_down1b, &enemy1_down2y, &enemy1_down2b, abs(rand()) % (WIN_WIDTH - enemy1y.getwidth())));
}

void EnemyShow(vector<enemy_plane>&ep_vec) //�л��ƶ���ʾ
{
	for (int i = 0; i < ep_vec.size(); i++) {
		ep_vec[i].show();
	}
}

void CreatePlaneBullet(vector<plane_bullet>& plane_bul_vec,hero_plane& hp,IMAGE& img) //������ҷɻ��ӵ�
{
	plane_bul_vec.push_back(plane_bullet(&img, &hp));
}

void BulletShow(vector<plane_bullet>& bul_vec) //��ҷɻ��ӵ��ƶ���ʾ
{
	for (int i = 0; i < bul_vec.size();i++) {
		bul_vec[i].show();
	}
}

void DeleteEnemy(vector<enemy_plane>& ep_vec) //ɾ���з�����ķɻ�����߳���Ч��
{
	if (ep_vec.empty())	return;
	if (ep_vec[0].GetRect().top >= WIN_HEIGHT) {
		ep_vec.erase(ep_vec.begin());
	}
}

void DeleteBullet(vector<plane_bullet>& bul_vec) //ɾ����ҷɻ�������ӵ�����߳���Ч��
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

void DeleteEnemyBullet(vector<enemy_bullet>& enemy_bul_vec) //ɾ���з��ɻ�������ӵ�����߳���Ч��
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

void DestroyEnemy(vector<plane_bullet>& bul_vec, vector<enemy_plane>& ep_vec, hero_plane& hp) //�ɹ����ел���Ĳ���
{
	// ����л����ҷ��ɻ�����ײ
	for (auto ep = ep_vec.begin(); ep != ep_vec.end();ep++) {
		if (RectCrashRect((*ep).GetRect(), hp.GetRect())) {
			ep = ep_vec.erase(ep);
			hp.HP -= 3;
			break;
		}
	}
	// �����ӵ���л�����ײ
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
					//��ɾ��һ���л���ep�����������Ѿ�ʧЧ��������ѭ�����ܻ����δ�������Ϊ��
					//���������������ɾ���л�����ȷ�ظ��µ�������ȷ��ѭ������ȷ��
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

// ʵ�� enemy_bullet �ĳ�Ա����
enemy_bullet::enemy_bullet(IMAGE* img, enemy_plane* ep)//��ʼ��
	: bullet_ptr(img), move(0), plane_ref(ep)
{
	bullet_rect.left = (ep->width() - img->getwidth()) / 2 + ep->GetRect().left;
	bullet_rect.right = bullet_rect.left + img->getwidth();
	bullet_rect.top = ep->GetRect().bottom;
	bullet_rect.bottom = bullet_rect.top + img->getheight();
}

bool enemy_bullet::show() //�з��ɻ��ӵ����ƶ���ʾ
{
	if (bullet_rect.top <= 0) return false;
	bullet_rect.top += 2;
	bullet_rect.bottom += 2;
	move += 2;
	putimage(bullet_rect.left, bullet_rect.top, bullet_ptr);
	return true;
}

RECT& enemy_bullet::GetRect() //��ȡ�з��ɻ�������ӵ�����ͼ�����Ϣ
{
	return bullet_rect;
}

// ʵ�� enemy_plane �ĳ�Ա����
enemy_plane::enemy_plane(IMAGE* enemy1y, IMAGE* enemy1b, IMAGE* enemy1_down1y, IMAGE* enemy1_down1b, IMAGE* enemy1_down2y, IMAGE* enemy1_down2b, int x)
	//��ʼ��
	: imgenemy1y(enemy1y), imgenemy1b(enemy1b), imgenemy1_down1y(enemy1_down1y), imgenemy1_down1b(enemy1_down1b),
	imgenemy1_down2y(enemy1_down2y), imgenemy1_down2b(enemy1_down2b), enemy1_hp(ENEMY_HP)
{
	rect.left = x;
	rect.right = x + imgenemy1b->getwidth();
	rect.top = -imgenemy1b->getheight();
	rect.bottom = 0;
}

bool enemy_plane::show() //�з��ɻ����ƶ���ʾ
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

RECT& enemy_plane::GetRect() //��ȡ�з��ɻ���ͼ�����Ϣ
{
	return rect;
}

void RunGame() //��������
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
	vector<plane_bullet>plane_bul_vec;	//�ҷ��ɻ��ӵ�
	vector<enemy_bullet>enemy_bul_vec;	//�з��ɻ��ӵ�
	ep_vec.push_back(enemy_plane(&enemy1[0], &enemy1[1], &enemy1_down1[0], &enemy1_down1[1], &enemy1_down2[0], &enemy1_down2[1], 50));

	int count = 0;
	while (hp.HP>0) {
		count++;
		BeginBatchDraw();
		flushmessage();

		//��ʼ׼��
		bk.show();
		Sleep(8);
		hp.MouseControl();
		hp.show();

		//���ɵз��ɻ�
		if (ep_vec.empty() || ep_vec[ep_vec.size() - 1].GetRect().top>=50) {
			CreateEnemy(ep_vec, enemy1[0], enemy1[1], enemy1_down1[0], enemy1_down1[1], enemy1_down2[0], enemy1_down2[1]);
		}
		EnemyShow(ep_vec);
		DeleteEnemy(ep_vec);
		
		//�����ҷ��ɻ��ӵ�
		if (!plane_bul_vec.size()) {
			CreatePlaneBullet(plane_bul_vec, hp, bullet1);
		}
		else if (plane_bul_vec[plane_bul_vec.size() - 1].move >= 36) {
			CreatePlaneBullet(plane_bul_vec, hp, bullet1);
		}

		//���ɵл��ӵ�
		auto ep = ep_vec.begin();
		while (ep != ep_vec.end()) {
			if ((*ep).GetRect().bottom % 150 == 20) {
				//enemy_plane& epn = *ep;
				enemy_bullet eb(&bullet2, &(*ep));
				enemy_bul_vec.push_back(eb);
			}
			ep++;
		}
		//�л��ӵ������ƶ��ͻ����ж�
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


