#include "fileOp.h"
#include <windows.h>
using namespace std;
#define USER_DIR "./users.dat"
#define QUIZ_DIR "./quiz.dat"

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
		EXIT
};

int layer = TOP;
string username;
string pwd;
User *curUser = NULL;

vector<User*> userList;
vector<Quiz> quizList;


void process_top(){
	system("cls");
	cout << "------------\n";
	cout << "|   TOP    |\n";
	cout << "------------\n";
	cout << "|1. login  |\n";
	cout << "|2. sign up|\n";
	cout << "|3. EXIT   |\n";
	cout << "------------\n";
	string op;
	cin >> op;
	switch(op[0]){
		case '1': layer = LOGIN; return;
		case '2': layer = SIGN_UP; return;
		case '3': layer = EXIT; return;
		default: cout << "Unexpected input\n"; Sleep(1000); return;
	}
}
void process_login(){
	system("cls");
	cout << "-------\n";
	cout << "|Login|\n";
	cout << "-------\n";
	cout << "username: ";
	cin >> username;
	cout << "password: ";
	cin >> pwd;
	for(int i = 0; i < (int)userList.size(); i++){
		if(userList[i]->getName() == username && userList[i]->login(pwd) == true){
			curUser = userList[i];
			cout << "Welcome, " << userList[i]->getName() << "\n";
			if(curUser->getUserType() == TYPE_PLAYER){
				layer = PLAYER_MAIN;
			}
			else{
				layer = AUTHOR_MAIN;
			}
			Sleep(1000);
			return;
		}
	}
	string op;
	cout << "Wrong username or password, retry again?[y/n]\n";
	while(1){
		cin >> op;
		switch(op[0]){
			case 'y':	return;
			case 'n':	layer = TOP; return;
			default:	cout << "please input \"y\" or \"n\":\n";
		}
	}
}

void process_sign_up(){
	system("cls");
	cout << "---------\n";
	cout << "|Sign Up|\n";
	cout << "---------\n";
	cout << "username: ";
	cin >> username;
	cout << "password: ";
	cin >> pwd;
	bool ok = true;
	for(int i = 0; i < (int)userList.size(); i++){
		if(userList[i]->getName() == username){
			ok = false;
			break;
		}
	}
	if(ok == false){
		string op;
		cout << "Username already existed, wanna retry?[y/n]\n";
		while(1){
			cin >> op;
			switch(op[0]){
				case 'y':	return;
				case 'n':	layer = TOP; return;
				default:	cout << "please input \"y\" or \"n\":\n";
			}
		}
	}
	cout << "Your role is?\n";
	cout << "1.player 2.author\n";
	string op;
	User *u = NULL;
	while(1){
		cin >> op;
		if(op[0] == '1' || op == "player" || op == "Player"){u = new Player(username, pwd,0,0,0); break;}
		if(op[0] == '2' || op == "author" || op == "Author"){u = new Author(username, pwd,0,0); break;}
		else{cout << "Unexpected input!"; Sleep(1000); return;}
	}
	userList.push_back(u);
	cout << "Done!\n";
	Sleep(1000);
	layer = TOP;
	return;
}

void process_player_main(){
	system("cls");
	cout << "----------------\n";
	cout << "|  PLAYER MENU |\n";
	cout << "----------------\n";
	cout << "|1. Play now   |\n";
	cout << "|2. Score borad|\n";
	cout << "|3. User info  |\n";
	cout << "|4. Log out    |\n";
	cout << "----------------\n";
	string op;
	cin >> op;
	switch (op[0]){
		case '1': layer = PLAY_GAME; return;
		case '2': layer = SCORE_BOARD; return;
		case '3': layer = INFO; return;
		case '4': cout << "Log out!"; Sleep(1000); layer = TOP; return;
		default : cout << "Unexpected input!"; Sleep(1000); return; 
	}
}

void process_author_main(){
	system("cls");
	cout << "----------------\n";
	cout << "|  AUTHOR MENU |\n";
	cout << "----------------\n";
	cout << "|1. New word   |\n";
	cout << "|2. Score borad|\n";
	cout << "|3. User info  |\n";
	cout << "|4. Log out    |\n";
	cout << "----------------\n";
	string op;
	cin >> op;
	switch (op[0]){
		case '1': layer = NEW_WORD; return;
		case '2': layer = SCORE_BOARD; return;
		case '3': layer = INFO; return;
		case '4': cout << "Log out!"; Sleep(1000); layer = TOP; return;
		default : cout << "Unexpected input!"; Sleep(1000); return; 
	}
}
#define RANK_BORAD_LENGTH 10

