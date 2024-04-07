#include <bits/stdc++.h>
#include <WinSock2.h>
using namespace std;

int main(){
	WSADATA data;
	int ret0 = WSAStartup(MAKEWORD(2,2),&data);
	if (ret0) {
		printf("初始化网络错误！\n");
		return -1;
	}
	SOCKET cliSock = socket(AF_INET, SOCK_STREAM, 0);
	if(cliSock == -1){
		printf("套接字创建错误\n");
		return -1;
	}
	struct sockaddr_in cliaddr;
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(1145);
	cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	cout << "寻找主机\n";
	int ret1 = -1;
	while(ret1 == -1){
		ret1 = connect(cliSock, (sockaddr*)&cliaddr, sizeof(cliaddr));
	}
	cout << "已链接\n";
	Sleep(1000);
	char cmd[100000];
	char screen[100000];
	char input[10000];
	char *OK = (char *)malloc(sizeof(char));
	*OK = 6;
	while(1){
		memset(cmd, 0, sizeof(cmd));
		memset(screen, 0, sizeof(screen));

		int state = recv(cliSock, cmd, sizeof(cmd), 0);
		if(state <=0){
			cout << "链接关闭";
			return 0;
		}
		
		switch(cmd[0]){
			case 'c'://清屏
				system("cls");
				send(cliSock, OK, sizeof(char), 0);
				//cout << "ACK for: C\n";
				break;

			case 's'://数据
				send(cliSock, OK, sizeof(char), 0);
				//cout << "ACK for: S1\n";
				recv(cliSock, screen, sizeof(screen), 0);
				cout << screen;

				send(cliSock, OK, sizeof(char), 0);
				//cout << "ACK for: S2\n";
				break;
			case 'g'://输入
				
				cin.sync();
				cin >> input;
				send(cliSock, input, sizeof(input), 0);
				//cout << "ACK for: G\n";
				break;
			case 't'://延迟测试
				send(cliSock, OK, sizeof(char), 0);
				//cout << "ACK for: T\n";
				break;
			case 'e'://退出

				
				send(cliSock, OK, sizeof(char), 0);
				cout << "ACK for: E\n";
				shutdown(cliSock, SD_BOTH);
				return 0;
		}
	}
}