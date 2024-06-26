#include "ds.h"
using namespace std;



User::User(string _name, string _pwd, int _level): level(_level){
	this->name = string(_name);
	this->pwd = string(_pwd);
	this->level = 0;
}
string User::getName(){
	return this->name;
}
string User::getPwd(){
	return this->pwd;
}
int User::getLevel(){
	return this->level;
}
bool User::login(string pwd){
	return this->pwd == pwd;
}

Player::Player(string name, string pwd, int level = 0, int exp = 0, int passed = 0):User(name, pwd, level), expr(exp), passedCourse(passed) {}

void Player::levelUp(){
	int i;
	for(i = 1; i <= 10; i++){
		if(this->expr < this->levelDiv[i]){
			i--;
			break;
		}
	}
	this->level = i;
	return;
}
int Player::getUserType(){
	return TYPE_PLAYER;
}
int Player::getExp(){
	return this->expr;
}
int Player::getPassedCourse(){
	return this->passedCourse;
}
void Player::gainExp(int expr){
	this->expr += expr;
	this->levelUp();
	return;
}
void Player::courseComplete(){
	this->passedCourse++;
	return;
}
Author::Author(string name, string pwd, int level = 0, int quizNum = 0):User(name, pwd, level), quizNum(quizNum){}

void Author::levelUp(){
	int i;
	for(i = 1; i <= 10; i++){
		if(this->quizNum < this->levelDiv[i]){
			i--;
			break;
		}
	}
	this->level = i;
	return;
}
int Author::getQuizNum(){
	return this->quizNum;
}
int Author::getUserType(){
	return TYPE_AUTHOR;
}
void Author::quizEstablished(){
	this->quizNum++;
	this->levelUp();
	return;
}

Quiz::Quiz(string _word, int _diff):word(_word), diff(_diff){}

int Quiz::getDiff(){
	return this->diff;
}
string Quiz::getWord(){
	return this->word;
}
bool Quiz::check(string word_){
	return word_ == this->word;
}


Course::Course(int courseNum, vector<Quiz> &fullQuiz){
	this->courseNum = courseNum;
	this->round = courseNum / 10 + 1;
	this->index = 0;
	int diff = courseNum / 5 + 3;
	vector<Quiz> limitedQuiz;
	for(int i = 0; i < (int)fullQuiz.size(); i++){
		if(fullQuiz[i].getDiff() == diff){
			limitedQuiz.push_back(fullQuiz[i]);
		}
	}
	//随机排序
	for(int i = limitedQuiz.size(); i > 0; i--){
		srand(unsigned(time(NULL)));
		int index = rand() % i;
		this->quizList.push_back(limitedQuiz[index]);
		limitedQuiz.erase(limitedQuiz.begin() + index);
	}
}
Course::Course(){
	this->courseNum = 0;
	this->round = 0;
	this->index = 0;
}
Course::Course(const Course &C){
	this->courseNum = C.getCourseNum();
	this->round = C.getRound();
	this->index = C.getIndex();
	this->quizList = C.quizList;
}

int Course::calcExp(int time){
	float rate = 4 / (time + 2);
	int basicExp = pow(this->courseNum, 2);
	int finalExp = (basicExp * rate);
	return finalExp;
}

Quiz* Course::getQuiz(){
	if(this->index < (int)this->quizList.size() && this->index < round){
		return &(this->quizList[this->index++]);
	}
	return NULL;
}
int Course::getCourseNum() const {
	return this->courseNum;
}

int Course::getIndex() const {
	return this->index;
}

int Course::getRound() const {
	return this->round;
}