enum {
	LEVEL,
	EXP,
	COURSE,
	QUIZ
};
int team = TYPE_PLAYER;
int playerSortTag = 0;
int authorSortTag = 0;


void process_score_board(){
	int type = curUser->getUserType();
	system("cls");
	cout << "----------------\n";
	cout << "|  SCORE BOARD |\n";
	cout << "----------------\n";
	cout << "|1. Players    |\n";
	cout << "|2. Authors    |\n";
	cout << "|3. Quit       |\n";
	cout << "----------------\n";
	string op;
	cin >> op;
	switch (op[0]){
		case '1': 	layer = RANK_PLAYER; return;
		case '2': 	layer = RANK_AUTHOR; return;
		case '3': 	switch(type){
						case TYPE_PLAYER:	layer = PLAYER_MAIN;
											break;
						case TYPE_AUTHOR:	layer = AUTHOR_MAIN;
											break;
					}
					return;
		default : 	cout << "Unexpected input!"; Sleep(1000); return; 
	}
}
bool player_cmp(User *A, User *B){
	Player *a = (Player*)A;
	Player *b = (Player*)B;
	if(a->getUserType() == TYPE_AUTHOR) return false;
	if(b->getUserType() == TYPE_AUTHOR) return true;
	switch(playerSortTag){
		case LEVEL:		return a->getLevel() > b->getLevel();
		case EXP:		return a->getExp() > b->getExp();
		case COURSE:	return a->getPassedCourse() > b->getPassedCourse();
	}
	return false;
}
bool author_cmp(User *A, User *B){
	Author *a = (Author*)A;
	Author *b = (Author*)B;
	if(a->getUserType() == TYPE_PLAYER) return false;
	if(b->getUserType() == TYPE_PLAYER) return true;
	switch(authorSortTag){
		case LEVEL:		return a->getLevel() > b->getLevel();
		case QUIZ:		return a->getQuizNum() > b->getQuizNum();
	}
	return false;
}
void process_rank_player(){
	system("cls");
	cout << "\t\t\t\t\tPlayer Rank\n";
	cout << "RANK\tNAME\t\t\tLEVEL\tEXP\tCOURSE\n";
	sort(userList.begin(),userList.end(),player_cmp);
	for(int i = 0; i < (int)userList.size() && i < RANK_BORAD_LENGTH; i++){
		if(userList[i]->getUserType() == TYPE_AUTHOR) {
			break;
		}
		cout << i + 1 << "\t";
		cout << userList[i]->getName() << "\t\t\t";
		cout <<	userList[i]->getLevel() << "\t";
		cout << ((Player*)userList[i])->getExp() << "\t";
		cout << ((Player*)userList[i])->getPassedCourse() << "\n";
	}
	cout << "--------------------\n";
	cout << "|PLAYER SCORE BOARD|\n";
	cout << "--------------------\n";
	cout << "|1. Sort by LEVEL  |\n";
	cout << "|2. Sort by EXP    |\n";
	cout << "|3. Sort by COURSES|\n";
	cout << "|4. Quit           |\n";
	cout << "--------------------\n";
	string op;
	cin >> op;
	switch (op[0]){
		case '1': playerSortTag = LEVEL; return;
		case '2': playerSortTag = EXP; return;
		case '3': playerSortTag = COURSE; return;
		case '4': layer = SCORE_BOARD; return;
		default : cout << "Unexpected input!"; Sleep(1000); return; 
	}
}
void process_rank_author(){
	system("cls");
	cout << "\t\t\t\t\tAuthor Rank\n";
	cout << "RANK\tNAME\t\t\tLEVEL\tQUIZ\n";
	sort(userList.begin(),userList.end(),author_cmp);
	for(int i = 0; i < (int)userList.size() && i < RANK_BORAD_LENGTH; i++){
		if (userList[i]->getUserType() == TYPE_PLAYER) {
			break;
		}
		cout << i + 1 << "\t";
		cout << userList[i]->getName() << "\t\t\t";
		cout <<	userList[i]->getLevel() << "\t";
		cout << ((Author*)userList[i])->getQuizNum() << "\n";
	}
	cout << "--------------------\n";
	cout << "|AUTHOR SCORE BOARD|\n";
	cout << "--------------------\n";
	cout << "|1. Sort by LEVEL  |\n";
	cout << "|2. Sort by QUIZ   |\n";
	cout << "|3. Quit           |\n";
	cout << "--------------------\n";
	string op;
	cin >> op;
	switch (op[0]){
		case '1': authorSortTag = LEVEL; return;
		case '2': authorSortTag = QUIZ; return;
		case '3': layer = SCORE_BOARD; return;
		default : cout << "Unexpected input!"; Sleep(1000); return; 
	}
}

