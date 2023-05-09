#include<stdio.h>
#include<graphics.h>
#include<conio.h>
#include<vector>	//Speed up compilation
#include<stdbool.h>
#include <ctime>
#include<mmstream.h>
#include"tool.h"
#pragma comment(lib, "winmm.lib")
using namespace std;

#define WIN_SCORE 10
#define WIN_WIDTH 576			//Game window width
#define WIN_HIGHT 324			//Game window length
#define OBSTACLE_COUNT 10		//Type of obstacle

bool gameState;
IMAGE imgBgs[4];				//Global background image
int bgX[4];						//Background x coordinates
int bgSpeed[4] = { 1, 2, 4, 5};	//Background velocity


typedef struct {		//Dinos' Special state
	bool Run;
	bool Jump;
	bool Squat;
}DinoState;

int DinoX;				//Dinosaur x coordinate
int DinoY;				//Dinosaur y coordinate
int DinoIndex;			//Image sequence number (not limited to run
bool ReFrash;			//screen refresh function is enabled
int DinoBlood;
int Score;

IMAGE imgDinoRun[2];	//run material
bool RunState;			//running state

IMAGE imgDinoJump;		//Jump imge
bool JumpState;			//Jumping state
int JumMAXHight;		//Height restriction
int JumpOffset;			//offset

IMAGE imgDinoSquat[2];	//Squat imge
bool SquatState;		//squat state

typedef enum {
	LargeCactus,	//¥Ûœ…»À’∆1~3
	SmallCactus,	//–°œ…»À’∆1~3
	Bird,			//ƒÒ
	OBSTACLE_TYPE_COUNT
}obstacle_type;		//Type of obstacle


vector<vector<IMAGE>>obstacleImages;//two-dimensional mutable array to store obstacles

//Obstacle attribute
typedef struct obstacle{
	obstacle_type type;
	int ObstacleIndex;	//Obstacle picture number
	int x, y;			//location
	int speed;
	int Direct_Damage;
	int FreshSpeed;
	bool exist;
	bool hited;			//hit state
	bool passed;
}obstacle_t;
obstacle_t obstacles[OBSTACLE_COUNT]; 

IMAGE imgSZ[10];


