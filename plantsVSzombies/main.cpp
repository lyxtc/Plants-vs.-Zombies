#include<stdio.h>
#include<graphics.h>
#include<time.h>
#include"tools.h"

#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")


#define WIN_WIDTH 900
#define WIN_HEIGHT 600

enum {WAN_DOU, XIANG_RI_KUI,ZHI_WU_COUNT};

IMAGE imgBg;//��ʾ����ͼƬ
IMAGE imgBar;
IMAGE imgCards[ZHI_WU_COUNT];
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];


int curX, curY;//��ǰѡ�е�ֲ����ƶ������е�����
int curZhiWu; // 0��û��ѡ�У�1��ѡ���˵�һ��ֲ��

struct zhiwu {
	int type;      //0��û��ֲ��   1��ѡ���˵�һ��ֲ��
	int frameIndex;//����֡���
};

struct zhiwu map[3][9];

struct sunshineBall {
	int x, y; //��������Ʈ������е����꣨x���䣬��ֱ���£�
	int frameIndex;//��ǰ��ʾ��ͼƬ֡�����
	int destY;//Ʈ���Ŀ��λ�õ�y����
	bool used;//�Ƿ���ʹ��
	int timer; 
};

struct sunshineBall balls[10];
IMAGE imgSunshineBall[29];
int sunshine;

struct zm {
	int x, y;
	int frameIndex;
	bool used;
	int speed;
};
struct zm zms[10];
IMAGE imgZM[22];


bool fileExist(const char* name) {
	FILE* fp = fopen(name, "r");
	if (fp) {
		fclose(fp);
	}
	return fp != NULL;
}

void gameInit() {
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imgBar, "res/bar5.png");

	memset(imgZhiWu, 0, sizeof(imgZhiWu));
	memset(map, 0, sizeof(map));

	//��ʼ��ֲ�￨��
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		//����ֲ�￨�Ƶ��ļ���
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);
		
		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j+1);
			//���ж��ļ��Ƿ����
			if (fileExist(name)) {
				imgZhiWu[i][j] = new IMAGE;
				loadimage(imgZhiWu[i][j], name);
			}
			else {
				break;
			}
		}
	}
	curZhiWu = 0;
	sunshine = 50;

	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++) {
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}

	//�����������
	srand(time(NULL));
	//������Ϸ��ͼ�δ���
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);
	//��������
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY; // �����
	settextstyle(&f);
	setbkmode(TRANSPARENT);//���ñ���ģʽ������͸��
	setcolor(BLACK); //����������ɫ

	//��ʼ����ʬ����
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgZM[i], name);
	}
}

void drawZM() {
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used) {
			IMAGE* img = &imgZM[zms[i].frameIndex];
			putimagePNG(zms[i].x, zms[i].y - img->getheight(), img);
		}
	}
}

void updateWindow() {
	BeginBatchDraw();
	putimage(0, 0, &imgBg);
	putimagePNG(250, 0, &imgBar);
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		int x = 338 + i * 65;
		int y = 6;
		putimage(x, y, &imgCards[i]);
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				int x = 256 + j * 81;
				int y = 179 + i * 102 + 10;
				int zhiWuType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				putimagePNG(x, y, imgZhiWu[zhiWuType][index]);
			}
		}
	}

	//��Ⱦ �϶������е�ֲ��
	if (curZhiWu > 0) {
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}

	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];
			putimagePNG(balls[i].x, balls[i].y, img);
		}
	}

	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(276,67,scoreText);

	drawZM();

	EndBatchDraw();
}

void collectSunshine(ExMessage* msg) {
	int count = sizeof(balls) / sizeof(balls[0]);
	int w = imgSunshineBall[0].getwidth();
	int h = imgSunshineBall[0].getheight();
	for (int i = 0; i < count; i++) {
		if (balls[i].used) {
			int x = balls[i].x;
			int y = balls[i].y;
			if (msg->x > x && msg->x<x + w && msg->y>y && msg->y < y + h) {
				balls[i].used = false;
				sunshine += 25;
				mciSendString("play res/sunshine.mp3", NULL, 0, NULL);
			}
		}
	}
}

