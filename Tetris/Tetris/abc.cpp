
#include <conio.h>
#include <stdlib.h>     //随机生成方块
#include <windows.h>     //使用API函数
#include <time.h>    //使用定时器
#include "colorConsole.h" 
#include<mmsystem.h>
#include<cstdlib>
#pragma comment(lib,"winmm.lib")
using namespace std;



//#define KEY_UP      72
//#define KEY_DOWN    80
//#define KEY_LEFT    75
//#define KEY_RIGHT   77
#define KEY_ESC     27
#define MAPW    12     //地图的宽度
#define MAPH    20     //地图的高度
CRITICAL_SECTION csK;  //定义一个临界区变量，控制键盘读写


//7种不同形状的方块
int b[7][4][4]={	{{1},{1,1,1}},
					{{0,2},{2,2,2}},
					{{3,3},{0,3,3}},
					{{0,0,4},{4,4,4}},
					{{0,5,5},{5,5}},
                    {{6,6,6,6}},
                    {{7,7},{7,7}}
                };

WORD SQUARE_COLOR[7] ={ FOREGROUND_RED|FOREGROUND_INTENSITY,
                        FOREGROUND_GREEN|FOREGROUND_INTENSITY,
						FOREGROUND_BLUE|FOREGROUND_INTENSITY,
						FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY,
						FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY,
						FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY,
						FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY
					 };		

class Tetris
{
private:
	HANDLE handle;
	int map[MAPH][MAPW] ; //保存工作区的区域
	int dx ; //初始化屏幕时起始坐标
	int dy ;
	const int KEY_UP ,KEY_DOWN,KEY_LEFT,KEY_RIGHT;

public:
	int key;

	Tetris(int a,int b, int c, int d)
		:KEY_UP(a),KEY_DOWN(b),KEY_LEFT(c),KEY_RIGHT(d){}

	void Init(int x, int y, HANDLE handle);  //初始化工作			
	void Turn(int a[][4],int w,int h,int *x,int y);    //方块转动
	bool IsAvailable(int a[],int x,int y,int w,int h);  //判定是否能放下
	void DrawBlocks(int a[],int w,int h,int x,int y,WORD wColors[],int nColors);   //显示方块和边界
	void ClearSquare(int a[],int w,int h,int x,int y);  //清除方块
	void GameOver();                         //游戏结束
	void DeleteLine(int m[][MAPW],int row);  //消除一行

	static DWORD WINAPI run(LPVOID obj);

};

void main()
{
	HANDLE handle=initiate();
	PlaySound("C:\\Users\\Administrator\\Desktop\\Tetris\\Burning.wav ", NULL, SND_FILENAME | SND_ASYNC|SND_LOOP);     //播放背景音乐


	Tetris t1('w','s','a','d'),t2(72,80,75,77);
	t1.Init(4,3,handle);
	t2.Init(40,3,handle);
	InitializeCriticalSection(&csK);
	DWORD dwThreadID;
	CreateThread(NULL,0,Tetris::run,&t1,NULL,&dwThreadID);
	CreateThread(NULL,0,Tetris::run,&t2,NULL,&dwThreadID);    //创建一个新线程，实现双人对战

	while(1)
	{
		if (_kbhit)
		{
			int key = getch();
			switch(key)
			{
			case 72:
			case 80:
			case 75:
			case 77:
			case 27:
				EnterCriticalSection(&csK);
				t2.key = key;
				LeaveCriticalSection(&csK);
				break;
			case 'a':
			case 'd':
			case 's':
			case 'w':
				EnterCriticalSection(&csK);
				t1.key = key;
				LeaveCriticalSection(&csK);
				break;
			};
		}
		Sleep(10);
	}
	DeleteCriticalSection(&csK);
}

