#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

int main()
{
	ifstream in{ "data.txt" };
	if (!in)
	{
		cout << "data.txt ������ �ҷ����µ� �����߽��ϴ�." << endl;
		exit(0);
	}

	vector<string> txt;
	string s;
	while (getline(in, s))
	{
		txt.push_back(s);
		cout << s << endl;
	}

	char cmd;
	vector<char> toggle;
	toggle.reserve(3);
	bool e_flag = false;
	do
	{
		cout << endl << "command: ";
		cin >> cmd;


		if (cmd == 'd' || cmd == 'e' || cmd == 'f')
		{
			auto it = find(toggle.begin(), toggle.end(), cmd);

			if (it == toggle.end())
			{
				toggle.push_back(cmd);
			}
			else
			{
				toggle.erase(it);
			}

			if (cmd == 'e')
				e_flag = !e_flag;
		}

		else if (cmd == 'g')
		{
			string ch, trg;
			cout << endl << "�ٲ� ���ڸ� �Է����ּ���: ";
			cin >> ch;
			cout << "� ���ڷ� �ٲٽðڽ��ϱ�?: ";
			cin >> trg;

			for (int i = 0; i < txt.size(); ++i)
			{
				while (1)
				{
					auto pos = txt[i].find(ch);
					if (pos == string::npos)
						break;

					txt[i].replace(pos, ch.length(), trg);
				}
				cout << txt[i] << endl;
			}
			continue;
		}

		for (auto str : txt)
		{
			if (cmd == 'h')
			{
				string tmp;
				auto it_b = str.cbegin();
				auto it_e = str.crbegin();

				while (it_b != str.cend())
				{
					if (*it_b == *it_e)
					{
						tmp.push_back(*it_b);
						++it_b;
						++it_e;
						continue;
					}
					break;
				}

				if (tmp.empty())
					tmp.push_back('0');
				cout << tmp << endl;
			}

			else if (cmd == 'd' || cmd == 'e' || cmd == 'f')
			{
				string s_tmp(str);
				for (auto i : toggle)
				{
					if (i == 'd')
					{
						reverse(s_tmp.begin(), s_tmp.end());
					}

					else if (i == 'e')
					{
						string tmp;
						int n = 3;
						for (auto s : s_tmp)
						{
							if (n == 0)
							{
								tmp.push_back('*');
								tmp.push_back('*');
								n = 3;
							}
							tmp.push_back(s);
							n--;
						}
						s_tmp = tmp;
					}

					else if (i == 'f')
					{
						// *�� ��������
						if (e_flag)
						{
							string tmp1, tmp2;
							int n = 3;
							for (auto s : s_tmp)
							{
								if (n == 0)
								{
									reverse(tmp2.begin(), tmp2.end());
									tmp1.append(tmp2);
									tmp2.clear();
									n = 5;
								}

								if (n > 3)
									tmp1.push_back(s);
								else
									tmp2.push_back(s);
								n--;
							}
							reverse(tmp2.begin(), tmp2.end());
							tmp1.append(tmp2);
							s_tmp = tmp1;
						}

						// ���⸦ ��������
						else
						{
							string tmp1, tmp2;
							for (auto s : s_tmp)
							{
								if (s == ' ')
								{
									reverse(tmp2.begin(), tmp2.end());
									tmp1.append(tmp2);
									tmp1.push_back(s);

									tmp2.clear();
								}
								else
									tmp2.push_back(s);
							}
							reverse(tmp2.begin(), tmp2.end());
							tmp1.append(tmp2);
							s_tmp = tmp1;
						}
					}
				}
				cout << s_tmp << endl;
			}
		}
	} while (cmd != 'q');
}