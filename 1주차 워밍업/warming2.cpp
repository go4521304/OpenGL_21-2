#include <iostream>
#include <fstream>
using namespace std;

int main()
{
	ifstream in{ "data.txt" };

	if (!in)
	{
		cout << "data.txt를 열지 못했습니다." << endl;
		exit(0);
	}

	string s;

	int num_cnt = 0;
	int wd_cnt = 0;

	while (in >> s)
	{
		bool digit = false;
		for (auto i : s)
			if ('0' <= i && i <= '9')
			{
				digit = true;
				break;
			}
		if (digit)
			num_cnt++;
		else
			wd_cnt++;
	}

	cout << "word count: " << wd_cnt << endl << "numbet count: " << num_cnt << endl;
}