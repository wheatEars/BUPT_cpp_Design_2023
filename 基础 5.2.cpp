#include <bits/stdc++.h>
using namespace std;
class Point{
private:
	float x, y;
public:
	Point(float _x = 0, float _y = 0): x(_x), y(_y){}
 	void dump(){
 		cout << "(" << x << ", " << y << ")\n";
 	}
 	Point &operator++();
 	Point operator++(int);
 	Point &operator--();
 	Point operator--(int);
};

Point &Point::operator++(){
	this->x++;
	this->y++;
	return *this;
}
Point Point::operator++(int){
	Point r(*this);
	this->x++;
	this->y++;
	return r;
}
Point &Point::operator--(){
	this->x--;
	this->y--;
	return *this;
}
Point Point::operator--(int){
	Point r(*this);
	this->x--;
	this->y--;
	return r;
}

int main(){
	Point a(0,0);
	Point b(0,0);
	a.dump();
	b = a++;
	b.dump();
	b = a--;
	b.dump();
	b = ++a;
	b.dump();
	b = --a;
	b.dump();
}