#include "fileOp.h"
#include <winsock2.h>
#include <windows.h>
#include <strstream>
#include <pthread.h>
#include <atomic>
using namespace std;
#define USER_DIR "./users.dat"
#define QUIZ_DIR "./quiz.dat"

#define MAX_LINK_NUM 100

#define CHALLENGE_MULTI_RATE 1.5

#define RANK_BORAD_LENGTH 10

#define SHOW_TIME 1000

enum {
	LEVEL,
	EXP,
	COURSE,
	QUIZ
};
enum {
	TOP,
		LOGIN,
		SIGN_UP,
			PLAYER_MAIN,
				PLAY_GAME,
			AUTHOR_MAIN,
				NEW_WORD,
			SCORE_BOARD,
				RANK_PLAYER,
				RANK_AUTHOR,
			LOGOUT,
			INFO,
			ONLINE_LIST,
		DUEL,
		DUEL_GAIN,
			DUEL_GAME,
		EXIT
};

//挑战相关
Course challengeCourse[100];
int duelRequest[MAX_LINK_NUM];
pthread_cond_t *duelCond[MAX_LINK_NUM];
int layerBackup[MAX_LINK_NUM] = {};
long timeCost[MAX_LINK_NUM] = {};
unordered_map<pthread_cond_t *, bool> duelAvaliable;

//主程序相关
int lag[MAX_LINK_NUM] = {};
int playerSortTag[MAX_LINK_NUM] = {};
int authorSortTag[MAX_LINK_NUM] = {};
SOCKET sockList[MAX_LINK_NUM] = {};
int layer[MAX_LINK_NUM] = {};
User *curUser[MAX_LINK_NUM];

//数据存储
vector<User*> userList;
vector<Quiz> quizList;

//线程号到整形的映射
unordered_map<pthread_t, int> thread2int;

//各种锁
atomic<bool> exitFlag;
pthread_mutex_t userInfoLock;
pthread_mutex_t wordBaseLock;
pthread_mutex_t challengeLock;

HANDLE gameBlock1[MAX_LINK_NUM];
HANDLE gameBlock2[MAX_LINK_NUM];

void lagTest(){
	int fd = thread2int[pthread_self()];
	SOCKET srvSock = sockList[fd];
	int startTime = time(NULL);
	char cmd[100] = "t";
	char ack[20];
	for(int i = 0; i < 5; i++){
		memset(ack, 0 ,sizeof(char));
		int state = send(srvSock, cmd, sizeof(cmd), 0);
		if(state < 0){
			cout << "net fail\n";
			return;
		}
		recv(srvSock, ack, sizeof(char), 0);
	}
	startTime = time(NULL) - startTime;
	lag[fd] = startTime / 5;
	return;
}
void process_exit();
int trans(char type, string &data = *(new string(""))){
	int fd = thread2int[pthread_self()];
	SOCKET srvSock = sockList[fd];
	char *cmd = (char *)malloc(sizeof(char));
	*cmd = type;
	cout << "Send "<< fd << " cmd:" << *cmd << "\n";
	int state = send(srvSock, cmd, sizeof(char), 0);
	if(state < 0){
		cout << fd << ": Net fail in sending cmd\n";
		return -1;
	}
	char op[100000];
	if(type == 'g'){
		memset(op, 0, sizeof(op));
		state = recv(srvSock, op, sizeof(op), 0);
		if(state < 0){
			cout << fd << ": Net fail in 1st OK\n";
			return -1;
		}
		cout << "Recived "<< fd << " Message:" << op << "\n";
		data = string(op);
		return 0;
	}
	else{
		memset(op, 0, sizeof(op));
		state = recv(srvSock, op, sizeof(op), 0);
		if(state < 0){
			cout << fd << ": Net fail in 1st shake\n";
			return -1;
		}
		cout << "Recived "<< fd << ": 1st Handshake\n";
	}
	if(type == 's'){
		char screen[1000];
		strcpy(screen, data.c_str());
		cout << "send "<< fd << ": Screen\n";
		state = send(srvSock, screen, sizeof(screen), 0);
		if(state < 0){
			cout << fd << ": Net fail in showing screen\n";
			return -1;
		}
		memset(op, 0, sizeof(op));
		state = recv(srvSock, op, sizeof(op), 0);
		if(state < 0){
			cout << fd << ": Net fail in 2nd shake\n";
			return -1;
		}
		cout << "Recived "<< fd << ": 2nd Handshake\n";
	}
	return 0;
}
	
