#include<stdio.h>
#include<graphics.h>
#include<conio.h>
#include<vector>	//�����ӿ�����ٶ�
#include<stdbool.h>
#include <ctime>
#include<mmstream.h>
#include"tool.h"
#pragma comment(lib, "winmm.lib")
using namespace std;

#define WIN_WIDTH 576			//��Ϸ���ڿ�
#define WIN_HIGHT 324			//��Ϸ���ڳ�
#define OBSTACLE_COUNT 10		//�ϰ�������

IMAGE imgBgs[4];				//ȫ�ֱ���ͼƬ
int bgX[4];						//����x��������
int bgSpeed[3] = { 1, 2, 4};	//�����ƶ��ٶ�
bool gameState;

typedef struct {		//����״̬�ṹ��
	bool Run;
	bool Jump;
	bool Squat;
}DinoState;

int DinoX;				//���� x ����
int DinoY;				//���� y ����
int DinoIndex;			//��ͼ���(������run
bool ReFrash;			//ˢ�»���ʹ��
int DinoBlood;
int Score;

IMAGE imgDinoRun[2];	//�����ܶ��ز�
bool RunState;			//�����ܶ�״̬

IMAGE imgDinoJump;		//��Ծ��ͼ
bool JumpState;			//��Ծ״̬
int JumMAXHight;		//�߶�����
int JumpOffset;			//ƫ����

bool SquatState;		//�¶�״̬
IMAGE imgDinoSquat[2];	//�¶�ͼƬ

//********************���Ż�����*********************
//IMAGE imgBarrier[3];//�ϰ���
//int BarriX;		//�ϰ���ˮƽ����
//int BarriY;		//�ϰ��ﴹֱ����
//bool BarrierExi;//�ϰ������
//***************************************************

typedef enum {
	LargeCactus,	//��������1~3
	SmallCactus,	//С������1~3
	Bird,			//��
	OBSTACLE_TYPE_COUNT
}obstacle_type;		//�ϰ�������

//IMAGE obstacleImg[3][10];//������ã����ֽ�ʡ�ڴ�
//���ǻ��������ɱ�������ӽ�ʡ,������е��ϰ���ͼƬ
vector<vector<IMAGE>>obstacleImages;

typedef struct obstacle{
	obstacle_type type;
	int ObstacleIndex;	//��ǰ��ʾͼƬ����� 
	int x, y;			//location
	int speed;
	int Direct_Damage;
	int FreshSpeed;
	bool exist;
	bool hited;			//��ײstate	
	//IMAGE img[10];	//����һ���ϰ��ﶼ�ü���һ�������زģ��˷��ڴ�
}obstacle_t;
obstacle_t obstacles[OBSTACLE_COUNT];