DWORD WINAPI Tetris::run(LPVOID obj)
{
	Tetris *pt = (Tetris*)obj;

	srand(time(NULL)+pt->dx);

	int score=0;  //初始化分数
	int level=0;  //初始化游戏级别

	int Num = rand() %7 ; //创建第一个方块编号
	int nextNum = Num;    //保存下一个方块编号

	int blank;  //记录每个方块起始位置
	int x = 0,y = 0;  //记录游戏开始的相对坐标

	int a[4][4] = {0}; //临时使用用来保存当前方块


	while(1)
	{
		for(int i=0;i<4;i++)          //复制方块
			for(int j=0;j<4;j++)
				if(a[i][j] = b[nextNum][i][j]) 
					blank=i;
				
		y = 1- blank;
		x = 4;

		//创建下一个方块
		Num = nextNum;
		pt->ClearSquare(b[Num][0],4,4,13,13);
		nextNum = rand() % 7 ;
		WORD wColors[1]= { SQUARE_COLOR[nextNum] };
		pt->DrawBlocks(b[nextNum][0],4,4,13,13,wColors,1);
	
		wColors[0] = SQUARE_COLOR[Num] ;
		pt->DrawBlocks(&a[0][0],4,4,x,y,wColors,1);

		//显示分数信息
		char string[5];
		wColors[0]=FOREGROUND_RED| FOREGROUND_GREEN|FOREGROUND_INTENSITY;
		textout(pt->handle,26+pt->dx,5+pt->dy,wColors,1,itoa(score,string,10));
		textout(pt->handle,26+pt->dx,9+pt->dy,wColors,1,itoa(level,string,10));
		
		int max_delay = 100 - 10 * level; //计算不同游戏级别的下落时间间隔

		while(1)
		{
			int delay=0; //延迟量
			while(delay<max_delay)
			{
				EnterCriticalSection(&csK);
				int key = pt->key;
				LeaveCriticalSection(&csK);
				//if(_kbhit())  //用if避免按住键使方块卡住
				{
					//int key=_getch();
					if (key==pt->KEY_UP)
					{
					
							pt->ClearSquare(&a[0][0],4,4,x,y);
							pt->Turn(a,4,4,&x,y);
							wColors[0]=SQUARE_COLOR[Num];
							pt->DrawBlocks(&a[0][0],4,4,x,y,wColors,1);
					}
					else if (key==pt->KEY_DOWN)
					{
						delay=max_delay;
					}
					else if (key== pt->KEY_LEFT)
						{
							if(pt->IsAvailable(&a[0][0],x-1,y,4,4))
							{
								pt->ClearSquare(&a[0][0],4,4,x,y);
								x--;
								wColors[0]=SQUARE_COLOR[Num];
								pt->DrawBlocks(&a[0][0],4,4,x,y,wColors,1);
							}	
						}
				
					else if (key==pt->KEY_RIGHT)
						{
							if(pt->IsAvailable(&a[0][0],x+1,y,4,4))
							{
								pt->ClearSquare(&a[0][0],4,4,x,y);
								x++;
								wColors[0]=SQUARE_COLOR[Num];
								pt->DrawBlocks(&a[0][0],4,4,x,y,wColors,1);
							}
						}
						
					else if (key== KEY_ESC)
					{
						exit(EXIT_SUCCESS);
					}
					pt->key = 0;
					key = 0;
				}
				_sleep(8);delay++;
			}
			if(pt->IsAvailable(&a[0][0],x,y+1,4,4)) //是否能下移
			{
				pt->ClearSquare(&a[0][0],4,4,x,y);
				y++;
				wColors[0]=SQUARE_COLOR[Num];
				pt->DrawBlocks(&a[0][0],4,4,x,y,wColors,1);
			}
			else
			{
				if(y<=1) pt->GameOver();     //是否结束
				for(int i=0;i<4;i++)     //放下方块，更新工作区
					for(int j=0;j<4;j++)
						if(a[i][j]&&((i+y)<MAPH-1)&&((j+x)<MAPW-1)) 
							pt->map[i+y][j+x]=a[i][j];

				int full,k=0;
				for(int i=y;i<min(y+4,MAPH-1);i++)
				{
					full=1;
					for(int j=1;j<11;j++)
						if(!pt->map[i][j]) full=0;
					if(full)   //消掉一行
					{
						pt->DeleteLine(pt->map,i);
						k++;
						score=score+k;
						level=min(score/30,9);
						max_delay=100-10*level;
					}
				}
				break; 
			}
		}
	}
	return EXIT_SUCCESS;
}