void process_top(){
	int fd = thread2int[pthread_self()];
	int networkState;
	//显示
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	string screen = "------------\n|   TOP    |\n------------\n|1. login  |\n|2. sign up|\n|3. EXIT   |\n------------\n";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	//输入
	string op;
	networkState = trans('g', op);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	switch(op[0]){
		case '1': layer[fd] = LOGIN; return;
		case '2': layer[fd] = SIGN_UP; return;
		case '3': layer[fd] = EXIT; return;
		default: 
			screen = "Unexpected input\n";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			Sleep(1000);
			return;
	}
}
void process_login(){
	int fd = thread2int[pthread_self()];
	int networkState;
	string username;
	string pwd;
	string op;
	//显示
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	string screen = "-------\n|Login|\n-------\nusername: ";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	//读账号
	networkState = trans('g', username);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	//显示
	screen = "password: ";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	//读密码
	networkState = trans('g', pwd);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	//验证登录
	pthread_mutex_lock(&userInfoLock);
	for(auto it = thread2int.begin(); it !=thread2int.end(); ++it){
		User *u = curUser[it->second];
		if(u != nullptr){
			if(u->getName() == username){
				pthread_mutex_unlock(&userInfoLock);
				screen = "User already online!";
				networkState = trans('s', screen);
				if(networkState < 0){
					layer[fd] = EXIT;
					return;
				}
				Sleep(1000);
				return;
			}
		}
	}
	for(int i = 0; i < (int)userList.size(); i++){
		if(userList[i]->getName() == username && userList[i]->login(pwd) == true){

			curUser[fd] = userList[i];
			//发送问好信息
			screen = "Welcome, " + userList[i]->getName() + "\n";
			pthread_mutex_unlock(&userInfoLock);
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			//进入不同主页
			if(curUser[fd]->getUserType() == TYPE_PLAYER){
				layer[fd] = PLAYER_MAIN;
			}
			else{
				layer[fd] = AUTHOR_MAIN;
			}
			Sleep(1000);
			return;
		}
	}
	pthread_mutex_unlock(&userInfoLock);
	//打印提示信息
	screen = "Wrong username or password, retry again?[y/n]:\n";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	while(1){
		//读操作
		networkState = trans('g', op);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		switch(op[0]){
			case 'y':	return;
			case 'n':	layer[fd] = TOP; return;
			default:
				//打印提示信息
				screen = "please input \"y\" or \"n\":\n";
				networkState = trans('s', screen);
				if(networkState < 0){
					layer[fd] = EXIT;
					return;
				}
				Sleep(1000);
		}
	}
}
void process_sign_up(){
	int fd = thread2int[pthread_self()];
	int networkState;
	string username;
	string pwd;
	string op;
	//显示
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	string screen = "---------\n|Sign Up|\n---------\nusername: ";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	//读用户名
	networkState = trans('g', username);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	cout << "Sign up for user" << username<< endl;
	//显示
	screen = "password: ";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	//读密码
	networkState = trans('g', pwd);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	cout << "With password" << pwd << endl;
	//注册
	bool ok = true;
	pthread_mutex_lock(&userInfoLock);
	for(int i = 0; i < (int)userList.size(); i++){
		if(userList[i]->getName() == username){
			ok = false;
			break;
		}
	}
	pthread_mutex_unlock(&userInfoLock);
	if(ok == false){
		//发送错误信息
		cout << "Same name: " << username << endl;
		screen = "Username already existed, wanna retry?[y/n]\n";
		networkState = trans('s', screen);

		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}

		while(1){
			//读操作
			trans('g', op);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			switch(op[0]){
				case 'y':	return;
				case 'n':	layer[fd] = TOP; return;
				default:
					//打印提示信息
					screen = "please input \"y\" or \"n\":\n";
					networkState = trans('s', screen);
					if(networkState < 0){
						layer[fd] = EXIT;
						return;
					}
			}
		}
	}
	
	cout << "No same name\n"; 
	screen = "Your role is?\n1.player 2.author\n";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	User *u = nullptr;
	while(1){
		//读用户类型
		networkState = trans('g', op);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}

		pthread_mutex_lock(&userInfoLock);
		if(op[0] == '1' || op[0] == 'p' || op[0] == 'P'){u = new Player(username, pwd,0,0,0); break;}
		else if(op[0] == '2' || op[0] == 'a' || op[0] == 'A'){u = new Author(username, pwd,0,0); break;}
		else{
			pthread_mutex_unlock(&userInfoLock);
			screen = "Unexpected Input!\n";
			networkState = trans('s', screen); 
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			Sleep(1000);
			return;
		}
	}
	pthread_mutex_unlock(&userInfoLock);
	if(u == nullptr){
		cout << "wtf??\n";
	}
	userList.push_back(u);
	screen = "Done!";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	Sleep(1000);
	layer[fd] = TOP;
	return;
}

void process_player_main(){
	int fd = thread2int[pthread_self()];
	int networkState;
	string op;
	//显示
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	string screen = "----------------\n|  PLAYER MENU |\n|--------------|\n|1. Play now   |\n|2. Score borad|\n|3. Online List|\n|4. User info  |\n|5. Log out    |\n----------------\n";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	//读操作
	networkState = trans('g', op);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	switch (op[0]){
		case '1': layer[fd] = PLAY_GAME; return;
		case '2': layer[fd] = SCORE_BOARD; return;
		case '3': layer[fd] = ONLINE_LIST; return;
		case '4': layer[fd] = INFO; return;
		case '5': layer[fd] = TOP; return;
		default :
			screen = "Unexpected input!";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			Sleep(1000);
			return; 
	}
}

