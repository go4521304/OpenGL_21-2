#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

struct xyz
{
	int x, y, z;
};

void print(int i, xyz tmp)
{
	cout << i << " : " << tmp.x << " " << tmp.y << " " << tmp.z << endl;
}

double getVal(xyz tmp)
{
	return pow(tmp.x, 2) + pow(tmp.y, 2) + pow(tmp.z, 2);
}

int main()
{
	xyz lst[10] = { xyz{0,0,0} };
	int idx_f = 0, idx_e = 0;
	string cmd;

	bool s_flag = false;

	while (1)
	{
		// 저장 리스트 출력
		if (s_flag)
		{
			vector<int> index;
			for (int i = idx_f; i < idx_e; ++i)
				index.push_back(i);

			sort(index.begin(), index.end(), [&lst](const int a, const int b) {
				return getVal(lst[a]) < getVal(lst[b]);
				});

			int l;
			for (l = 9; l >= index.size(); --l)
				cout << l << " : " << endl;
			auto iter = index.crbegin();
			while (iter != index.crend())
			{
				print(l--, lst[*iter]);
				iter++;
			}
		}

		else
		{
			int l;
			for (l = 9; l >= idx_e; --l)
				cout << l << " : " << endl;
			while (l >= idx_f)
			{
				print(l, lst[l]);
				l--;
			}
			for (l = idx_f - 1; l >= 0; --l)
				cout << l << " : " << endl;
		}
		

		cout << endl;

		getline(cin, cmd);

		if (cmd.at(0) == '+')
		{
			// 좌표 분리
			string tmp_s;
			vector<int> cord;
			auto i = find(cmd.begin(), cmd.end(), ' ');
			while (1)
			{
				++i;

				if (i == cmd.end() || *i == ' ')
				{
					cord.push_back(stoi(tmp_s));
					tmp_s.clear();

					if (i == cmd.end())
						break;
				}
				else
				{
					tmp_s.push_back(*i);
				}
			}

			// 빈공간 찾기
			int target_idx = -1;
			for (int i = -1; i < idx_f; ++i)
				target_idx = i;
			for (int i = 9; i >= idx_e; --i)
				target_idx = i;

			if (target_idx != -1)
			{
				lst[target_idx].x = cord.at(0);
				lst[target_idx].y = cord.at(1);
				lst[target_idx].z = cord.at(2);

				if (target_idx < idx_f)
					idx_f = target_idx;
				else
					idx_e = target_idx + 1;
			}
		}

		else if (cmd.at(0) == '-')
		{
			idx_e--;
			if (idx_e == 0 || idx_e < idx_f)
			{
				idx_f = 0;
				idx_e = 0;
			}
		}

		else if (cmd.at(0) == 'e')
		{
			if (idx_e != 10 || idx_f != 0)
			{
				// 좌표 분리
				string tmp_s;
				vector<int> cord;
				auto i = find(cmd.begin(), cmd.end(), ' ');
				while (1)
				{
					++i;

					if (i == cmd.end() || *i == ' ')
					{
						cord.push_back(stoi(tmp_s));
						tmp_s.clear();

						if (i == cmd.end())
							break;
					}
					else
					{
						tmp_s.push_back(*i);
					}
				}

				// 빈공간 찾기
				int target_idx = -1;
				for (int i = -1; i < idx_f; ++i)
					target_idx = i;

				// 아래에 빈 공간이 없을 때, 위쪽의 여유공간이 있을 때
				if (target_idx == -1 && idx_e != 10)
				{
					for (int i = idx_e; i > idx_f; --i)
					{
						lst[i] = lst[i - 1];
					}
					target_idx = idx_f;
					idx_e += 1;
				}

				if (target_idx != -1)
				{
					lst[target_idx].x = cord.at(0);
					lst[target_idx].y = cord.at(1);
					lst[target_idx].z = cord.at(2);
				}
			}
		}

		else if (cmd.at(0) == 'd')
		{
			idx_f++;
			if (idx_f == 10 || idx_e < idx_f)
			{
				idx_f = 0;
				idx_e = 0;
			}
		}

		else if (cmd.at(0) == 'l')
		{
			cout << "Lenght: " << idx_e - idx_f << endl << endl;
		}

		else if (cmd.at(0) == 'c')
		{
			idx_f = 0;
			idx_e = 0;
		}

		else if (cmd.at(0) == 'm')
		{
			if (idx_e - idx_f > 0)
			{
				int max = idx_f;
				for (int i = idx_f; i < idx_e; ++i)
				{
					if (getVal(lst[max]) < getVal(lst[i]))
						max = i;
				}

				cout << "Far ";
				print(max, lst[max]);
			}
			else
			{
				cout << "There's no data." << endl << endl;
			}
		}

		else if (cmd.at(0) == 'n')
		{
			if (idx_e - idx_f > 0)
			{
				int min = idx_f;
				for (int i = idx_f; i < idx_e; ++i)
				{
					if (getVal(lst[min]) > getVal(lst[i]))
						min = i;
				}

				cout << "Near ";
				print(min, lst[min]);
				cout << endl;
			}
			else
			{
				cout << "There's no data." << endl << endl;
			}
		}

		else if (cmd.at(0) == 's')
		{
			s_flag = !s_flag;
		}

		else if (cmd.at(0) == 'q')
			break;

		if (idx_e - idx_f < 1)
		{
			idx_f = 0;
			idx_e = 0;
		}
	}
}