void userClick() {
	ExMessage msg;
	static int status = 0;
	if (peekmessage(&msg)) {
		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96) {
				int index = (msg.x - 338) / 65;
				//printf("%d\n", index);
				status = 1;
				curZhiWu = index + 1;
			}
			else {
				collectSunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status ==1) {
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP) {
			if (msg.x > 256 && msg.y > 179 && msg.y < 489) {
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 256) / 81;
				if (map[row][col].type == 0) {
					map[row][col].type = curZhiWu;
					map[row][col].frameIndex = 0;
				}
			}

			curZhiWu = 0;
			status = 0;
		}
	}
}

void createSunshine() {
	//���������ȡһ������ʹ�õ�
	static int count = 0;
	static int fre = 400;
	count++;
	if (count >= fre)
	{
		fre = 200 + rand() % 200;
		count = 0;

		//�������ȡһ������ʹ�õ�
		int ballMax = sizeof(balls) / sizeof(balls[0]);
		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax) return;
		balls[i].used = true;
		balls[i].frameIndex = 0;
		balls[i].x = rand() % (900 - 260) + 260;//260..900
		balls[i].y = 60;
		balls[i].destY = 200 + (rand() % 4) * 90;
		balls[i].timer = 0;
	}
}

void updateSunshine() {
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;
			if (balls[i].timer == 0) {
				balls[i].y += 2;
			}
			if (balls[i].y >= balls[i].destY) {
				balls[i].timer++;
				if (balls[i].timer > 100) {
					balls[i].used = false;
				}
			}
		}
	}
}

void createZM() {
	static int zmFre = 200;
	static int count = 0;
	count++;
	if (count > zmFre) {
		count = 0;
		zmFre = rand() % 200 + 300;
		int i;
		int zmMax = sizeof(zms) / sizeof(zms[0]);
		for (i = 0; i < zmMax && zms[i].used; i++);
		if (i < zmMax) {
			zms[i].used = true;
			zms[i].x = WIN_WIDTH;
			zms[i].y = 172 + (1 + rand() % 3) * 100;
			zms[i].speed = 1;

		}
	}
}

void updateZM() {
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	static int count = 0;
	count++;
	if (count > 2) {
		count = 0;
		//���½�ʬ��λ��
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				zms[i].x -= zms[i].speed;
				if (zms[i].x < 170) {
					printf("GAME OVER\n");
					MessageBox(NULL, "over", "over", 0);//���Ż�
					exit(0);//���Ż�
				}
			}
		}
	}
	static int count2 = 0;
	count2++;
	if (count2 > 4) {
		count2 = 0;
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
			}
		}
	}
}

void updateGame() {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				map[i][j].frameIndex++;
				int zhiWuType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				if (imgZhiWu[zhiWuType][index] == NULL) {
					map[i][j].frameIndex = 0;
				}
			}
		}
	}
	createSunshine();//��������
	updateSunshine();//��������״̬
	createZM();//������ʬ
	updateZM();//���½�ʬ��״̬
}

void startUI() {
	IMAGE imgBg,imgMenu1,imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");
	int flag = 0;
	while (1) {
		BeginBatchDraw();
		putimage(0, 0, &imgBg);
		putimagePNG(474, 75, flag ? &imgMenu2 : &imgMenu1);

		ExMessage msg;
		if (peekmessage(&msg)) {
			if (msg.message == WM_LBUTTONDOWN && msg.x > 474 && msg.x < 474 + 300 && msg.y>75 && msg.y < 75 + 140) {
				flag = 1;
			}
			else if (msg.message == WM_LBUTTONUP && flag == 1) {
				return;
			}
		}
		EndBatchDraw();
	}
}

int main(){
	gameInit();
	startUI();
	/*IMAGE* img = new IMAGE;
	img = imgZhiWu[1][1];
	int a = img->getwidth() / 2;
	int b = img->getheight() / 2;
	putimagePNG(10 - img->getwidth() / 2, 10 - img->getheight() / 2, img);*/
	int timer = 0;
	int flag = true;
	while (1) {
		userClick();
		timer += getDelay();
		if (timer > 20) {
			flag = true;
			timer = 0;
		}
		if (flag) {
			flag = false;
			updateWindow();
			updateGame();
		}
	}
	system("pause");
	return 0;
}