void process_author_main(){
	int fd = thread2int[pthread_self()];
	int networkState;
	string op;
	//显示
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	string screen = "----------------\n|  AUTHOR MENU |\n----------------\n|1. New word   |\n|2. Score borad|\n|3. User info  |\n|4. Log out    |\n----------------\n";
	//读操作
	networkState = trans('g', op);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	switch (op[0]){
		case '1': layer[fd] = NEW_WORD; return;
		case '2': layer[fd] = SCORE_BOARD; return;
		case '3': layer[fd] = INFO; return;
		case '4': layer[fd] = TOP; return;
		default : 
			screen = "Unexpected input!";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			Sleep(1000);
			return; 
	}
}



void process_score_board(){
	int fd = thread2int[pthread_self()];
	int networkState;
	string op;
	//显示
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	int type = curUser[fd]->getUserType();
	string screen = "----------------\n|  SCORE BOARD |\n----------------\n|1. Players    |\n|2. Authors    |\n|3. Quit       |\n----------------\n";
	networkState = trans('s',screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	//读操作
	networkState = trans('g', op);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	switch (op[0]){
		case '1': 	layer[fd] = RANK_PLAYER; return;
		case '2': 	layer[fd] = RANK_AUTHOR; return;
		case '3': 	switch(type){
						case TYPE_PLAYER:	layer[fd] = PLAYER_MAIN;
											break;
						case TYPE_AUTHOR:	layer[fd] = AUTHOR_MAIN;
											break;
					}
					return;
		default :
			screen = "Unexpected input!";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			Sleep(1000);
			return; 
	}
}

bool player_cmp(User *A, User *B){
	int fd = thread2int[pthread_self()];
	Player *a = (Player*)A;
	Player *b = (Player*)B;
	if(a->getUserType() == TYPE_AUTHOR) return false;
	if(b->getUserType() == TYPE_AUTHOR) return true;
	switch(playerSortTag[fd]){
		case LEVEL:		return a->getLevel() > b->getLevel();
		case EXP:		return a->getExp() > b->getExp();
		case COURSE:	return a->getPassedCourse() > b->getPassedCourse();
	}
	return false;
}
bool author_cmp(User *A, User *B){
	int fd = pthread_self();
	Author *a = (Author*)A;
	Author *b = (Author*)B;
	if(a->getUserType() == TYPE_PLAYER) return false;
	if(b->getUserType() == TYPE_PLAYER) return true;
	switch(authorSortTag[fd]){
		case LEVEL:		return a->getLevel() > b->getLevel();
		case QUIZ:		return a->getQuizNum() > b->getQuizNum();
	}
	return false;
}
void process_rank_player(){
	int fd = thread2int[pthread_self()];
	int networkState;
	string op;
	//显示
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	string screen = "\t\t\t\t\tPlayer Rank\nRANK\tNAME\t\t\tLEVEL\tEXP\tCOURSE\n";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	//打印排行榜
	pthread_mutex_lock(&userInfoLock);
	sort(userList.begin(),userList.end(),player_cmp);
	for(int i = 0; i < (int)userList.size() && i < RANK_BORAD_LENGTH; i++){
		if(userList[i]->getUserType() == TYPE_AUTHOR) {
			break;
		}
		screen =
			to_string(i + 1) + "\t" +
			userList[i]->getName() + "\t\t\t" +
			to_string(userList[i]->getLevel()) + "\t" +
			to_string(((Player*)userList[i])->getExp()) + "\t" +
			to_string(((Player*)userList[i])->getPassedCourse()) + "\n";
		networkState = trans('s', screen);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
	}
	pthread_mutex_unlock(&userInfoLock);
	screen = "--------------------\n|PLAYER SCORE BOARD|\n--------------------\n|1. Sort by LEVEL  |\n|2. Sort by EXP    |\n|3. Sort by COURSES|\n|4. Quit           |\n--------------------\n";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	networkState = trans('g', op);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	switch (op[0]){
		case '1': playerSortTag[fd] = LEVEL; return;
		case '2': playerSortTag[fd] = EXP; return;
		case '3': playerSortTag[fd] = COURSE; return;
		case '4': layer[fd] = SCORE_BOARD; return;
		default : 
			screen = "Unexpected input!";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			Sleep(1000);
			return;  
	}
}

void process_rank_author(){
	int fd = thread2int[pthread_self()];
	int networkState;
	string op;
	//显示
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	string screen = "\t\t\t\t\tAuthor Rank\nRANK\tNAME\t\t\tLEVEL\tQUIZ\n";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	pthread_mutex_lock(&userInfoLock);
	sort(userList.begin(),userList.end(),author_cmp);
	for(int i = 0; i < (int)userList.size() && i < RANK_BORAD_LENGTH; i++){
		if (userList[i]->getUserType() == TYPE_PLAYER) {
			break;
		}
		screen = 
			to_string(i + 1) + "\t" +
			userList[i]->getName() + "\t\t\t" +
			to_string(userList[i]->getLevel()) + "\t" +
			to_string(((Author*)userList[i])->getQuizNum()) + "\n";
		networkState = trans('s', screen);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
	}
	pthread_mutex_unlock(&userInfoLock);
	screen = "--------------------\n|AUTHOR SCORE BOARD|\n--------------------\n|1. Sort by LEVEL  |\n|2. Sort by QUIZ   |\n|3. Quit           |\n--------------------\n";
	//读操作
	networkState = trans('g', op);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	switch (op[0]){
		case '1': authorSortTag[fd] = LEVEL; return;
		case '2': authorSortTag[fd] = QUIZ; return;
		case '3': layer[fd] = SCORE_BOARD; return;
		default :
			screen = "Unexpected input!";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			Sleep(1000);
			return; 
	}
}

void process_info(){
	int fd = thread2int[pthread_self()];
	int type = curUser[fd]->getUserType();
	int networkState;
	string op;
	//显示
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	pthread_mutex_lock(&userInfoLock);
	string screen = "--------------------\n| USER INFORMATION |\n--------------------\nName: " + curUser[fd]->getName() + "\nLevel: " + to_string(curUser[fd]->getLevel()) + "\n";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}

	switch(type){
		case TYPE_PLAYER:
			screen = "Exp: " + to_string(((Player*)curUser[fd])->getExp()) + "\nCourses: " + to_string(((Player*)curUser[fd])->getPassedCourse()) + "\n";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			break;
		case TYPE_AUTHOR:
			screen = "Quiz number: " + to_string(((Author*)curUser[fd])->getQuizNum()) + "\n";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			break;
	}
	pthread_mutex_unlock(&userInfoLock);
	screen = "--------------------\nPress any key to continue:\n";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	networkState = trans('g', op);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	switch(type){
		case TYPE_PLAYER:	layer[fd] = PLAYER_MAIN; return;
		case TYPE_AUTHOR:	layer[fd] = AUTHOR_MAIN; return;
	}
	return;
}

