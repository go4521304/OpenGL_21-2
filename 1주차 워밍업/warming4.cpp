#include <iostream>
#include <random>
using namespace std;

int main()
{
	random_device rd;
	uniform_int_distribution<int> r_set(1, 800);
	uniform_int_distribution<int> l_set(1, 600);

	int rect[2][2] = { r_set(rd), r_set(rd), r_set(rd), r_set(rd) };
	int line[2][2] = { l_set(rd), l_set(rd), l_set(rd), l_set(rd) };

	while (1)
	{
		cout << endl;
		cout << "Rect: (" << rect[0][1] << ", " << rect[0][0] << ") (" << rect[1][1] << ", " << rect[1][0] << ")" << endl;
		cout << "Line: (" << line[0][1] << ", " << line[0][0] << ") (" << line[1][1] << ", " << line[1][0] << ")" << endl;

		char cmd;
		cin >> cmd;

		int moveR[2] = { 0 }, moveL[2] = { 0 };
		switch (cmd)
		{
		case 'w':
			moveR[0] = 50;
			break;
		case's':
			moveR[0] = -50;
			break;
		case 'a':
			moveR[1] = -50;
			break;
		case 'd':
			moveR[1] = 50;
			break;

		case 'i':
			moveL[0] = 50;
			break;
		case'k':
			moveL[0] = -50;
			break;
		case 'j':
			moveL[1] = -50;
			break;
		case 'l':
			moveL[1] = 50;
			break;

		default:
			break;
		}

		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
			{
				rect[i][j] += moveR[j];
				line[i][j] += moveL[j];
			}

		if (min(rect[0][0], rect[1][0]) < 1 || min(rect[0][1], rect[1][1]) < 1 || max(rect[0][0], rect[1][0]) > 800 || max(rect[0][1], rect[1][1]) > 800)
		{
			cout << "Boundary error" << endl;
			for (int i = 0; i < 2; ++i)
				for (int j = 0; j < 2; ++j)
				{
					rect[i][j] -= moveR[j];
					line[i][j] -= moveL[j];
				}
		}

		if (min(line[0][0], line[1][0]) < 1 || min(line[0][1], line[1][1]) < 1 || max(line[0][0], line[1][0]) > 600 || max(line[0][1], line[1][1]) > 600)
		{
			cout << "Boundary error" << endl;
			for (int i = 0; i < 2; ++i)
				for (int j = 0; j < 2; ++j)
				{
					rect[i][j] -= moveR[j];
					line[i][j] -= moveL[j];
				}
		}

		// 사각형 안에 선분의 끝 중 하나가 있을 경우
		if (min(rect[0][0], rect[1][0]) <= line[0][0] && line[0][0] <= max(rect[0][0], rect[0][1]) &&
			min(rect[0][1], rect[1][1]) <= line[0][1] && line[0][1] <= max(rect[0][1], rect[1][1]))
			cout << "Collide!" << endl;
		else if (min(rect[0][0], rect[1][0]) <= line[1][0] && line[1][0] <= max(rect[0][0], rect[0][1]) &&
			min(rect[0][1], rect[1][1]) <= line[1][1] && line[1][1] <= max(rect[0][1], rect[1][1]))
			cout << "Collide!" << endl;

		else
		{
			int rect_min[2] = { min(rect[0][0], rect[1][0]), min(rect[0][1], rect[1][1]) }, rect_max[2] = { max(rect[0][0], rect[1][0]), max(rect[0][1], rect[1][1]) };
			double slop1, slop2, line_slop;
			slop1 = abs((line[0][1] - rect_min[1]) / (line[0][0] - rect_min[0]));
			slop2 = abs((line[0][1] - rect_max[1]) / (line[0][0] - rect_max[0]));
			line_slop = abs((line[1][1] - line[0][1]) / (line[1][0] - line[0][0]));

			if (min(slop1, slop2) <= line_slop && line_slop <= max(slop1, slop2))
			{
				cout << "Collide!" << endl;
			}
		}
	}
}