#include <bits/stdc++.h>
using namespace std;

#define TYPE_PLAYER 1
#define TYPE_AUTHOR 2



class User{
protected:
	string name;
	string pwd;
	int level;
	virtual void levelUp() = 0;
public:
	virtual int getUserType() = 0;
	bool login(string);
	string getName();
	string getPwd();
	int getLevel();
	User(string, string, int);
};

class Player: public User{
private:
	int levelDiv[11] = {0, 0, 10, 20, 30, 50, 100, 200, 500, 700, 1000};
	int expr;
	int passedCourse;
	void levelUp();
public:
	Player(string, string, int, int, int);
	void gainExp(int);
	int getExp();
	int getPassedCourse();
	int getUserType();
	void quizSolved(int);
	void courseComplete();
};

class Author: public User{
private:
	int levelDiv[11] = {0, 0, 1, 2, 3, 5, 10, 20, 50, 70, 100};
	int quizNum;
	void levelUp();
public:
	Author(string, string, int, int);
	int getQuizNum();
	int getUserType();
	void quizEstablished();
};

class Quiz{
private:
	string word;
	int diff;
public:
	Quiz(string, int);
	int getDiff();
	string getWord();
	bool check(string);
};

class Course{
private:
	int round;
	vector<Quiz> quizList;
	int index;
	long startTime;
	int courseNum;

public:
	Course(int,vector<Quiz>&);
	int calcExp(int);
	int getTimeUsed(long time_);
	Quiz *getQuiz();
	int getIndex();
	int getRound();
};