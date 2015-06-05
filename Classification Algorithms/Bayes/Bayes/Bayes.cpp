#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<map>

using namespace std;

map<string, double> hCountMap; //��¼������߶εĸ���
map<string, double>pMap;    //��¼�Ա����ֵ
map<string, vector<double> > pSegmentMap;  //ÿ����߶���Short��Medium��Tall����
vector<vector<string> > allRecords; //��������(������ͷ)
vector<string> attribute_row; //��ͷ����
int COL_NUM;

//��10CM��Height�ֶΣ��õ��ĸ��εı�ʾ��
string h1 = "[0,1.6)";
string h2 = "[1.6,1.7)";
string h3 = "[1.7,1.8)";
string h4 = "[1.8,1.9)";
string h5 = "[1.9,2.0)";
string h6 = "[2.0,)";

//���ı��ļ��ж���ѵ��������
int ReadData(string filename, vector<vector<string> > &allRecords){
    ifstream ifs(filename.c_str());
    if(!ifs){
        cerr<<"open inputfile failed!"<<endl;
        return -1;
    }
    string line;
    getline(ifs,line);
    string item;
    istringstream strstm(line);

	while(!strstm.eof()){
		strstm>>item;
		attribute_row.push_back(item);
	}
	COL_NUM = attribute_row.size();	

    //��ȡѵ�������ݣ�������allRecords��
    while(getline(ifs,line)){
        vector<string> aRow(COL_NUM);
        istringstream strstm(line);
        for(int i=0;i<aRow.size();++i){
            strstm>>item;
            aRow[i]=item;
        }
        allRecords.push_back(aRow);
    }
    ifs.close();
    return 0;
}

//��Height��ɢ������10CM�ֶ�
/*
ͳ�Ƹ�����߶ε�����������ʵ�����ת������߶�
�ı�ʾ��(��h1��)���浽���ݵ�Height�С��ҽ�ͳ������
���浽Map�У���������ʹ���ݸ���ֱ�ۣ��������ֱ���Ҽ�ࡣ
*/
void DisperseData(){

	hCountMap[h1] = 0;
	hCountMap[h2] = 0;
	hCountMap[h3] = 0;
	hCountMap[h4] = 0;
	hCountMap[h5] = 0;
	hCountMap[h6] = 0;
	string heightStr;
	double tempHeight;
	for(int i = 0; i < allRecords.size(); i++){
		heightStr = allRecords[i][COL_NUM - 2];
		tempHeight = atof(heightStr.c_str());
		if(tempHeight >= 0 && tempHeight < 1.6){
			allRecords[i][COL_NUM - 2] = h1;
		}
		if(tempHeight >= 1.6 && tempHeight < 1.7){
			allRecords[i][COL_NUM - 2] = h2;
		}
		if(tempHeight >= 1.7 && tempHeight < 1.8){
			allRecords[i][COL_NUM - 2] = h3;
		}
		if(tempHeight >= 1.8 && tempHeight < 1.9){
			allRecords[i][COL_NUM - 2] = h4;
		}
		if(tempHeight >= 1.9 && tempHeight < 2.0){
			allRecords[i][COL_NUM - 2] = h5;
		}
		if(tempHeight >= 2.0){
			allRecords[i][COL_NUM - 2] = h6;
		}

		//ͳ�ƴ�������Map����
		hCountMap[allRecords[i][COL_NUM - 2]]++;
	}
}