void process_play_game(){
	int fd = thread2int[pthread_self()];
	int networkState;
	string op;
	//显示
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	pthread_mutex_lock(&userInfoLock);
	int courseNum = ((Player*)curUser[fd])->getPassedCourse();
	pthread_mutex_unlock(&userInfoLock);
	string screen = "-----------\n|Play Now!|\n-----------\nYour Course: " + to_string(courseNum) + "\nready?[y/n]: ";
	networkState = trans('s',screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	networkState = trans('g', op);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	switch(op[0]){
		case 'y':	break;
		case 'n':	layer[fd] = PLAYER_MAIN; return;
		default:
			screen = "please input \"y\" or \"n\":\n";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			Sleep(1000);
			return;
	}
	pthread_mutex_lock(&wordBaseLock);
	Course course(courseNum + 1, quizList);
	pthread_mutex_unlock(&wordBaseLock);
	Quiz *quiz = course.getQuiz();
	int expGained = 0;
	while(quiz != nullptr){
		networkState = trans('c');
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		int round = course.getIndex() + 1;
		screen = "ROUND " + to_string(round) + "\n" +quiz->getWord();
		networkState = trans('s', screen);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		Sleep(SHOW_TIME);

		networkState = trans('c');
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		long startTime = time(NULL);
		screen = "ROUND " + to_string(round) + "\n";
		networkState = trans('s', screen);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		//读答案
		networkState = trans('g', op);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		long endTime = time(NULL);
		if(!quiz->check(op)){
			screen = quiz->getWord() + "\n" +"YOU FAILED...\n";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			Sleep(1000);
			return;
		}
		else{
			int time_ = endTime - startTime - lag[fd];
			int expr = course.calcExp(time_);
			expGained += expr;
			screen = "That's right!\n";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}

		}
		quiz = course.getQuiz();
		Sleep(1000);
	}

	//结算
	int round = course.getIndex();
	pthread_mutex_lock(&userInfoLock);
	((Player*)curUser[fd])->gainExp(expGained);
	((Player*)curUser[fd])->courseComplete();
	pthread_mutex_unlock(&userInfoLock);
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	screen = "You'v got " + to_string(round) + " quiz\n"+to_string(expGained) + " exp gained!\nYou wanna go next Course?[y/n]:\n";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	while(1){
		networkState = trans('g', op);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		switch(op[0]){
			case 'y':	return;
			case 'n':	layer[fd] = PLAYER_MAIN; return;
			default:
				screen = "please input \"y\" or \"n\":\n";
				networkState = trans('s', screen);
				if(networkState < 0){
					layer[fd] = EXIT;
					return;
				}
		}
	}
}

