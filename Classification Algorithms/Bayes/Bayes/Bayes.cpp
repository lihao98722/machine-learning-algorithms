#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<map>

using namespace std;

map<string, double> hCountMap; //记录各个身高段的个数
map<string, double>pMap;    //记录性别概率值
map<string, vector<double> > pSegmentMap;  //每个身高段上Short、Medium、Tall概率
vector<vector<string> > allRecords; //所有数据(不含表头)
vector<string> attribute_row; //表头数据
int COL_NUM;

//以10CM将Height分段，得到的各段的标示符
string h1 = "[0,1.6)";
string h2 = "[1.6,1.7)";
string h3 = "[1.7,1.8)";
string h4 = "[1.8,1.9)";
string h5 = "[1.9,2.0)";
string h6 = "[2.0,)";

//从文本文件中读出训练集数据
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

    //读取训练集数据，保存在allRecords中
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

//将Height离散化，以10CM分段
/*
统计各个身高段的人数，并将实际身高转换成身高段
的标示符(如h1等)保存到数据的Height中。且将统计数据
保存到Map中，这样可以使数据更加直观，代码更加直观且简洁。
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

		//统计次数，用Map保存
		hCountMap[allRecords[i][COL_NUM - 2]]++;
	}
}

//计算概率
/*
算法思路：
计算出每个ClassType中男女的个数，从而得出在各个身高段中
男女的概率，用Map保存可以使代码更加简洁直观。
接着计算各个身高段中，各ClassType出现的次数，再除以各个
身高段的总人数从而得出各个身高段中各ClassType出现的概率，
并保存在Map中。

数据结构设计巧妙之处在于，通过Map来保存各个身高段的数据，
在计算各个身高段出现的概率等可以巧妙地通过for循环解决。
比如，pSegmentMap与hCountMap都采用相投的string(h1等)作为索引，
所以可以用for循环来计算概率，而不需要一条语句一条语句写。代码
简洁且易于理解。
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

	//性别中各个ClassType占的比例
	pMap["pMaleShort"] = maleShortCount / maleCount;
	pMap["pMaleMedium"] = maleMediumCount / maleCount;
	pMap["pMaleTall"] = maleTallCount / maleCount;
	pMap["pFemaleShort"] = femaleShortCount / femaleCount;
	pMap["pFemaleMedium"] = femaleMediumCount / femaleCount;
	pMap["pFemaleTall"] = femaleTallCount / femaleCount;

	//用来保存各个身高段中，各ClassType出现的次数
	vector<double> v1(3,0);
	vector<double> v2(3,0);
	vector<double> v3(3,0);
	vector<double> v4(3,0);
	vector<double> v5(3,0);
	vector<double> v6(3,0);

	//计算各个身高段中，各ClassType出现的次数
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

	//计算各个身高段中各ClassType出现的概率，并保存在Map中
	/*
	其中pSegmentMap与hCountMap都采用相投的string(h1等)作为索引，
	所以可以用for循环来计算概率，而不需要一条语句一条语句写。代码
	简洁且易于理解。
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

//测试(贝叶斯公式计算概率)
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
	//概率最大值对应的即为预测的ClassType
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
	cout<<"请输入要测试的数据： (Name，Gender(M / F), Height(m)(double型))"<<endl;
	string name, gender;
	double height;
	cin>>name>>gender>>height;
	string result = DoTest(name, gender, height);
	cout<<"测试结果为："<<result<<endl;
	return 0;
}