//�������
/*
�㷨˼·��
�����ÿ��ClassType����Ů�ĸ������Ӷ��ó��ڸ�����߶���
��Ů�ĸ��ʣ���Map�������ʹ������Ӽ��ֱ�ۡ�
���ż��������߶��У���ClassType���ֵĴ������ٳ��Ը���
��߶ε��������Ӷ��ó�������߶��и�ClassType���ֵĸ��ʣ�
��������Map�С�

���ݽṹ�������֮�����ڣ�ͨ��Map�����������߶ε����ݣ�
�ڼ��������߶γ��ֵĸ��ʵȿ��������ͨ��forѭ�������
���磬pSegmentMap��hCountMap��������Ͷ��string(h1��)��Ϊ������
���Կ�����forѭ����������ʣ�������Ҫһ�����һ�����д������
�����������⡣
*/
void CalProbs(const vector<vector<string> > &allRecords){
	double maleCount = 0;
	double femaleCount = 0;
	double maleShortCount = 0;
	double maleMediumCount = 0;
	double maleTallCount = 0;
	double femaleShortCount = 0;
	double femaleMediumCount = 0;
	double femaleTallCount = 0;
	
	for(int i = 0; i < allRecords.size(); i++){
		if(allRecords[i][COL_NUM - 3] == "M"){
			maleCount++;
			if(allRecords[i][COL_NUM - 1] == "Short")
				maleShortCount++;
			else if(allRecords[i][COL_NUM - 1] == "Medium")
				maleMediumCount++;
			else if(allRecords[i][COL_NUM - 1] == "Tall")
				maleTallCount++;
			continue;
		}
		if(allRecords[i][COL_NUM - 3] == "F"){
			femaleCount++;
			if(allRecords[i][COL_NUM - 1] == "Short")
				femaleShortCount++;
			else if(allRecords[i][COL_NUM - 1] == "Medium")
				femaleMediumCount++;
			else if(allRecords[i][COL_NUM - 1] == "Tall")
				femaleTallCount++;
		}
	}

	//�Ա��и���ClassTypeռ�ı���
	pMap["pMaleShort"] = maleShortCount / maleCount;
	pMap["pMaleMedium"] = maleMediumCount / maleCount;
	pMap["pMaleTall"] = maleTallCount / maleCount;
	pMap["pFemaleShort"] = femaleShortCount / femaleCount;
	pMap["pFemaleMedium"] = femaleMediumCount / femaleCount;
	pMap["pFemaleTall"] = femaleTallCount / femaleCount;

	//�������������߶��У���ClassType���ֵĴ���
	vector<double> v1(3,0);
	vector<double> v2(3,0);
	vector<double> v3(3,0);
	vector<double> v4(3,0);
	vector<double> v5(3,0);
	vector<double> v6(3,0);

	//���������߶��У���ClassType���ֵĴ���
	for(int i = 0; i < allRecords.size(); i++){
		string seg = allRecords[i][COL_NUM - 2];
		if(allRecords[i][COL_NUM - 1] == "Short"){		
			if(seg == h1)
				v1[0]++;
			else if(seg == h2)
				v2[0]++;
			else if(seg == h3)
				v3[0]++;
			else if(seg == h4)
				v4[0]++;
			else if(seg == h5)
				v5[0]++;
			else if(seg == h6)
				v6[0]++;
			continue;
		}
		if(allRecords[i][COL_NUM - 1] == "Medium"){		
			if(seg == h1)
				v1[1]++;
			else if(seg == h2)
				v2[1]++;
			else if(seg == h3)
				v3[1]++;
			else if(seg == h4)
				v4[1]++;
			else if(seg == h5)
				v5[1]++;
			else if(seg == h6)
				v6[1]++;
			continue;
		}
		if(allRecords[i][COL_NUM - 1] == "Tall"){		
			if(seg == h1)
				v1[2]++;
			else if(seg == h2)
				v2[2]++;
			else if(seg == h3)
				v3[2]++;
			else if(seg == h4)
				v4[2]++;
			else if(seg == h5)
				v5[2]++;
			else if(seg == h6)
				v6[2]++;
		}
	}

	vector<string> hVec;
	hVec.push_back(h1);
	hVec.push_back(h2);
	hVec.push_back(h3);
	hVec.push_back(h4);
	hVec.push_back(h5);
	hVec.push_back(h6);

	vector<vector<double> > vV;
	vV.push_back(v1);
	vV.push_back(v2);
	vV.push_back(v3);
	vV.push_back(v4);
	vV.push_back(v5);
	vV.push_back(v6);

	//���������߶��и�ClassType���ֵĸ��ʣ���������Map��
	/*
	����pSegmentMap��hCountMap��������Ͷ��string(h1��)��Ϊ������
	���Կ�����forѭ����������ʣ�������Ҫһ�����һ�����д������
	�����������⡣
	*/
	for(int i = 0; i < hVec.size(); i++){
		string hn = hVec[i];
		for(int j = 0; j < vV[i].size(); j++){
			double p = 0.0;
			if(hCountMap[hn] != 0)
				p = vV[i][j] / hCountMap[hn];
			pSegmentMap[hn].push_back(p);
		}
	}
}

//����(��Ҷ˹��ʽ�������)
string DoTest(string name, string gender, double height){
	vector<double> p(3, 0.0);
	vector<double> temp(3, 0.0);
	string hn;
	if(gender == "M"){
		temp[0] = pMap["pMaleShort"];
		temp[1] = pMap["pMaleMedium"];
		temp[2] = pMap["pMaleTall"];
	}else{
		temp[0] = pMap["pFemaleShort"];
		temp[1] = pMap["pFemaleMedium"];
		temp[2] = pMap["pFemaleTall"];
	}
	if(height >= 0 && height < 1.6)
		hn = h1;
	else if(height >= 1.6 && height < 1.7)
		hn = h2;
	else if(height >= 1.7 && height < 1.8)
		hn = h3;	
	else if(height >= 1.8 && height < 1.9)
		hn = h4;	
	else if(height >= 1.9 && height < 2.0)
		hn = h5;
	else if(height >= 2.0)
		hn = h6;
	for(int i = 0; i < 3; i++){
		p[i] = temp[i] * pSegmentMap[hn][i];
	}
	//�������ֵ��Ӧ�ļ�ΪԤ���ClassType
	double maxP = 0.0;
	int maxIndex = 0;
	for(int i = 0; i < p.size(); i++){
		if(p[i] > maxP){
			maxP = p[i];
			maxIndex = i;
		}
	}
	vector<string> types;
	types.push_back("Short");
	types.push_back("Medium");
	types.push_back("Tall");
	return types[maxIndex];
}

int main(){
	ReadData("test.txt", allRecords);
	DisperseData();
	CalProbs(allRecords);
	cout<<"������Ҫ���Ե����ݣ� (Name��Gender(M / F), Height(m)(double��))"<<endl;
	string name, gender;
	double height;
	cin>>name>>gender>>height;
	string result = DoTest(name, gender, height);
	cout<<"���Խ��Ϊ��"<<result<<endl;
	return 0;
}