void process_new_word(){
	int fd = thread2int[pthread_self()];
	int networkState;
	string op;
	//显示
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	string screen = "-----------\n|NEW WORDS|\n-----------\nInput \"-\" to quit\nWrite down your new word here:\n";
	networkState = trans('g', op);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	if(op[0] == '-'){
		layer[fd] = AUTHOR_MAIN;
		return;
	}
	pthread_mutex_lock(&wordBaseLock);
	bool ok = true;
	for(int i = 0; i < (int)quizList.size(); i++){
		if(quizList[i].getWord() == op){
			ok = false;
		}
	}
	pthread_mutex_unlock(&wordBaseLock);
	if(!ok){
		screen = "That word has already exist!\n";
		networkState = trans('s', screen);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
	}
	else{
		pthread_mutex_lock(&wordBaseLock);
		screen = "Done!\n";
		networkState = trans('s', screen);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		Quiz q(op, op.length());
		quizList.push_back(q);
		pthread_mutex_unlock(&wordBaseLock);
		pthread_mutex_lock(&userInfoLock);
		((Author*)curUser[fd])->quizEstablished();
		pthread_mutex_unlock(&userInfoLock);


	}
	Sleep(1000);
	return;
}

void process_online_list(){
	int fd = thread2int[pthread_self()];
	int networkState;
	string op;
	int type = curUser[fd]->getUserType();
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	string screen = "----------------------\n| ONLINE PLAYER LIST |\n----------------------\nNo.\tUsername\tLevel\tCourse\n";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	pthread_mutex_lock(&userInfoLock);
	int No = 0;
	int onlineUserList[100] = {};
	memset(onlineUserList, 0, sizeof(onlineUserList));
	for(auto it = thread2int.begin(); it !=thread2int.end(); ++it){
		User *u = curUser[it->second];
		if(u != nullptr && u->getName() != curUser[fd]->getName()){
			screen = to_string(++No) + "\t" + u->getName() + "\t\t" + to_string(u->getLevel()) + "\t" + to_string(((Player *)u)->getPassedCourse()) + "\n";
			networkState = trans('s', screen);
			onlineUserList[No] = it->second;//记录下no号对应的会话标识符
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
		}
	}
	pthread_mutex_unlock(&userInfoLock);
	screen = "0. Exit\nn. Challenge Player No.n\n";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	networkState = trans('g', op);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	try{
		int num = stoi(op);
		if(num == 0){
			switch(type){
				case TYPE_PLAYER: layer[fd] = PLAYER_MAIN; return;
				case TYPE_AUTHOR: layer[fd] = AUTHOR_MAIN; return;
			}
			return;
		}
		//挑战处理
		else if(num <= No){
			int fd1 = onlineUserList[num];
			pthread_mutex_lock(&challengeLock);
			if(duelRequest[fd1] != 0 || layer[fd1] == DUEL || layer[fd1] == DUEL_GAIN){//已经有人挑战了
				pthread_mutex_unlock(&challengeLock);
				screen = "Player " + curUser[fd1]->getName() + " got other challenges!";
				networkState = trans('s', screen);
				if(networkState < 0){
					layer[fd] = EXIT;
					return;
				}
				Sleep(1000);
				return;
			}
			else{//对方可以接收挑战
				//交给对方指针
				pthread_cond_t *challengerCond = new pthread_cond_t;
				pthread_cond_init(challengerCond,NULL);
				duelAvaliable[challengerCond] = true;
				duelRequest[fd1] = fd;

				duelCond[fd] = challengerCond;
				//自身进入等待界面
				layerBackup[fd] = layer[fd];
				layer[fd] = DUEL;
			}
			pthread_mutex_unlock(&challengeLock);
			return;
		}
		else{
			screen = "Unexpected input!";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			Sleep(1000);
			return; 
		}
	}
	catch(invalid_argument& e){
        screen = "Unexpected input!";
		networkState = trans('s', screen);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		Sleep(1000);
		return; 
    }
    catch(out_of_range& e){
    	screen = "Unexpected input!";
		networkState = trans('s', screen);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		Sleep(1000);
		return; 
    }
}