void process_info(){
	system("cls");
	int type = curUser->getUserType();
	cout << "--------------------\n";
	cout << "| USER INFORMATION |\n";
	cout << "--------------------\n";
	cout << "Name: " << curUser->getName() << "\n";
	cout << "Level: " << curUser->getLevel() << "\n";
	switch(type){
		case TYPE_PLAYER:
			cout << "Exp: " << ((Player*)curUser)->getExp() << "\n";
			cout << "Courses: " << ((Player*)curUser)->getPassedCourse() << "\n";
			break;
		case TYPE_AUTHOR:
			cout << "Quiz number: " << ((Author*)curUser)->getQuizNum() << "\n";
			break;
	}
	cout << "--------------------\n";
	cout << "Press any key to continue:";
	string op;
	cin >> op;
	switch(type){
		case TYPE_PLAYER:	layer = PLAYER_MAIN; return;
		case TYPE_AUTHOR:	layer = AUTHOR_MAIN; return;
	}
	return;
}

#define COURSE_ROUND 3
#define SHOW_TIME 1000
void process_play_game(){
	system("cls");
	cout << "-----------\n";
	cout << "|Play Now!|\n";
	cout << "-----------\n";
	cout << "Your Course: " << ((Player*)curUser)->getPassedCourse() << "\n";
	cout << "ready?[y/n]: ";
	string op;
	cin >> op;
	switch(op[0]){
		case 'y':	break;
		case 'n':	layer = PLAYER_MAIN; return;
		default:	cout << "please input \"y\" or \"n\":\n"; Sleep(1000); return;
	}
	Course course(((Player*)curUser)->getPassedCourse() + 1, quizList);
	Quiz *quiz = course.getQuiz();
	while(quiz != NULL){
	cout << "?" <<endl;
		system("cls");
		int round = course.getIndex() + 1;
		cout << "ROUND "<< round <<"\n";
		cout << quiz->getWord();
		Sleep(SHOW_TIME);

		system("cls");
		cout << "ROUND "<< round <<"\n";
		string ans;
		cin >> ans;
		if(!quiz->check(ans)){
			cout << quiz->getWord();
			cout << "YOU FAILED...\n";
			return;
		}
		else{
			cout << "That's right!\n";
		}
		quiz = course.getQuiz();
		Sleep(1000);
	}

	//结算
	int round = course.getIndex();
	long time_ = time(NULL) - round * 2;
	int expr = course.calcExp(time_);
	((Player*)curUser)->gainExp(expr);
	system("cls");

	((Player*)curUser)->courseComplete();
	cout << "You'v got " << round << " quiz\n";
	cout << expr << " exp gained!\n";
	cout << "You wanna do this one more time?[y/n]:\n";
	while(1){
		cin >> op;
		switch(op[0]){
			case 'y':	return;
			case 'n':	layer = PLAYER_MAIN; return;
			default:	cout << "please input \"y\" or \"n\":\n";
		}
	}
}

void process_new_word(){
	system("cls");
	cout << "-----------\n";
	cout << "|NEW WORDS|\n";
	cout << "-----------\n";
	cout << "Input \"-\" to quit\n";
	cout << "Write down your new word here:\n";
	string word;
	cin >> word;
	if(word[0] == '-'){
		layer = AUTHOR_MAIN;
		return;
	}
	bool ok = true;
	for(int i = 0; i < (int)quizList.size(); i++){
		if(quizList[i].getWord() == word){
			ok = false;
		}
	} 
	if(!ok){
		cout << "That word has already exist!\n";
	}
	else{
		cout << "Done!\n";
		Quiz q(word, word.length());
		quizList.push_back(q);
		((Author*)curUser)->quizEstablished();
	}
	Sleep(1000);
	return;
}

void process_exit(){
	stoUser(userList, USER_DIR);
	stoQuiz(quizList, QUIZ_DIR);
	return;
}
int main(){
	loadUser(userList, USER_DIR);
	loadQuiz(quizList, QUIZ_DIR);;
	while(1){
		switch(layer){
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
			case EXIT:			process_exit();			return 0;
			default: cout << "ERR layer num out of range:" << layer << "\n"; return 0;
		}
	}
}