void Tetris::Init(int x, int y, HANDLE handle)   //初始化函数
{
	dx = x;
	dy = y;
	this->handle = handle;

	memset(map,0,sizeof(map));
	
	//初始化工作区
	for(int i=0;i<20;i++)
	{
		map[i][0]=-2;              //左右边界
		map[i][11]=-2;
	}
	for(int i=0;i<12;i++)
	{
		map[0][i]=-1;              //上下边界
		map[19][i]=-1;
	}
	map[0][0]=-3;                  //四个角
	map[0][11]=-3;
	map[19][0]=-3;
	map[19][11]=-3;


	WORD wColors[1]={FOREGROUND_RED| FOREGROUND_GREEN|FOREGROUND_INTENSITY};
	textout(handle,26+dx,3+dy,wColors,1,"SCORE");
	textout(handle,26+dx,7+dy,wColors,1,"LEVEL");
	textout(handle,26+dx,11+dy,wColors,1,"NEXT");

	wColors[0]=FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY;
	DrawBlocks(&map[0][0],12,20,0,0,wColors,1);
	textout(handle,dx,dy,wColors,1,"◇══════════◇");

	wColors[0]=FOREGROUND_RED| FOREGROUND_GREEN|FOREGROUND_INTENSITY;
	//textout(handle,dx-16,dy,wColors,1,"按任意键开始");

	//int ch=_getch();
	//textout(handle,dx-16,dy,wColors,1,"              ");

}

bool Tetris::IsAvailable(int a[],int x,int y,int w,int h)    //判断方块可否移动
{
	for(int i=y;i<y+h;i++)   //x,y表示方块在下一工作区的位置，w,h表示二维数组的宽和高
		for(int j=x;j<x+w;j++)
			if(map[i][j] && a[w*(i-y)+j-x])
				return 0;     //不能移动
	return 1;    //可以移动
}

void Tetris::DrawBlocks(int a[],int w,int h,int x,int y,WORD wColors[],int nColors)  //显示方块算法
{
	int temp;    //x,y表示方块左上角距离工作区左上角的相对位置
	
	for(int i=0;i<h;i++)
		for(int j=0;j<w;j++)
			if((temp=a[i*w+j])&&y+i>0)
			{
				if(temp==-3)
					textout(handle,2*(x+j)+dx,y+i+dy,wColors,nColors,"◆");
				else if(temp==-2)
					textout(handle,2*(x+j)+dx,y+i+dy,wColors,nColors,"║");
				else if(temp==-1)
					textout(handle,2*(x+j)+dx,y+i+dy,wColors,nColors,"═");
				else if(temp>=1)
					textout(handle,2*(x+j)+dx,y+i+dy,wColors,nColors,"■");
			}
}

void Tetris::ClearSquare(int a[], int w, int h, int x, int y)
{
	WORD wColors[1] = { 0 };
	for (int i = 0; i<h; i++)
		for (int j = 0; j<w; j++)
		{
			if (a[i*w + j]>0 && (i + y>0))
			{
				textout(handle, 2 * (x + j) + dx, y + i + dy, wColors, 1, "  ");
			}
		}

}

void Tetris::Turn(int a[][4],int w,int h,int *x,int y)
{
	int b[4][4]={{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
	int sign=0,line=0;
	for(int i=h-1;i>=0;i--)
	{
		for(int j=0;j<w;j++)
			if(a[i][j])
			{
				b[j][line]=a[i][j];
				sign=1;
			}
		if(sign) 
		{
			line++;
			sign=0;
		}
	}
	for(int i=0;i<4;i++)
		if(IsAvailable(b[0],*x-i,y,w,h))
		{
			*x-=i;
			for(int k=0;k<h;k++)
				for(int j=0;j<w;j++)
					a[k][j]=b[k][j];
			break;
		}
}

void Tetris::GameOver()
{
	WORD wColors[1]={FOREGROUND_RED| FOREGROUND_GREEN|FOREGROUND_INTENSITY};
	textout(handle,7+dx,10+dy,wColors,1,"GAME OVER");

	exit(EXIT_SUCCESS);
}

void Tetris::DeleteLine(int m[][MAPW],int row)
{
	WORD wColors[1]={FOREGROUND_RED| FOREGROUND_GREEN|FOREGROUND_INTENSITY};
	textout(handle,2+dx,row+dy,wColors,1,"﹌﹌﹌﹌﹌﹌﹌﹌﹌﹌");
	_sleep(100);

	for(int i=row;i>1;i--)
	{
		ClearSquare(&m[i][1],MAPW-2,1,1,i);
		for(int j=1;j<MAPW-1;j++)
		{
			m[i][j]=m[i-1][j];
			if (m[i][j] == 0)
				wColors[0] = 0;
			else
				wColors[0] = SQUARE_COLOR[m[i][j]-1];
			DrawBlocks(&m[i][j],1,1,j,i,wColors,1);
		}
	}
	for(int i=1;i<MAPW-1;i++)
		m[1][i]=0;
}