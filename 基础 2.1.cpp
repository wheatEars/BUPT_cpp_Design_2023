#include <bits/stdc++.h>
using namespace std;
int main(){
	int res = rand() % 1000;
	int tmp, n = 1;
	while(cin >> tmp, tmp != res){
		cout << "这个数太" << (tmp > res ? "大" : "小") << "了" << endl;
		n++;
	}
	cout << "恭喜答对！答案是" << res << ", 共尝试" << n << "次" << endl;
}