//called by main(): Initial load
void init() {
	//Build game window
	initgraph(WIN_WIDTH, WIN_HIGHT);	//option: EX_SHOWCONSOLE
	gameState = true;

	//Load game background resources
	char name[64];
	for (int i = 0; i < 4; i++){
		sprintf(name, "res/img/bg%03d.png", i + 1);
		loadimage(&imgBgs[i], name);
		bgX[i] = 0;
	}
	//Loading Dinosaur material - Run
	for (int i = 0; i < 2; i++){
		sprintf(name, "res/img/DinoRun%01d.png", i + 1);
		loadimage(&imgDinoRun[i], name);
	}
	//Loading Dinosaur material - Jump
	loadimage(&imgDinoJump, "res/img/DinoJump.png");
	JumMAXHight = WIN_HIGHT - imgDinoRun[0].getheight() - 10 - 100;
	JumpOffset = -6;

	//Loading Dinosaur material - Squat
	loadimage(&imgDinoSquat[0], "res/img/DinoSquat1.png");
	loadimage(&imgDinoSquat[1], "res/img/DinoSquat2.png");
	
	//Dinosaur initial position parameters
	DinoX = WIN_WIDTH / 2 - imgDinoRun[0].getwidth() / 2;
	DinoY = WIN_HIGHT     - imgDinoRun[0].getheight() - 15;
	DinoIndex = 0;

	//Initialize the dinosaur state
	DinoBlood = 100;

	DinoState dinoState;
	dinoState = {true, false, false};
	RunState = dinoState.Run;
	JumpState = dinoState.Jump;	
	SquatState = dinoState.Squat;

	//1.Load obstacles - Small cactus
	vector<IMAGE> imgSmallCactusArrary;
	IMAGE imgCactus;
	for (int i = 0; i < 3; i++) {
		sprintf(name, "res/img/SmallCactus%01d.png", i + 1);
		loadimage(&imgCactus, name);
		imgSmallCactusArrary.push_back(imgCactus);
	}
	//2.Load obstacles - Big cactus
	vector<IMAGE> imgLargeCactusArrary;
	for (int i = 0; i < 3; i++) {
		sprintf(name, "res/img/LargeCactus%01d.png", i + 1);
		loadimage(&imgCactus, name);
		imgLargeCactusArrary.push_back(imgCactus);
	}
	//3.Load obstacles - bird
	vector<IMAGE> imgBirdArrary;
	IMAGE imgBird;
	for (int i = 0; i < 2; i++) {
		sprintf(name, "res/img/Bird%01d.png", i + 1);
		loadimage(&imgBird, name);
		imgBirdArrary.push_back(imgBird);
	}
	//4.Load all types of obstructions
	obstacleImages.push_back(imgSmallCactusArrary);
	obstacleImages.push_back(imgLargeCactusArrary);
	obstacleImages.push_back(imgBirdArrary);

	//5.Example Initialize the obstacle pool
	for (int i = 0; i < OBSTACLE_COUNT; i++){
		obstacles[i].exist = false;
	}
	//6.Refresh rate of obstacle "motion"
	obstacles[0].FreshSpeed = 30;	//SmallCactus
	obstacles[1].FreshSpeed = 30;	//LargeCactus
	obstacles[2].FreshSpeed = 10;	//bird

	//Set the start screen
	mciSendString("play res/music/startM.mp3", 0, 0, 0); // play res / bg.mp3
	ReFrash = true;
	loadimage(0, "res/img/start2.png");
	FlushBatchDraw(); 
	system("pause");
	mciSendString("play res/music/bg.mp3", 0, 0, 0); // play res / bg.mp3
	//Preloaded sound
	preLoadSound("res/music/newhit.wav");
	
	//Initialization score
	Score = 0;

	//load score img
	for (int  i = 0; i < 10; i++){
		sprintf(name, "res/score/%d.png", i);
		loadimage(&imgSZ[i], name);
	}
}

/*
	called by ScreRefresh(): 
	Creates an obstacle after refreshing a certain number of frames
*/
void creatObstacle() {
	int i;	//Obstruction i
	for (i = 0; i < OBSTACLE_COUNT; i++) {
		if (obstacles[i].exist == false){
			break;
		}
	}
	if (i >= OBSTACLE_COUNT) {
		return;
	}
	
	//Set the public attributes of obstacles
	obstacles[i].exist = true;
	obstacles[i].hited = false;
	obstacles[i].ObstacleIndex = 0;
	obstacles[i].type = (obstacle_type)(rand() % OBSTACLE_TYPE_COUNT);
	obstacles[i].x = WIN_WIDTH;

	//Customize obstacle properties: speed, damage, movement, etc
	if (obstacles[i].type == LargeCactus) {
		obstacles[i].speed = 1;
		obstacles[i].Direct_Damage = 15;
		obstacles[i].y = 322 - (obstacleImages[LargeCactus][0]).getheight();
	}
	else if (obstacles[i].type == SmallCactus) {
		obstacles[i].speed = 1;
		obstacles[i].Direct_Damage = 10;
		obstacles[i].y = 312 - (obstacleImages[SmallCactus][0]).getheight();
	}
	else if (obstacles[i].type == Bird) {
		obstacles[i].speed = 6;
		obstacles[i].Direct_Damage = 20;
		obstacles[i].y = 260 - (obstacleImages[Bird][0]).getheight();
	}

	obstacles[i].passed = false;
}