void init() {
	//************************** ����Ϸ���� *****************************
	initgraph(WIN_WIDTH, WIN_HIGHT, EX_SHOWCONSOLE);	//Pere'3th���ڵ���
	gameState = true;

	//*********************** ������Ϸ������Դ ***************************
	char name[64];
	for (int i = 0; i < 4; i++){
		sprintf(name, "res/bg%03d.png", i + 1);
		loadimage(&imgBgs[i], name);
		bgX[i] = 0;
	}
	//*********************** ���ؿ����ز�--�ܶ� **************************
	for (int i = 0; i < 2; i++){
		//"res/DinoRun1.png~res/DinoRun2.png"
		sprintf(name, "res/DinoRun%01d.png", i + 1);
		loadimage(&imgDinoRun[i], name);
	}
	//*********************** ���ؿ����ز�--��Ծ **************************
	loadimage(&imgDinoJump, "res/DinoJump.png");
	JumMAXHight = WIN_HIGHT - imgDinoRun[0].getheight() - 10 - 100;
	JumpOffset = -6;

	//*********************** ���ؿ����ز�--�¶� **************************
	loadimage(&imgDinoSquat[0], "res/DinoSquat1.png");
	loadimage(&imgDinoSquat[1], "res/DinoSquat2.png");
	
	//************************ ������ʼλ�ò��� ***************************
	DinoX = WIN_WIDTH / 2 - imgDinoRun[0].getwidth() / 2;
	DinoY = WIN_HIGHT     - imgDinoRun[0].getheight() - 5;
	DinoIndex = 0;

	//************************* ��ʼ������״̬ ****************************
	DinoBlood = 100;

	DinoState dinoState;
	dinoState = {true, false, false};
	RunState = dinoState.Run;
	JumpState = dinoState.Jump;	
	SquatState = dinoState.Squat;


	/* ���Ż�����
	//for (int i = 0; i < 3; i++) {	//�����ϰ���barrier�ز�
	//	//"res/SmallCactus1.png~res/SmallCactus3.png"
	//	sprintf(name, "res/SmallCactus%01d.png", i + 1);
	//	loadimage(&imgBarrier[i], name);
	//}
	////�����ϰ���
	//BarrierExi = false;
	//BarriX = WIN_WIDTH *2 ;
	//BarriY = 240 ;
	*/

	//*************************** �����ϰ��� *****************************
	//1.����С������
	vector<IMAGE> imgSmallCactusArrary;
	IMAGE imgCactus;
	for (int i = 0; i < 3; i++) {
		sprintf(name, "res/SmallCactus%01d.png", i + 1);
		loadimage(&imgCactus, name);
		imgSmallCactusArrary.push_back(imgCactus);
	}
	//2.���ش�������
	vector<IMAGE> imgLargeCactusArrary;
	for (int i = 0; i < 3; i++) {
		sprintf(name, "res/LargeCactus%01d.png", i + 1);
		loadimage(&imgCactus, name);
		imgLargeCactusArrary.push_back(imgCactus);
	}
	//3.������
	vector<IMAGE> imgBirdArrary;
	IMAGE imgBird;
	for (int i = 0; i < 2; i++) {
		sprintf(name, "res/Bird%01d.png", i + 1);
		loadimage(&imgBird, name);
		imgBirdArrary.push_back(imgBird);
	}
	//4.װ���������͵��ϰ���
	obstacleImages.push_back(imgSmallCactusArrary);	//С������
	obstacleImages.push_back(imgLargeCactusArrary);	//��������
	obstacleImages.push_back(imgBirdArrary);			//��
	//5.��ʼ���ϰ����
	for (int i = 0; i < OBSTACLE_COUNT; i++){
		obstacles[i].exist = false;
	}
	//6.�ϰ���"�˶���"ˢ������
	obstacles[0].FreshSpeed = 30;//SmallCactus
	obstacles[1].FreshSpeed = 30;//LargeCactus
	obstacles[2].FreshSpeed = 10;//bird

	//************************ ���ÿ�ʼ���� ****************************
	ReFrash = true;
	loadimage(0, "res/over.png");
	FlushBatchDraw(); 
	system("pause");
	//*************************** Ԥ������Ч ***************************
	preLoadSound("res/newhit.wav");
	mciSendString("play res/bg.mp3", 0, 0, 0);
	//*************************** ��ʼ������ ***************************
	Score = 0;
}

//�����ϰ���
void creatObstacle() {
	int i;
	for (i = 0; i < OBSTACLE_COUNT; i++) {
		if (obstacles[i].exist == false) {
			break;	//����������Ҫ����
		}
	}
	if (i >= OBSTACLE_COUNT) {
		return;		//û�ҵ��ͷ���
	}
	obstacles[i].exist = true;
	obstacles[i].hited = false;
	obstacles[i].ObstacleIndex = 0;
	obstacles[i].type = (obstacle_type)(rand() % OBSTACLE_TYPE_COUNT);
	obstacles[i].x = WIN_WIDTH;
	//�Զ����ϰ����ٶ�
	if (obstacles[i].type == LargeCactus) {
		obstacles[i].speed = 4;
		obstacles[i].Direct_Damage = 15;
		obstacles[i].y = 310 + 5 - (obstacleImages[obstacles[i].type][0]).getheight();
	}
	else if (obstacles[i].type == SmallCactus) {
		obstacles[i].speed = 5;
		obstacles[i].Direct_Damage = 10;
		obstacles[i].y = 310 + 5 - (obstacleImages[obstacles[i].type][0]).getheight();
	}
	else if (obstacles[i].type == Bird) {
		obstacles[i].speed = 6;
		obstacles[i].Direct_Damage = 20;
		obstacles[i].y = 280 + 5 - (obstacleImages[obstacles[i].type][0]).getheight();
	}
}

