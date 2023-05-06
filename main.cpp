#include<stdio.h>
#include<graphics.h>
#include<conio.h>
#include<vector>	//容器加快编译速度
#include<stdbool.h>
#include <ctime>
#include<mmstream.h>
#include"tool.h"
#pragma comment(lib, "winmm.lib")
using namespace std;

#define WIN_WIDTH 576			//游戏窗口宽
#define WIN_HIGHT 324			//游戏窗口长
#define OBSTACLE_COUNT 10		//障碍物种类

IMAGE imgBgs[4];				//全局背景图片
int bgX[4];						//背景x方向坐标
int bgSpeed[3] = { 1, 2, 4};	//背景移动速度
bool gameState;

typedef struct {		//特殊状态结构体
	bool Run;
	bool Jump;
	bool Squat;
}DinoState;

int DinoX;				//恐龙 x 坐标
int DinoY;				//恐龙 y 坐标
int DinoIndex;			//动图序号(不限于run
bool ReFrash;			//刷新画面使能
int DinoBlood;
int Score;

IMAGE imgDinoRun[2];	//恐龙跑动素材
bool RunState;			//恐龙跑动状态

IMAGE imgDinoJump;		//跳跃单图
bool JumpState;			//跳跃状态
int JumMAXHight;		//高度限制
int JumpOffset;			//偏移量

bool SquatState;		//下蹲状态
IMAGE imgDinoSquat[2];	//下蹲图片

//********************被优化部分*********************
//IMAGE imgBarrier[3];//障碍物
//int BarriX;		//障碍物水平坐标
//int BarriY;		//障碍物垂直坐标
//bool BarrierExi;//障碍物存在
//***************************************************

typedef enum {
	LargeCactus,	//大仙人掌1~3
	SmallCactus,	//小仙人掌1~3
	Bird,			//鸟
	OBSTACLE_TYPE_COUNT
}obstacle_type;		//障碍物种类

//IMAGE obstacleImg[3][10];//分类调用，部分节省内存
//但是还不够，可变数组更加节省,存放所有的障碍物图片
vector<vector<IMAGE>>obstacleImages;

typedef struct obstacle{
	obstacle_type type;
	int ObstacleIndex;	//当前显示图片的序号 
	int x, y;			//location
	int speed;
	int Direct_Damage;
	int FreshSpeed;
	bool exist;
	bool hited;			//碰撞state	
	//IMAGE img[10];	//这样一个障碍物都得加载一次所有素材，浪费内存
}obstacle_t;
obstacle_t obstacles[OBSTACLE_COUNT];