//called by ScreRefresh(): Collision detection
void checkHit() {
	for (int i = 0; i < OBSTACLE_COUNT; i++){
		DinoState dinoS = { RunState, JumpState, SquatState };
		if (obstacles[i].exist&& obstacles[i].hited == false) {
			int a1x, a1y, a2x, a2y;		//Dinosaur reference point
			int b1x, b1y, b2x, b2y;		//Obstacle reference point
			int off = 30;				//Narrow the detection range
			bool* p = (bool *) &dinoS;	//Type unity
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
				printf("Surplus blood volume %d\n", DinoBlood);
				playSound("res/music/newhit.wav");
				obstacles[i].hited = true;
			}
		}
	}
}


//called by main(): Refresh the image after the rendering task is complete
void ScreRefresh() {
	//Background slide
	for (int i = 0; i < 4; i++) {
		bgX[i] -= bgSpeed[i];
		if (bgX[i] < -WIN_WIDTH) {
			bgX[i] = 0;
		}
	}
	//Dinosaur motion control logic
	if (JumpState){
		//Dinosaur jump
		if (DinoY < JumMAXHight){
			//Move down
			JumpOffset = 6;
		}
		DinoY += JumpOffset;

		if (DinoY > WIN_HIGHT - imgDinoRun[0].getheight() - 10){
			//Stop jump at bottom && reset offset
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
		//Default running state
		static	int count = 0;
		count++;
		if (count >= 4) {
			count = 0;
			DinoIndex = (DinoIndex + 1) % 2;
		}
	}

	//Create obstacles
	static int frameCount = 0;	//n calls to refresh the page -ScreRefresh is created
	static int BarriFre = 50;
	frameCount++;
	if (frameCount > BarriFre){
		frameCount = 0;
		BarriFre = 50 + rand() % 50;//Random occurrence, 50~99Fre
		creatObstacle();
	}
	
	//Update the coordinates of all obstacles
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
				if (obstacles[i].type == Bird)
				{
					obstacles[i].ObstacleIndex = (obstacles[i].ObstacleIndex + 1) % len;
				}
			}
		}
	}
	freshCount++;
	checkHit();		//Collision detection
}


//called by keyEvent(): Detect keyEvent for dinosaur status updates
void jump() {
	JumpState = true;
	ReFrash = true;		//Need to refresh immediately
}

//called by keyEvent(): Detect keyEvent for dinosaur status updates
void Squat() {
	SquatState = true;
	ReFrash = true;		//Need to refresh immediately
	DinoIndex = 0;		//Start squatting to play the 0th picture of the posture
}


//called by main(): Detects the keyEvent
void keyEvent() {
	char ch;
	if (_kbhit()){		//There is a key pressed to return true
		ch = _getch();	//no press enter to read a character
		if (ch == 'w') {
			jump();
		}
		else if (ch == 's') {
			Squat();
		}
	}
}


//called by main(): Render background movement
void updateBg() {
	putimagePNG2(bgX[0], 0, &imgBgs[0]);	//Background 001 is at x=bgX[0]
	putimagePNG2(bgX[1], 0, &imgBgs[1]);
	putimagePNG2(bgX[2], 0, &imgBgs[2]);
	putimagePNG2(bgX[3], 380, &imgBgs[3]);

	putimagePNG2(bgX[0] + 576, 0, &imgBgs[0]);
	putimagePNG2(bgX[1] + 576, 0, &imgBgs[1]);
	putimagePNG2(bgX[2] + 576, 0, &imgBgs[2]);
	putimagePNG2(bgX[3] + 576, 275, &imgBgs[3]);
}


//called by main(): Render obstacle
void updateBarrier() {
	for (int i = 0; i < OBSTACLE_COUNT; i++){
		if (obstacles[i].type != Bird && obstacles[i].exist)		{
			putimagePNG2(obstacles[i].x, obstacles[i].y, WIN_WIDTH,
				&obstacleImages[obstacles[i].type][obstacles[i].ObstacleIndex]);
		}
		else if (obstacles[i].type == Bird && obstacles[i].exist) {
			putimagePNG2(obstacles[i].x, obstacles[i].y, WIN_WIDTH, 
				&obstacleImages[obstacles[i].type][obstacles[i].ObstacleIndex]);
		}
	}
}


