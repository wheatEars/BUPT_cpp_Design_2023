#include <bits/stdc++.h>
using namespace std;
class Point{
private:
	float x, y, z;
public:
	Point(float _x = 0, float _y = 0, float _z = 0): x(_x), y(_y), z(_z){
		cout << "Point constructed" << endl;
 	}
 	~Point(){
 		cout << "Point deconstructed" << endl;
 	}
	float distTo(Point &);
};

float Point::distTo(Point &p){
	return sqrt(pow(this->x - p.x, 2) + pow(this->y - p.y, 2) + pow(this->z - p.z, 2));
}

class circle{
private:
	Point centre;
	float radius;

public:
	circle(float x, float y, float rad): centre(x, y), radius(rad){
		cout << "circle constructed" << endl;
	}
	~circle(){
		cout << "circle deconstructed"<< endl;
	}
	bool isIntersecting(circle &);
};

bool circle::isIntersecting(circle &c){
	return this->centre.distTo(c.centre) < this->radius + c.radius;
}

main(){
	int x, y, r;
	cin >> x >> y >> r;
	circle c1(x, y, r);
	cin >> x >> y >> r;
	circle c2(x, y, r);
	cout << c2.isIntersecting(c2) << endl;
	return 0;
}