void init() {
	//************************** 建游戏窗口 *****************************
	initgraph(WIN_WIDTH, WIN_HIGHT, EX_SHOWCONSOLE);	//Pere'3th用于调试
	gameState = true;

	//*********************** 加载游戏背景资源 ***************************
	char name[64];
	for (int i = 0; i < 4; i++){
		sprintf(name, "res/bg%03d.png", i + 1);
		loadimage(&imgBgs[i], name);
		bgX[i] = 0;
	}
	//*********************** 加载恐龙素材--跑动 **************************
	for (int i = 0; i < 2; i++){
		//"res/DinoRun1.png~res/DinoRun2.png"
		sprintf(name, "res/DinoRun%01d.png", i + 1);
		loadimage(&imgDinoRun[i], name);
	}
	//*********************** 加载恐龙素材--跳跃 **************************
	loadimage(&imgDinoJump, "res/DinoJump.png");
	JumMAXHight = WIN_HIGHT - imgDinoRun[0].getheight() - 10 - 100;
	JumpOffset = -6;

	//*********************** 加载恐龙素材--下蹲 **************************
	loadimage(&imgDinoSquat[0], "res/DinoSquat1.png");
	loadimage(&imgDinoSquat[1], "res/DinoSquat2.png");
	
	//************************ 恐龙初始位置参数 ***************************
	DinoX = WIN_WIDTH / 2 - imgDinoRun[0].getwidth() / 2;
	DinoY = WIN_HIGHT     - imgDinoRun[0].getheight() - 5;
	DinoIndex = 0;

	//************************* 初始化恐龙状态 ****************************
	DinoBlood = 100;

	DinoState dinoState;
	dinoState = {true, false, false};
	RunState = dinoState.Run;
	JumpState = dinoState.Jump;	
	SquatState = dinoState.Squat;


	/* 被优化部分
	//for (int i = 0; i < 3; i++) {	//加载障碍物barrier素材
	//	//"res/SmallCactus1.png~res/SmallCactus3.png"
	//	sprintf(name, "res/SmallCactus%01d.png", i + 1);
	//	loadimage(&imgBarrier[i], name);
	//}
	////设置障碍物
	//BarrierExi = false;
	//BarriX = WIN_WIDTH *2 ;
	//BarriY = 240 ;
	*/

	//*************************** 加载障碍物 *****************************
	//1.加载小仙人掌
	vector<IMAGE> imgSmallCactusArrary;
	IMAGE imgCactus;
	for (int i = 0; i < 3; i++) {
		sprintf(name, "res/SmallCactus%01d.png", i + 1);
		loadimage(&imgCactus, name);
		imgSmallCactusArrary.push_back(imgCactus);
	}
	//2.加载大仙人掌
	vector<IMAGE> imgLargeCactusArrary;
	for (int i = 0; i < 3; i++) {
		sprintf(name, "res/LargeCactus%01d.png", i + 1);
		loadimage(&imgCactus, name);
		imgLargeCactusArrary.push_back(imgCactus);
	}
	//3.加载鸟
	vector<IMAGE> imgBirdArrary;
	IMAGE imgBird;
	for (int i = 0; i < 2; i++) {
		sprintf(name, "res/Bird%01d.png", i + 1);
		loadimage(&imgBird, name);
		imgBirdArrary.push_back(imgBird);
	}
	//4.装载所有类型的障碍物
	obstacleImages.push_back(imgSmallCactusArrary);	//小仙人掌
	obstacleImages.push_back(imgLargeCactusArrary);	//大仙人掌
	obstacleImages.push_back(imgBirdArrary);			//鸟
	//5.初始化障碍物池
	for (int i = 0; i < OBSTACLE_COUNT; i++){
		obstacles[i].exist = false;
	}
	//6.障碍物"运动的"刷新速率
	obstacles[0].FreshSpeed = 30;//SmallCactus
	obstacles[1].FreshSpeed = 30;//LargeCactus
	obstacles[2].FreshSpeed = 10;//bird

	//************************ 设置开始界面 ****************************
	ReFrash = true;
	loadimage(0, "res/over.png");
	FlushBatchDraw(); 
	system("pause");
	//*************************** 预加载音效 ***************************
	preLoadSound("res/newhit.wav");
	mciSendString("play res/bg.mp3", 0, 0, 0);
	//*************************** 初始化分数 ***************************
	Score = 0;
}

//创建障碍物
void creatObstacle() {
	int i;
	for (i = 0; i < OBSTACLE_COUNT; i++) {
		if (obstacles[i].exist == false) {
			break;	//看看哪种需要创建
		}
	}
	if (i >= OBSTACLE_COUNT) {
		return;		//没找到就返回
	}
	obstacles[i].exist = true;
	obstacles[i].hited = false;
	obstacles[i].ObstacleIndex = 0;
	obstacles[i].type = (obstacle_type)(rand() % OBSTACLE_TYPE_COUNT);
	obstacles[i].x = WIN_WIDTH;
	//自定义障碍物速度
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
			int a1x, a1y, a2x, a2y;		//恐龙参考点
			int b1x, b1y, b2x, b2y;		//障碍物参考点
			int off = 30;				//缩小检测范围 模拟刚体
			bool* p = (bool *) &dinoS;	//类型统一
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
				printf("血量剩余 %d\n", DinoBlood);
				playSound("res/newhit.wav");
				obstacles[i].hited = true;
			}
		}
	}
}

