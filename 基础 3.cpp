#include <math.h>
#include <iostream>
#define PI 3.1415926535897932384626
using namespace std;
class shape{
public:
	virtual double calArea(){};
	shape(){
		cout << "SHAPE constructed" << "\n";
	}
	~shape(){
		cout << "SHAPE disconstructed" << "\n";
	
	}
};
class rectangle:public shape{
protected:
	double a,b;
public:
	rectangle(double a_ = 0, double b_ = 0):a(a_),b(b_){
		cout << "RECTANGLE constructed" << "\n";
	}
	~rectangle(){
		cout << "RECTANGLE disconstructed" << "\n";
	}
	double calArea(){
		return a * b;
	}
};
class square:public rectangle{
public:
	square(double a_ = 0){
		a=a_;
		b=a_;
		cout << "SQUARE constructed" << "\n";
	}
	~square(){
		cout << "SQUARE disconstructed" << "\n";
	};
	void reSize(double a_){
		a = a_;
		b = a_;
	}
	double calArea(){
		return a * b;
	}
};

class circle:public shape{
private:
	double x,y,r;
public:
	circle(double x_ = 0, double y_ = 0, double r_ = 0):x(x_), y(y_), r(r_){
		cout << "CIRCLE constructed" << "\n";
	}
	~circle(){
		cout << "CIRCLE disconstructed" << "\n";
	}
	void reSize(double x_ = 0, double y_ = 0, double r_ = 0){
		x = x_;
		y = y_;
		r = r_;
	}
	double calArea(){
		return PI * pow(r, 2);
	}
};

int main(){
	square A(2);
	circle B(1,1,2);
	rectangle C(1,2);
	cout << A.calArea() << " " << B.calArea() << " " << C.calArea() << "\n";
}