void checkHit() {
	for (int i = 0; i < OBSTACLE_COUNT; i++){
		DinoState dinoS = { RunState, JumpState, SquatState };
		if (obstacles[i].exist&& obstacles[i].hited == false) {
			int a1x, a1y, a2x, a2y;		//�����ο���
			int b1x, b1y, b2x, b2y;		//�ϰ���ο���
			int off = 30;				//��С��ⷶΧ ģ�����
			bool* p = (bool *) &dinoS;	//����ͳһ
			switch (0)
			{
			case 0:
				if (dinoS.Run) {
					a1x = DinoX + off;
					a1y = DinoY + off;
					a2x = DinoX + imgDinoRun[DinoIndex].getwidth() - off;
					a2y = DinoY + imgDinoRun[DinoIndex].getheight();
				}
				break;
			case 1:
				if (dinoS.Jump) {
					a1x = DinoX + off;
					a1y = DinoY + off;
					a2x = DinoX + imgDinoJump.getwidth() - off;
					a2y = DinoY + imgDinoJump.getheight();
				}
			case 2:
				if (dinoS.Squat){
					a1x = DinoX + off;
					a1y = DinoY + 20;
					a2x = DinoX + imgDinoSquat[DinoIndex].getwidth() - off;
					a2y = DinoY ;
				}
			default:
				break;
			}

			IMAGE img = obstacleImages[obstacles[i].type][obstacles[i].ObstacleIndex];
			b1x = obstacles[i].x + off;
			b1y = obstacles[i].y + off;
			b2x = obstacles[i].x + img.getwidth() - off;
			b2y = obstacles[i].y + img.getheight() - 10;

			if (rectIntersect(a1x, a1y, a2x, a2y, b1x, b1y, b2x, b2y)){
				DinoBlood -= obstacles[i].Direct_Damage;
				printf("Ѫ��ʣ�� %d\n", DinoBlood);
				playSound("res/newhit.wav");
				obstacles[i].hited = true;
			}
		}
	}
}

//����ˢ��
void ScreRefresh() {
	//��������
	for (int i = 0; i < 3; i++) {
		bgX[i] -= bgSpeed[i];
		if (bgX[i] < -WIN_WIDTH) {
			bgX[i] = 0;
		}
	}
	//�������������߼�
	if (JumpState){
		//������Ծ
		if (DinoY < JumMAXHight){
			//����
			JumpOffset = 6;
		}
		DinoY += JumpOffset;

		if (DinoY > WIN_HIGHT - imgDinoRun[0].getheight() - 10){
			//����ʼ�ײ�ֹͣ��Ծ && ����ƫ����
			JumpState = false;
			JumpOffset = -6;
		}
	}
	else if (SquatState){
		static	int count = 0;
		int delay[2] = { 4, 10 };
		count++;
		if (count >= delay[DinoIndex]) {
			count = 0;
			DinoIndex++;
			if (DinoIndex >= 2){
				DinoIndex = 0;
				SquatState = false;
			}
		}
	}
	else{
		//Ĭ���ܶ�״̬
		static	int count = 0;
		count++;
		if (count >= 4) {
			count = 0;
			DinoIndex = (DinoIndex + 1) % 2;
		}
	}

	//�����ϰ���
	static int frameCount = 0;	//n�ε���ˢ��ҳ��-ScreRefresh �Ŵ���
	static int BarriFre = 50;
	frameCount++;
	if (frameCount > BarriFre){
		frameCount = 0;
		BarriFre = 50 + rand() % 50;//�������,50~99Fre
		//********************���Ż�����*********************
		//if (!BarrierExi){
		//	BarrierExi = true;
		//	BarriX = WIN_WIDTH;
		//	BarriFre = 200 + rand()%300;//�������
		//}
		//***************************************************
		creatObstacle();
	}
	//********************���Ż�����*********************
	//barrier run
	/*if (BarrierExi){
		BarriX -= Speed[3];
		if (BarriX < -imgBarrier->getwidth()){
			BarrierExi = false;
		}
	}*/
	//***************************************************
	
	//���������ϰ��������
	static int freshCount = 0;
	for (int  i = 0; i < OBSTACLE_COUNT; i++){
		if (obstacles[i].exist)	{
			obstacles[i].x -= obstacles[i].speed + bgSpeed[2];
			if (obstacles[i].x < (obstacleImages[obstacles[i].type][0]).getwidth() * -2){
				obstacles[i].exist = false;
			}

			int len = obstacleImages[obstacles[i].type].size();
			if (freshCount > obstacles[obstacles[i].type].FreshSpeed){
				freshCount = 0;
				obstacles[i].ObstacleIndex = (obstacles[i].ObstacleIndex + 1) % len;
			}
		}
	}
	freshCount++;
	//��ײ���
	checkHit();
}