//等待界面
void process_duel(){
	int fd = thread2int[pthread_self()];
	int networkState;
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	string screen = "-------------------\n|Wait For Defender|\n-------------------\n";
	networkState = trans('s',screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	//阻塞等待回应
	pthread_mutex_t duelLock;
	pthread_mutex_init(&duelLock, NULL);
	pthread_mutex_lock(&duelLock);
	pthread_cond_wait(duelCond[fd], &duelLock);
	pthread_mutex_unlock(&duelLock);
	//看看应战者有没有应战
	pthread_mutex_lock(&challengeLock);
	if(duelRequest[fd] != 0){//应战了
		int fd1 = duelRequest[fd];
		gameBlock1[fd1] = CreateMutex(NULL, TRUE, NULL);
		gameBlock2[fd1] = CreateMutex(NULL, TRUE, NULL);
		pthread_mutex_unlock(&challengeLock);
		layer[fd] = DUEL_GAME;
		pthread_mutex_lock(&userInfoLock);
		int courseNum = ((Player *)curUser[fd1])->getPassedCourse();
		pthread_mutex_unlock(&userInfoLock);
		//申请一个关卡，使用被挑战者的关卡数
		pthread_mutex_lock(&wordBaseLock);
		Course course(courseNum, quizList);
		pthread_mutex_unlock(&wordBaseLock);
		challengeCourse[fd] = course;
		challengeCourse[fd1] = course;

		pthread_cond_signal(duelCond[fd1]);//释放对方阻塞，双方进入对战
		screen = "Success！！！\n";
		networkState = trans('s',screen);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		Sleep(1000);
		return;
	}
	else{//对方不应战
		layer[fd] = layerBackup[fd];
		duelAvaliable[duelCond[fd]] = false;
		pthread_cond_destroy(duelCond[fd]);
		duelCond[fd] = NULL;
		pthread_mutex_unlock(&challengeLock);
		screen = "Defender REFUSED!\n";
		networkState = trans('s',screen);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		Sleep(1000);
		return;
	}
}

void process_duel_gain(){//应战界面
	int fd = thread2int[pthread_self()];
	int networkState;
	string op;
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	
	pthread_mutex_lock(&challengeLock);
	int fd1 = duelRequest[fd];
	pthread_cond_t *rivalCond = duelCond[fd1];
	pthread_mutex_unlock(&challengeLock);
	//要求应战
	pthread_mutex_lock(&userInfoLock);
	string screen = "Player " + curUser[fd1]->getName() + " Wanna Challenge You!\n do you ACCEPT? [y/n]: ";
	pthread_mutex_unlock(&userInfoLock);
	networkState = trans('s',screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	pthread_mutex_t duel_gainLock;
	pthread_mutex_init(&duel_gainLock,NULL);
	pthread_cond_t *defenderCond = new pthread_cond_t;
	pthread_cond_init(defenderCond, NULL); 

	//读操作
	while(1){
		networkState = trans('g', op);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		switch(op[0]){
			case 'y'://应战
				//挑战相关在操作时锁死
				pthread_mutex_lock(&challengeLock);
				if(!duelAvaliable[rivalCond]){//挑战者跑了
					//释放挑战资源
					layer[fd] = layerBackup[fd];
					duelRequest[fd] = 0;
					return;
				}
				pthread_cond_init(defenderCond,NULL);
				duelAvaliable[defenderCond] = true;
				duelRequest[fd1] = fd;
				duelCond[fd] = defenderCond;
				

				pthread_mutex_unlock(&challengeLock);
				gameBlock1[fd1] = CreateMutex(NULL, TRUE, NULL);
				gameBlock2[fd1] = CreateMutex(NULL, TRUE, NULL);
				//唤起挑战者
				pthread_cond_signal(rivalCond);
				//自身进入游戏界面
				layer[fd] = DUEL_GAME;
				// pthread_mutex_lock(&duel_gainLock);
				// pthread_cond_wait(defenderCond, &duel_gainLock);//等挑战方准备就绪和分发试题
				// pthread_mutex_unlock(&duel_gainLock);
				return;

			case 'n'://拒绝
				delete defenderCond;
				layer[fd] = layerBackup[fd];
				duelRequest[fd] = 0;
				//直接唤起对方
				pthread_cond_signal(rivalCond);
				return;

			default:
				screen = "please input \"y\" or \"n\":\n";
				networkState = trans('s', screen);
				if(networkState < 0){
					layer[fd] = EXIT;
					return;
				}
		}
	}
}

void process_duel_game(){
	int fd = thread2int[pthread_self()];
	pthread_mutex_lock(&challengeLock);
	int fd1 = duelRequest[fd];
	pthread_cond_t *myCond = duelCond[fd];
	pthread_cond_t *rivalCond = duelCond[fd1];
	pthread_mutex_unlock(&challengeLock);
	int networkState;
	string op;
	int expGained = 0;
	int rightNum = 0;
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	string screen = "-----------\n|MULTYPLAY!|\n-----------\n";
	networkState = trans('s',screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	Sleep(1000);
	pthread_mutex_lock(&challengeLock);
	Quiz *quiz = challengeCourse[fd].getQuiz();
	pthread_mutex_unlock(&challengeLock);
	while(quiz != nullptr){
		networkState = trans('c');
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		pthread_mutex_lock(&challengeLock);
		if(!duelAvaliable[rivalCond]){//挑战者跑了
			//释放挑战资源
			layer[fd] = layerBackup[fd];
			duelAvaliable[myCond] = false;
			pthread_mutex_lock(&challengeLock);
			duelRequest[fd] = 0;
			duelCond[fd] = nullptr;
			pthread_mutex_unlock(&challengeLock);
			return;
		}
		pthread_mutex_unlock(&challengeLock);
		//同步双方

		ReleaseMutex(gameBlock1[fd1]);

		WaitForSingleObject(gameBlock1[fd], INFINITE);

		int round = challengeCourse[fd].getIndex();
		screen = "ROUND " + to_string(round) + "\n" +quiz->getWord();
		networkState = trans('s', screen);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		Sleep(SHOW_TIME);

		networkState = trans('c');
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}

		long startTime = time(NULL);
		screen = "ROUND " + to_string(round) + "\n";
		networkState = trans('s', screen);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		//读答案
		networkState = trans('g', op);
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}
		long endTime = time(NULL);
		//计算耗时
		long time_ = endTime - startTime - lag[fd];
		if(!quiz->check(op)){

			time_ = 100000000;//答错了给一个很大的时间，输掉就可以了
			screen = quiz->getWord() + "\n" +"Wrong answer.\n";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
		}
		else{
			screen = "That's right!\n";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
		}
		pthread_mutex_lock(&challengeLock);
		//互发时间
		timeCost[fd1] = time_;
		if(!duelAvaliable[rivalCond]){//挑战者跑了
			//释放挑战资源
			layer[fd] = layerBackup[fd];
			duelAvaliable[myCond] = false;
			pthread_mutex_lock(&challengeLock);
			duelRequest[fd] = 0;
			duelCond[fd] = nullptr;
			pthread_mutex_unlock(&challengeLock);
			return;
		}

		pthread_mutex_unlock(&challengeLock);
		//同步双方
		ReleaseMutex(gameBlock2[fd1]);

		WaitForSingleObject(gameBlock2[fd], INFINITE);
		networkState = trans('c');
		if(networkState < 0){
			layer[fd] = EXIT;
			return;
		}

		if(timeCost[fd] < time_){//输了
			screen = "You lose...\n";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
		}
		else{//赢了
			int expr = challengeCourse[fd].calcExp(time_) * CHALLENGE_MULTI_RATE;
			expGained += expr;
			rightNum ++;
			screen = "YOU WON!\n"+ to_string(expr) + " exp gained!";
			networkState = trans('s', screen);
			if(networkState < 0){
				layer[fd] = EXIT;
				return;
			}
			
		}
		quiz = challengeCourse[fd].getQuiz();
		Sleep(1000);
	}
	networkState = trans('c');
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	pthread_mutex_lock(&userInfoLock);
	((Player*)curUser[fd])->gainExp(expGained);
	pthread_mutex_unlock(&userInfoLock);
	screen = "You'v got " + to_string(rightNum) + " quiz\n"+to_string(expGained) + " exp gained!\n";
	networkState = trans('s', screen);
	if(networkState < 0){
		layer[fd] = EXIT;
		return;
	}
	Sleep(4000);
	//释放挑战资源
	layer[fd] = layerBackup[fd];
	duelAvaliable[myCond] = false;
	pthread_mutex_lock(&challengeLock);
	duelRequest[fd] = 0;
	duelCond[fd] = nullptr;
	pthread_mutex_unlock(&challengeLock);

	ReleaseMutex(gameBlock1[fd1]);

	WaitForSingleObject(gameBlock1[fd], INFINITE);
	return;
}

void process_exit(){
	int fd = thread2int[pthread_self()];
	//清除映射表
	thread2int.erase(pthread_self());
	duelAvaliable.erase(duelCond[fd]);
	//清理用户信息
	lag[fd] = 0;
	layer[fd] = TOP;
	playerSortTag[fd] = LEVEL;
	authorSortTag[fd] = LEVEL;
	curUser[fd] = nullptr;
	duelCond[fd] = NULL;
	//通知终端退出
	trans('e');
	//关闭套接字
	shutdown(sockList[fd],SD_BOTH);
	//结束线程
	pthread_exit(nullptr);
	return;
}
void *process_main(void *arg){
	lagTest();
	int fd = thread2int[pthread_self()];
	while(1){
		cout << fd << " Go to layer " << layer[fd] << "\n";
		if(exitFlag.load()){//检测到全局退出信号，结束所有进程
			layer[fd] = EXIT;
		}
		if(duelRequest[fd] != 0 && layer[fd] != DUEL_GAME && layer[fd] != DUEL_GAME && layer[fd] != DUEL_GAIN){
			layerBackup[fd] = layer[fd];
			layer[fd] = DUEL_GAIN;
		}
		switch(layer[fd]){
			case TOP: 			process_top();			break;
			case LOGIN:			process_login();		break;
			case SIGN_UP:		process_sign_up();		break;
			case PLAYER_MAIN:	process_player_main();	break;
			case AUTHOR_MAIN:	process_author_main();	break;
			case SCORE_BOARD:	process_score_board();	break;
			case RANK_PLAYER:	process_rank_player();	break;
			case RANK_AUTHOR:	process_rank_author();	break;
			case INFO:			process_info();			break;
			case PLAY_GAME:		process_play_game();	break;
			case NEW_WORD:		process_new_word();		break;
			case ONLINE_LIST:	process_online_list();	break;
			case DUEL: 			process_duel();			break;
			case DUEL_GAIN:		process_duel_gain();	break;
			case DUEL_GAME:		process_duel_game();	break;
			//接受后，每个对战唯一生成谜题，统一进入对战，使用两个互锁并等待muti事件以使二人试题同步，每次作答计算时间，时间短的获得经验并提示，最终以获得总经验分胜负，根据layerbackUp回到原界面。
			case EXIT:			process_exit();			return nullptr;
		}
	}
}
void *mainConsole(void *arg){
	string op;
	while(1){
		//system("cls");
		cin >> op;
		if(op == "online"){
			pthread_mutex_lock(&userInfoLock);
			cout<<"在线用户列表："<<"\n";
			for(auto it = thread2int.begin(); it !=thread2int.end(); ++it){

				User *u = curUser[it->second];
				if(u != nullptr){
					cout << u->getName() << "\n";
				}
			}
			pthread_mutex_unlock(&userInfoLock);
		}
		else if(op == "dataInfo"){
			pthread_mutex_lock(&userInfoLock);
			cout << "注册用户数: " << userList.size() << "\n";
			for(User *user : userList){
				cout << user->getName() << "\n";
				cout << "\t 密码" << user->getPwd() << "\n";
				cout << "\t 等级" << user->getLevel() << "\n";	
			}
			pthread_mutex_unlock(&userInfoLock);
			cout << "在库单词数: " << quizList.size() << "\n";
		}
		else if(op == "exit"){
			exitFlag = true;
			pthread_exit(nullptr);
			return nullptr;
		}
	}
}

struct SockandAddr{
	SOCKET sock;
	struct sockaddr_in srvaddr;
};

void *netHandler(void *arg){
	SOCKET srvSock = ((struct SockandAddr *)arg)->sock;
	struct sockaddr_in srvaddr = ((struct SockandAddr *)arg)->srvaddr;
	
	while(1){
		int addrlen = sizeof(srvaddr);
		long long newsocket = accept(srvSock, (sockaddr*)&srvaddr, (int *)&addrlen);
		if(newsocket < 0){
			cout << "netHandler down\n";
			pthread_exit(nullptr);
			return nullptr;
		}
		pthread_t fd;	
		int status = pthread_create(&fd, NULL, process_main,NULL);
		if(status != 0){
			cout << "threadFail\n";
		}
		else{
			if (thread2int.count(fd) == 0) {
		       int i = thread2int.size() + 1;
		       thread2int[fd] = i;
		    }
			sockList[thread2int[fd]] = newsocket;
		}
	}
}

int main(){
	pthread_mutex_init(&userInfoLock, NULL);
	pthread_mutex_init(&wordBaseLock, NULL);
	pthread_mutex_init(&challengeLock, NULL);
	exitFlag = false;

	loadUser(userList, USER_DIR);
	loadQuiz(quizList, QUIZ_DIR);
	for(int i = 0; i < MAX_LINK_NUM; i++){
		gameBlock1[i] = NULL;
		gameBlock2[i] = NULL;
		duelCond[i] = NULL;
		lag[i] = 0;
		playerSortTag[i] = LEVEL;
		authorSortTag[i] = LEVEL;
		layerBackup[i] = TOP;
		layer[i] = TOP;
		curUser[i] = nullptr;
	}
	memset(duelRequest, 0, sizeof(duelRequest));

	WSADATA data;
	int ret0 = WSAStartup(MAKEWORD(2,2),&data);
	if (ret0) {
		printf("初始化网络错误！\n");
		return -1;
	}
	SOCKET srvSock = socket(AF_INET, SOCK_STREAM, 0);
	if((long long)srvSock == -1){
		printf("套接字创建错误\n");
		return -1;
	}
	struct sockaddr_in srvaddr;
	memset(&srvaddr,0, sizeof(srvaddr));
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(1145);
	srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int ret1 = bind(srvSock, (sockaddr*)&srvaddr, sizeof(srvaddr));
	if(ret1 == -1) {
		printf("绑定失败\n");
		return -1;
	}
	listen(srvSock, 1024);


	pthread_t net,console;	
	SockandAddr arg = {
		srvSock,
		srvaddr
	};
	int status0 = pthread_create(&net, NULL, netHandler,(void *)&arg);
	int status1 = pthread_create(&console, NULL, mainConsole,NULL);
	if(status0 == -1 || status1 == -1){
		cout << "thread fail\n";
		return -1;
	}
	pthread_join(console, nullptr);
	cout<<"控制台下线\n";
	shutdown(srvSock, SD_BOTH);
	closesocket(srvSock);
	pthread_join(net, nullptr);
	cout << "链接线程下线\n";
	cout << "服务器下线";
	stoUser(userList, USER_DIR);
	stoQuiz(quizList, QUIZ_DIR);
	return 0;
}