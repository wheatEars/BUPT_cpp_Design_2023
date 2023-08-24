#include "fileOp.h"
using namespace std;

#define ERR_OS_FAILED -1
#define OK 0
int stoUser(vector<User*> &v, string fileDir){
	ofstream outfile(fileDir, ios::out);
	if(!outfile){
		return ERR_OS_FAILED;
	}
	outfile << (int)v.size() << "\n";
	for(int i = 0; i < (int)v.size(); i++){
		if(v[i]->getUserType() == TYPE_PLAYER){
			outfile << "1\n";
			outfile << v[i]->getName() << "\n";
			outfile << v[i]->getPwd() << "\n";
			outfile << v[i]->getLevel() << "\n";
			outfile << ((Player*)v[i])->getExp() << "\n";
			outfile << ((Player*)v[i])->getPassedCourse() << "\n";

		}
		else{
			outfile << "2\n";
			outfile << v[i]->getName() << "\n";
			outfile << v[i]->getPwd() << "\n";
			outfile << v[i]->getLevel() << "\n";
			outfile << ((Author*)v[i])->getQuizNum() << "\n";
		}
		delete v[i];
	}
	v.clear();
	outfile.close();
	return OK;
}
int loadUser(vector<User*> &v, string fileDir){
	ifstream infile(fileDir, ios::in);
	if(!infile){
		return ERR_OS_FAILED;
	}
	int i;
	for(infile >> i; i > 0; i--){
		string name,pwd;
		int type, level, exp, passed, quizNum;
		infile >> type;
		if(type == EOF){
			return ERR_OS_FAILED;
		}
		if(type == TYPE_PLAYER){
			int exp, passed;
			infile >> name;
			infile >> pwd;
			infile >> level;
			infile >> exp;
			infile >> passed;
			User *u = new Player(name, pwd, level, exp, passed);
			v.push_back(u);
		}
		else{
			int quizNum;
			infile >> name;
			infile >> pwd;
			infile >> level;
			infile >> quizNum;
			User *u = new Author(name, pwd, level, quizNum);
			v.push_back(u);
		}
	}
	infile.close();
	return OK;
}
int stoQuiz(vector<Quiz> &v, string fileDir){
	ofstream outfile(fileDir, ios::out);
	if(!outfile){
		return ERR_OS_FAILED;
	}
	outfile << (int)v.size() << "\n";
	for(int i = 0; i < (int)v.size(); i++){
		outfile << v[i].getWord() << "\n";
		outfile << v[i].getDiff() << "\n";
	}
	outfile.close();
	return OK;
}
int loadQuiz(vector<Quiz> &v, string fileDir){
	ifstream infile(fileDir, ios::in);
	if(!infile){
		return ERR_OS_FAILED;
	}
	int i;
	for(infile >> i; i > 0; i--){
		string word;
		int diff;
		infile >> word;
		infile >> diff;
		Quiz q(word,diff);
		v.push_back(q);
	}
	return OK;
}