//画面刷新
void ScreRefresh() {
	//背景滑动
	for (int i = 0; i < 3; i++) {
		bgX[i] -= bgSpeed[i];
		if (bgX[i] < -WIN_WIDTH) {
			bgX[i] = 0;
		}
	}
	//恐龙动作控制逻辑
	if (JumpState){
		//恐龙跳跃
		if (DinoY < JumMAXHight){
			//下移
			JumpOffset = 6;
		}
		DinoY += JumpOffset;

		if (DinoY > WIN_HIGHT - imgDinoRun[0].getheight() - 10){
			//到初始底部停止跳跃 && 重置偏移量
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
		//默认跑动状态
		static	int count = 0;
		count++;
		if (count >= 4) {
			count = 0;
			DinoIndex = (DinoIndex + 1) % 2;
		}
	}

	//创建障碍物
	static int frameCount = 0;	//n次调用刷新页面-ScreRefresh 才创建
	static int BarriFre = 50;
	frameCount++;
	if (frameCount > BarriFre){
		frameCount = 0;
		BarriFre = 50 + rand() % 50;//随机出现,50~99Fre
		//********************被优化部分*********************
		//if (!BarrierExi){
		//	BarrierExi = true;
		//	BarriX = WIN_WIDTH;
		//	BarriFre = 200 + rand()%300;//随机出现
		//}
		//***************************************************
		creatObstacle();
	}
	//********************被优化部分*********************
	//barrier run
	/*if (BarrierExi){
		BarriX -= Speed[3];
		if (BarriX < -imgBarrier->getwidth()){
			BarrierExi = false;
		}
	}*/
	//***************************************************
	
	//更新所有障碍物的坐标
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
	//碰撞检测
	checkHit();
}

//跳跃启动开关
void jump() {
	JumpState = true;
	ReFrash = true;		//需要立即刷新
}
//下蹲启动开关
void Squat() {
	SquatState = true;
	ReFrash = true;		//需要立即刷新
	DinoIndex = 0;		//开始下蹲设为0
}

//处理用户按键
void keyEvent() {
	//如何在避免阻塞的情况下进行输入检测
	char ch;
	if (_kbhit()){	//有按键按下 返回 true
		ch = _getch();	//无需按 enter 读取一个字符
		if (ch == 'w') {
			jump();
		}
		else if (ch == 's') {
			Squat();
		}
	}
}
// 渲染游戏背景
void updateBg() {
	putimagePNG2(bgX[0], 0, &imgBgs[0]);	//背景001 在x=bgX[0]处
	putimagePNG2(bgX[1], 0, &imgBgs[1]);
	putimagePNG2(bgX[2], 0, &imgBgs[2]);

	putimagePNG2(bgX[0] + 576, 0, &imgBgs[0]);
	putimagePNG2(bgX[1] + 576, 0, &imgBgs[1]);
	putimagePNG2(bgX[2] + 576, 0, &imgBgs[2]);
}

//渲染障碍物
void updateBarrier() {
	/*被优化部分
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

//渲染恐龙
void updateDinosaur() {
	if (!SquatState){
		putimagePNG2(DinoX, DinoY, &imgDinoRun[DinoIndex]);
	}else{
		//下蹲更新时：屏幕高度-当前图像的高度
		int y = WIN_HIGHT - (imgDinoSquat[DinoIndex]).getheight();
		putimagePNG2(DinoX, y, &imgDinoSquat[DinoIndex]);
	}
	
}

//渲染血条
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

		//暂停之后，复活 or 重来
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
	static int gameTime = 0;		//计时积分
	time_t start = time(nullptr);	//计时积分
	int timer = 0;
	while (1){
		//keyEvent && timer 双唤醒
		keyEvent();

		timer += getDelay();
		if (timer > 30)	{		//30ms 刷新
			timer = 0;
			ReFrash = true;
		}
		if (ReFrash){
			BeginBatchDraw();	//缓存 解决频闪
			updateBg();
			/*4.19.10:47优化：
				putimagePNG2(DinoX, DinoY, &DinoRun[DinoIndex]);
				解决方案：`updateDinosaur();`
			*/
			updateDinosaur();
			updateBarrier();
			updateBloodBar();
			FlushBatchDraw();	//flush 解决频闪
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