//called by main(): Render dinosaur
void updateDinosaur() {
	if (!SquatState){
		putimagePNG2(DinoX, DinoY, &imgDinoRun[DinoIndex]);
	}else{
		//When squatting update: Screen height - The height of the current image
		int y = WIN_HIGHT - (imgDinoSquat[DinoIndex]).getheight();
		putimagePNG2(DinoX, y, &imgDinoSquat[DinoIndex]);
	}
	
}


//called by main(): Render blood bar by API
void updateBloodBar() {
	drawBloodBar(10, 10, 200, 10, 2, BLUE, DARKGRAY, RED, DinoBlood / 100.0);
}


//called by main()
void checkGameOver() {
	if (DinoBlood <= 0) {
		mciSendString("stop res/music/bg.mp3", 0, 0, 0);
		mciSendString("stop res/music/false.mp3", 0, 0, 0);
		Sleep(3);
		mciSendString("play res/music/lose.wav", 0, 0, 0);
		int offset = 5;
		IMAGE img;
		loadimage(&img, "res/img/false1.png");
		for (int i = 0; i < 250; i++){
			putimagePNG2(10 + offset, 5 + offset, &img);
			FlushBatchDraw();
			offset = -offset;
			Sleep(50);
		}
		system("pause");

		//After a pause, revive or restart
		DinoBlood = 100;
		mciSendString("stop res/music/lose.wav", 0, 0, 0);

		mciSendString("play res/music/bg.mp3", 0, 0, 0);
		
		gameState = false;
		Score = 0;
	}
}


//called by main()
void checkScore() {
	for (int i = 0; i < OBSTACLE_COUNT; i++) {
		if (obstacles[i].exist &&
			obstacles[i].passed == false &&
			obstacles[i].hited == false &&
			obstacles[i].x + obstacleImages[obstacles[i].type][0].getwidth() < DinoX) {
			Score++;
			obstacles[i].passed = true;
			printf("score is %d\n", Score);
		}

	}
}

void updateScore() {
	char str[8];
	sprintf(str, "%d", Score);

	int x = 230;
	int y = 2;

	for (int i = 0; str[i]; i++) {
		int sz = str[i] - '0';
		putimagePNG2(x, y, &imgSZ[sz]);
		x += imgSZ[sz].getwidth() + 5;
	}
}

void checkWin() {
	FlushBatchDraw();
	if (Score >= WIN_SCORE){
		mciSendString("play res/music/¬ÛøÀ∞¢…™.mp3", 0, 0, 0); // play res / bg.mp3
		Sleep(2000);
		loadimage(0, "res/img/win.png");
		FlushBatchDraw();
		mciSendString("stop res/music/bg.mp3", 0, 0, 0);
		mciSendString("play res/music/win2.mp3", 0, 0, 0);
		system("pause");
		mciSendString("play res/music/bg.mp3", 0, 0, 0);
		DinoBlood = 100;
		Score = 0;
	}
}

int main(void) {
	init();

	int timer = 0;
	while (1){
		//keyEvent && timer Double wake up
		keyEvent();

		timer += getDelay();
		if (timer > 30)	{		//30ms refresh
			timer = 0;
			ReFrash = true;
		}
		if (ReFrash){
			ScreRefresh();
			BeginBatchDraw();	//Cache, stroboscopic resolution
			updateBg();
			updateDinosaur();
			updateBarrier();
			updateBloodBar();
			updateScore();
			checkWin();
			FlushBatchDraw();	//flush Stroboscopic resolution

			checkGameOver();
			checkScore();

			ReFrash = false;
		}
		//Sleep(30);
	}

	system("pause");
	return 0;
}