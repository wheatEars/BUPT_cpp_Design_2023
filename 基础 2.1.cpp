#include <bits/stdc++.h>
using namespace std;
int main(){
	int res = rand() % 1000;
	int tmp, n = 1;
	while(cin >> tmp, tmp != res){
		cout << "�����̫" << (tmp > res ? "��" : "С") << "��" << endl;
		n++;
	}
	cout << "��ϲ��ԣ�����" << res << ", ������" << n << "��" << endl;
}