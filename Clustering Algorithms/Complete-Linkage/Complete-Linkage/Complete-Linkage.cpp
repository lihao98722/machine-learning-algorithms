#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<sstream>

using namespace std;

//每个原始数据为一条Record
class Record{
public:
	string itemName; //类名称
	int typeIndex;  
	vector<double> distances;  //保存与各个点之间的距离(包括和自己本身)
	vector<string> partners;   //保存与其同类的itemName
	Record(string name){
		itemName = name;
	}
	Record(){
		itemName = "";
	}
};

int COL_NUM = 0;  //读取文件时动态判断列数从而保证通用性
int classTypeNum;  //类别个数
vector<string> AttributeRow; //表头描述
vector<Record*> originalRecords;  //原始记录(不含表头)
vector<Record*> currentRecords;  //当前记录(不含表头)
double threshold = 0.0;

//从文本文件中读取数据
void ReadData(string filename){
    ifstream ifs(filename.c_str());
    if(!ifs){
        cerr<<"open inputfile failed!"<<endl;
    }
    string line;
    getline(ifs,line);
    string item;
    istringstream strstm(line);
    strstm>>item;

	//读取表头数据并记录列数
	while(!strstm.eof()){
		strstm>>item;
		AttributeRow.push_back(item);
	}   
	COL_NUM = AttributeRow.size();

	//一开始，每个记录都是独立的一个类
	classTypeNum = AttributeRow.size();  

    //读取数据，保存在originalRecords中
	int index = 0;
    while(getline(ifs,line)){
        Record *aRow = new Record();
        istringstream strstm(line);
		strstm>>item;
		aRow->itemName = item;
        for(int i=0;i<COL_NUM;++i){
			strstm>>item;
			double tempDistance = atof(item.c_str());
			aRow->distances.push_back(tempDistance);
        }
		aRow->typeIndex = index;
		index++;
        originalRecords.push_back(aRow);
    }
	for(int i = 0; i < originalRecords.size(); i++)
		currentRecords.push_back(originalRecords[i]);
    ifs.close();
}

//计算出阀值，用来判断是否聚类
/*
算出各类别之间的总距离，除以两两类别个数，从而得到阀值。
算法优化：
因为每个类别与自身之间的距离都为0，所有读入数据表格的对角线
必定都为0，可以不加入计算。且表格中数据必然相对于对角线对称，
因为类别1与类别2之间的距离与类别2与类别1之间的距离必然相等。
所以此算法只需要算一半的数据量即可。
*/
void CalThreshold(){
	double sum = 0.0;
	int count = 0;
	for(int i = 0; i < currentRecords.size(); i++){
		for(int j = i + 1; j < currentRecords.size(); j++){
			sum += currentRecords[i]->distances[j];
			count++;
		}
	}
	threshold = sum / count;
}

//全连接算法
/*
全连接（CompleteLinkage ，就是取两个类中距离
最远的两个样本的距离作为这两个集合的距离。

算法思路：
循环遍历currentRecords，在遍历的时候只遍历currentRecords保存的记录的右上
半部分，因为距离值沿对角线对称，原理和计算阀值时原理相同。找到两个类间距离最小值，
并记录下这两个类的index，以便将这两个类合并成一个类。在聚类之前，需先判断这个最小
值是否比合并类的阀值小。如果不满足，则聚类结束(因为连最小距离都不满足，其余距离必然无法满足阀值条件。)
在进行聚类，即合并同类时。将被合并的类(即index较大的类)，合并到index较小的类中。
实际操作为：将被合并的类的itemName加入新类中，且更新这个新类对其余类的最小距离，即更新其
distances。然后删除被合并的类(将此类从currentRecords中删除，且将此类对应的index从所有剩余
类中的distances中删除，这样就保证了currentRecords的数据为N * N)。然后继续对剩余类别进行
聚类，直到全部聚为一类或无法继续聚类为止。

算法优化：
该算法的巧妙之处在于聚类过程也是使数据集变小的过程，并且在聚类的同时，同步更新类别的信息。
可以看到，更新类别信息时，只更新了一半信息，即对角线以上类别信息，因为类之间的距离在表格中
是对称的。且这种特殊的合并方式并不需要去改变被合并类的信息(若需要改变，算法时间复杂度将为n * n，
因为必须把所有和被合并的类的item找出，然后修改类别属性，再逐一计算最小距离。)，而此算法
在合并类的时候时间复杂度为N，即更新最小数据集的时间。(在这方面借鉴了动态规划的思想)
*/
void CompleteLinkage(vector<Record*> &currentRecords){ //一定要传引用，否则递归过程并没有改变currentRecords

	//若只剩一种类型，则返回
	if(currentRecords.size() == 1)
		return;
	//先去第一个distance，并记录下index
	double minDistance = currentRecords.front()->distances[1];
	int index1 = 0;
	int index2 = 1;

	//找出距离最短的两个类
	for(int i = 0; i < currentRecords.size(); i++){
		for(int j = i + 1; j < currentRecords[i]->distances.size(); j++){
			//如果minDistance为0，说明这是到本类的距离，不能算
			if(minDistance == 0){
				minDistance = currentRecords[i]->distances[j];
				index1 = i;
				index2 = j;
				continue;
			}
			if(currentRecords[i]->distances[j] == 0)
				continue;
			if(currentRecords[i]->distances[j] < minDistance){
				minDistance = currentRecords[i]->distances[j];
				index1 = i;
				index2 = j;
			}
		}
	}

	//判断是否要合并这两个类
	if(minDistance <= threshold){ //小于阀值则合并类

		//更新类别信息(即和其他类之间的距离信息)
		for(int i = 0; i < currentRecords[index1]->distances.size(); i++){
			if(currentRecords[index1]->distances[i] < currentRecords[index2]->distances[i]){
				currentRecords[index1]->distances[i] = currentRecords[index2]->distances[i];
			}
		}

		//将要被合并的类全部合并到index1的类中
		currentRecords[index1]->partners.push_back(currentRecords[index2]->itemName);
		for(int k = 0; k < currentRecords[index2]->partners.size(); k++)
			currentRecords[index1]->partners.push_back(currentRecords[index2]->partners[k]);

		//将被合并的类删除，即删除行
		vector<Record*>::iterator it = currentRecords.begin();
		for(int i = 0; i < index2; i++)
			it++;
		currentRecords.erase(it);

		//将被合并的类型的相关信息删除，即删除列
		for(int i = 0; i < currentRecords.size(); i++){
			currentRecords[i]->distances.erase(currentRecords[i]->distances.begin() + index2);		
		}
		classTypeNum--;
	}else  //不满足阀值条件则聚类结束
		return;
	//继续进行聚类
	CompleteLinkage(currentRecords); 
}

int main(){
	ReadData("table.txt");
	CalThreshold();
	CompleteLinkage(currentRecords);
	cout<<"经过全连接层次聚类，";
	cout<<"在阀值为 "<<threshold<<" 的条件下，";
	cout<<"总共有 "<<classTypeNum<<" 个分类"<<endl;
	cout<<"聚类结果如下："<<endl;
	for(int i = 0; i < currentRecords.size(); i++){
		cout<<"第"<<i+1<<"类为： ";
		cout<<currentRecords[i]->itemName<<" ";
		for(int j = 0; j < currentRecords[i]->partners.size(); j++)
			cout<<currentRecords[i]->partners[j]<<" ";
		cout<<endl;
	}
	return 0;
}