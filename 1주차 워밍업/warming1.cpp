#include <iostream>
#include <random>
#include <cmath>
#include <vector>
using namespace std;


int det(int matrix[][4])
{
	int sum = 0;
	for (int j = 0; j < 3; ++j)
	{
		vector<int> tmp;
		for (int n = 1; n < 3; ++n)
			for (int m = 0; m < 3; ++m)
				if (m != j)
					tmp.push_back(matrix[n][m]);

		int tmp_sum = tmp[0] * tmp[3] - tmp[1] * tmp[2];
		sum += (matrix[0][j] * pow(-1, j)) * tmp_sum;
	}
	return sum;
}


int main()
{
	int matrix[2][4][4] = { 0 };

	random_device rd;
	uniform_int_distribution<int> tmp(0, 1);

	for (int n = 0; n < 2; ++n)
	{
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				matrix[n][i][j] = tmp(rd);
				cout << matrix[n][i][j] << " ";
			}
			cout << endl;
		}
		cout << endl;

		matrix[n][3][3] = 1;
	}
		

	char command;
	do
	{
		cout << endl << "Command: ";
		cin >> command;

		int tmp_matrix[4][4] = { 0 };

		switch (command)
		{
		case 'm':
			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					int sum = 0;
					for (int l = 0; l < 3; ++l)
					{
						sum += (matrix[0][i][l] * matrix[1][l][j]);
					}

					cout << sum << " ";
				}
				cout << endl;
			}
			break;

		case 'a':
			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					cout << matrix[0][i][j] + matrix[1][i][j] << " ";
				}
				cout << endl;
			}
			break;

		case 'd':
			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					cout << matrix[0][i][j] + matrix[1][i][j] << " ";
				}
				cout << endl;
			}
			break;

		case 'r':
			
			for (int n = 0; n < 2; ++n)
			{
				for (int i = 0; i < 3; ++i)
					for (int j = 0; j < 3; ++j)
						tmp_matrix[i][j] = matrix[n][i][j];
				cout << "행렬식의 값: " << det(tmp_matrix) << " " << endl;
			}
			break;

		case 't':
			for (int n = 0; n < 2; ++n)
			{
				for (int i = 0; i < 3; ++i)
				{
					for (int j = 0; j < 3; ++j)
					{
						tmp_matrix[i][j] = matrix[n][j][i];
						cout << tmp_matrix[i][j] << " ";
					}
					cout << endl;
				}
					
				cout << "행렬식의 값: " << det(tmp_matrix) << " " << endl << endl;
			}
			break;

		case 'h':
			for (int n = 0; n < 2; ++n)
			{
				for (int i = 0; i < 4; ++i)
				{
					for (int j = 0; j < 4; ++j)
					{
						cout << matrix[n][i][j] << " ";

						if (i != 3 && j != 3)
							tmp_matrix[i][j] = matrix[n][i][j];

					}
					cout << endl;
				}
				cout << "행렬식의 값: " << det(tmp_matrix) << " " << endl << endl;

			}
			break;

		case 's':
			for (int n = 0; n < 2; ++n)
			{
				for (int i = 0; i < 3; ++i)
				{
					for (int j = 0; j < 3; ++j)
					{
						matrix[n][i][j] = tmp(rd);
						cout << matrix[n][i][j] << " ";
					}
					cout << endl;
				}
				cout << endl;
			}
			break;

		default:
			break;
		}
	} while (command != 'q');
}