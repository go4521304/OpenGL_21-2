#include <iostream>
#include <random>
#include <iomanip>
using namespace std;

#define XY 10

int main()
{
	int route[XY][XY] = { 0 }, n = 1;
	int pos[2] = { 0 }, last_pos[2] = { 0 }, last_dir[2] = { 0 }; // last_dir은 순서대로 마지막 방향, 얼마나 연속됬는지

	random_device re;
	uniform_int_distribution<int> rd(1,4);

	last_pos[0] = pos[0];
	last_pos[1] = pos[1];
	route[pos[0]][pos[1]] = n++;
	// 1 상 / 2 하 / 3 좌 / 4 우
	while (1)
	{
		int dir = rd(re);
		switch (dir)
		{
		case 1:
			pos[0] -= 1;
			break;
		case 2:
			pos[0] += 1;
			break;
		case 3:
			pos[1] -= 1;
			break;
		case 4:
			pos[1] += 1;
			break;
		default:
			break;
		}

		if (pos[0] < 0 || pos[0] > (XY - 1) || pos[1] < 0 || pos[1] > (XY - 1))
		{
			pos[0] = last_pos[0];
			pos[1] = last_pos[1];
			continue;
		}

		if (pos[0] != last_pos[0] || pos[1] != last_dir[1])
		{
			if (last_dir[0] != dir)
			{
				last_dir[0] = dir;
				last_dir[1] = 0;
				break;
			}
			else
			{
				last_dir[1]++;
				if (last_dir[1] < 5)
					break;
				else
				{
					pos[0] = last_pos[0];
					pos[1] = last_pos[1];
					continue;
				}
			}
		}
	}


	while (pos[0] != (XY - 1) || pos[1] != (XY - 1))
	{
		last_pos[0] = pos[0];
		last_pos[1] = pos[1];
		route[pos[0]][pos[1]] = n++;

		while (1)
		{
			int dir = rd(re);
			switch (dir)
			{
			case 1:
				pos[0] -= 1;
				break;
			case 2:
				pos[0] += 1;
				break;
			case 3:
				pos[1] -= 1;
				break;
			case 4:
				pos[1] += 1;
				break;
			default:
				break;
			}

			if (pos[0] < 0 || pos[0] > (XY - 1) || pos[1] < 0 || pos[1] > (XY - 1))
			{
				pos[0] = last_pos[0];
				pos[1] = last_pos[1];
				continue;
			}

			if (pos[0] != last_pos[0] || pos[1] != last_dir[1])
			{
				if (last_dir[0] != dir)
				{
					last_dir[0] = dir;
					last_dir[1] = 0;
					break;
				}
				else
				{
					last_dir[1]++;
					if (last_dir[1] < 5)
						break;
					else
					{
						pos[0] = last_pos[0];
						pos[1] = last_pos[1];
						continue;
					}
				}
			}
		}
		
	}
	route[XY-1][XY-1] = n;

	cout.setf(ios::left);
	for (int i = 0; i < XY; ++i)
	{
		for (int j = 0; j < XY; ++j)
			cout << setw(5) << route[i][j];
		cout << endl;
	}

}