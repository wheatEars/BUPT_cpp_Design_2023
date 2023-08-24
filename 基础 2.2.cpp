#include<bits/stdc++.h>
using namespace std;
class matrix{
public:
	int row,col;
	int **d = NULL;

	matrix(int, int);
	matrix(const matrix&);
	~matrix();
	void load();
	void dump();
	matrix operator+(const matrix);
	matrix operator-(const matrix);
	matrix &operator=(const matrix &);
};

matrix::matrix(int _row, int _col): row(_row), col(_col), d(new int*[row]){
	for(int i = 0; i < row; i++){
		this->d[i] = new int[col];
	}
}
matrix::matrix(const matrix &m): row(m.row), col(m.col), d(new int*[m.row]){
	for(int i = 0; i < m.row; i++){
		this->d[i] = new int[m.col];
		for(int j = 0; j < m.col; j++){
			this->d[i][j] = m.d[i][j];
		}
	}
}

matrix::~matrix(){
	for(int i = 0; i < this->row; i++){
		delete this->d[i];
	}
	delete this->d;
}
void matrix::load(){
	for(int i = 0; i < this->row; i++){
		for(int j = 0; j < this->col; j++){
			cin >> this->d[i][j];
		}
	}
}
void matrix::dump(){
	for(int i = 0; i < this->row; i++){
		for(int j = 0; j < this->col; j++){
			cout << this->d[i][j] << ' ';
		}
		cout << '\n';
	}
}
matrix matrix::operator+(const matrix m){
	if(this->row != m.row || this->col != m.col){
		cout << "not matched size\n";
		return matrix(0,0);
	}
	for(int i = 0; i < m.row; i++){
		for(int j = 0; j < m.col; j++){
			m.d[i][j] = this->d[i][j] + m.d[i][j];
		}
	}
	return m;
}
matrix matrix::operator-(const matrix m){
	if(this->row != m.row || this->col != m.col){
		cout << "not matched size\n";
		return matrix(0,0);
	}
	for(int i = 0; i < m.row; i++){
		for(int j = 0; j < m.col; j++){
			m.d[i][j] = this->d[i][j] - m.d[i][j];
		}
	}
	return m;
}
matrix &matrix::operator=(const matrix &m){
	for(int i = 0; i < this->row; i++){
		delete this->d[i];
	}
	delete this->d;
	this->d = new int*[m.row];
	this->row = m.row;
	this->col = m.col;
	for(int i = 0; i < m.row; i++){
		this->d[i] = new int[m.col];
		for(int j = 0; j < m.col; j++){
			this->d[i][j] = m.d[i][j];
		}
	}
	return *this;
}
int main(){
	int r,c;
	cin >> r >> c;
	matrix A1(r,c),A2(r,c),A3(r,c);
	A1.load();
	A2.load();
	A3 = A1 + A2;
	A3.dump();
	cout << "\n";
	A3 = A1 - A2;
	A3.dump();
	cout << "\n";

	matrix *pA1 = new matrix(r,c);
	matrix *pA2 = new matrix(r,c);
	matrix *pA3 = new matrix(r,c);
	pA1->load();
	pA2->load();
	*pA3 = *pA1 + *pA2;
	pA3->dump();
	cout << "\n";
	*pA3 = *pA1 - *pA2;
	pA3->dump();

	delete pA1;
	delete pA2;
	delete pA3 ;
}