//��Ծ��������
void jump() {
	JumpState = true;
	ReFrash = true;		//��Ҫ����ˢ��
}
//�¶���������
void Squat() {
	SquatState = true;
	ReFrash = true;		//��Ҫ����ˢ��
	DinoIndex = 0;		//��ʼ�¶���Ϊ0
}

//�����û�����
void keyEvent() {
	//����ڱ�������������½���������
	char ch;
	if (_kbhit()){	//�а������� ���� true
		ch = _getch();	//���谴 enter ��ȡһ���ַ�
		if (ch == 'w') {
			jump();
		}
		else if (ch == 's') {
			Squat();
		}
	}
}
// ��Ⱦ��Ϸ����
void updateBg() {
	putimagePNG2(bgX[0], 0, &imgBgs[0]);	//����001 ��x=bgX[0]��
	putimagePNG2(bgX[1], 0, &imgBgs[1]);
	putimagePNG2(bgX[2], 0, &imgBgs[2]);

	putimagePNG2(bgX[0] + 576, 0, &imgBgs[0]);
	putimagePNG2(bgX[1] + 576, 0, &imgBgs[1]);
	putimagePNG2(bgX[2] + 576, 0, &imgBgs[2]);
}

//��Ⱦ�ϰ���
void updateBarrier() {
	/*���Ż�����
	if (BarrierExi) {
		putimagePNG2(BarriX, BarriY, &imgBarrier[1]);
	}*/
	
	for (int i = 0; i < OBSTACLE_COUNT; i++){
		if (obstacles[i].exist) {
			putimagePNG2(obstacles[i].x, obstacles[i].y, WIN_WIDTH, 
				&obstacleImages[obstacles[i].type][obstacles[i].ObstacleIndex]);
		}
	}
}

//��Ⱦ����
void updateDinosaur() {
	if (!SquatState){
		putimagePNG2(DinoX, DinoY, &imgDinoRun[DinoIndex]);
	}else{
		//�¶׸���ʱ����Ļ�߶�-��ǰͼ��ĸ߶�
		int y = WIN_HIGHT - (imgDinoSquat[DinoIndex]).getheight();
		putimagePNG2(DinoX, y, &imgDinoSquat[DinoIndex]);
	}
	
}

//��ȾѪ��
void updateBloodBar() {
	drawBloodBar(10, 10, 200, 10, 2, BLUE, DARKGRAY, RED, DinoBlood / 100.0);
}

void checkGameOver() {
	if (DinoBlood <= 0) {
		loadimage(0, "res/over.png");
		loadimage(0, "res/GameOver.png");
		FlushBatchDraw();
		mciSendString("stop res/bg.mp3", 0, 0, 0);
		system("pause");

		//��֮ͣ�󣬸��� or ����
		DinoBlood = 100;
		mciSendString("play res/bg.mp3", 0, 0, 0);
		
		gameState = false;
		Score = 0;
	}
}

void checkScore(int time) {
	if (time > 5){
		Score = Score + 1;
		printf("score: %d\n", Score);
	}
}

int checAndsetTime(int time) {
	if (!gameState){
		return 0;
	}
}

int main(void) {
	init();
	static int gameTime = 0;		//��ʱ����
	time_t start = time(nullptr);	//��ʱ����
	int timer = 0;
	while (1){
		//keyEvent && timer ˫����
		keyEvent();

		timer += getDelay();
		if (timer > 30)	{		//30ms ˢ��
			timer = 0;
			ReFrash = true;
		}
		if (ReFrash){
			BeginBatchDraw();	//���� ���Ƶ��
			updateBg();
			/*4.19.10:47�Ż���
				putimagePNG2(DinoX, DinoY, &DinoRun[DinoIndex]);
				���������`updateDinosaur();`
			*/
			updateDinosaur();
			updateBarrier();
			updateBloodBar();
			FlushBatchDraw();	//flush ���Ƶ��
			ScreRefresh();

			checkGameOver();
			time_t end = time(nullptr);
			gameTime = difftime(end, start);
			checkScore(gameTime);

			gameTime = checAndsetTime(gameTime);
			ReFrash = false;
		}
		//Sleep(30);
	}

	system("pause");
	return 0;
}