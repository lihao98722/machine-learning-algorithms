#include <iostream>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
using namespace std;

int COL_LEN = 0;   //输入每行的数据个数(动态判断，先赋值为0)

//决策树节点结构
struct Node{
	string attribute;  //属性值
	string arrived_value;  //到达的属性值
	vector<Node *> childs;  //所有的孩子
	Node(){
		attribute = "";
		arrived_value = "";
	}
} *root; //树根节点

//函数声明
void InitialData();
int GetAttriNum(string attri);
string MostCommonLabel(vector <vector <string> > remain_state);
bool IfAllTheSameClassType(vector <vector <string> > remain_state, string label);
double CalEntropy(vector <vector <string> > remain_state, string attribute, string value,bool ifparent);
double CalGain(vector <vector <string> > remain_state, string attribute);
double CalGainRatio(vector <vector <string> > remain_state, string attribute);
Node * BulidDecisionTreeDFS(Node * p, vector <vector <string> > remain_state, vector <string> remain_attribute);
int ReadData(string filename);
void PrintTree(Node *p, int depth);
void FreeTree(Node *p);
void CalThresholds();
string JudgeRecord(Node* root, vector<string> aRow);
void PruneTree(Node* root);

//全局变量
vector <vector <string> > allRecords;  //所有数据的集合(包含表头数据)
vector <string> aRow(COL_LEN);   //保存每行的数据
map<string,vector < string > > attributeValues;//保存属性所对应的值

vector <string> attribute_row; //保存表头数据
vector <string> classType; //保存Class的数据类型
vector <vector<string> > originalRecords;  //保存在计算阀值之前的原始数据(包含表头数据)

//记录阀值
double threshold1;
double threshold2;

int main(){
	
	ReadData("test.txt");
	CalThresholds(); //计算阀值，并将连续型变量转换为离散型变量
	vector <string> remain_attribute;

	//将数据描述，即表头加入remain_attribute，说明这些表头对应的列还未被考虑
    for(int i = 1; i < attribute_row.size() - 1; i++){
        remain_attribute.push_back(attribute_row[i]);
    }
	vector <vector <string> > remain_state;
	for(unsigned int i = 0; i < allRecords.size(); i++){
		remain_state.push_back(allRecords[i]);
	}
	InitialData();
	
	root = BulidDecisionTreeDFS(root,remain_state,remain_attribute);
	PruneTree(root); //进行悲观剪枝

	PrintTree(root,0);
	FreeTree(root);
	return 0;
}


//初始化数据，即将属性与其值一一匹配
void InitialData(){
	unsigned int i,j,k;
	bool exited = false;
	vector<string> values;
	for(i = 1; i < COL_LEN-1; i++){
		for (j = 1; j < allRecords.size(); j++){
			for (k = 0; k < values.size(); k++){
				if(!values[k].compare(allRecords[j][i])) exited = true;
			}
			if(!exited){
				values.push_back(allRecords[j][i]);
			}
			exited = false;
		}
		attributeValues[allRecords[0][i]] = values;
		values.erase(values.begin(), values.end());
	}
}

//获取属性所在的列
/*
因为在整个程序中，用vector<vector<string>> allRecords来保存所有记录，
所以所有记录可以看成是一张带有表头的表。每一行是一条记录(除了第一行)，
每一列是一个属性的所有值。获取属性所在的列可以方便对属性进行操作。
*/
int GetAttriNum(string attri){
	for(int i = 0; i < COL_LEN; i++){
		if(!allRecords[0][i].compare(attri)) return i;
	}
	return 0;
}

//获取剩余记录中，出现最多次的ClassType。
/*
通过循环遍历得到每种ClassType出现的次数，记录在Vector<int> count中，
然后遍历count可到出现最多次数的ClassType的index，因为count和ClassType
一一对应，所以返回ClassType[maxIndex]即为出现最多次数的Class类型。
*/
string MostCommonLabel(vector <vector <string> > remain_state){
	vector<int> count(classType.size(), 0);
	for(unsigned i = 0; i < remain_state.size(); i++){
        for(int k = 0; k < classType.size(); k++){
            if(!remain_state[i][COL_LEN - 1].compare(classType[k])){
                count[k]++;
            }
        }
	}
	int maxIndex = 0;
	for(int i = 0; i < count.size(); i++){
        if(count[i] > maxIndex)
            maxIndex = i;
	}
    return classType[maxIndex];
}

//判断remain_state中的ClassType是否都为同类
bool IfAllTheSameClassType(vector <vector <string> > remain_state, string label){
	int count = 0;
	for(unsigned int i = 0; i < remain_state.size(); i++){
		if(!remain_state[i][COL_LEN-1].compare(label))
			count++;
	}
	if(count == remain_state.size()-1)
		return true;
	else
		return false;
}

//计算阀值，并将连续型变量Height转为离散型变量
/*
算法思想：
由于存储的Height为string类型，所以先要去掉其单位m，再将Height转换为double类型，用Vector<double> height保存。
在选取阀值点时，若一个一个选取判断，会花费很多时间，因为很多计算是无意义的。
(根据熵的原理，只有两个点之间ClassType不一样，计算才有意义。)
所以做如下算法优化：
先将所有记录行按height的大小排列，因为height的index和allRecords的index是对应的(去掉表头之后)，
所以可以通过比较height中的数据(因为allRecords中string数据无法实现身高比较)，来间接地比较allRecords
中得数据，并进行从小到大排列后保存到allRecordsSorted当中。
接着只需将每两个相邻的row且其ClassType不一样的点的index记录并保存到Vector<int> points中，这些点都是可能去阀值的点。
然后从points中任意取出两个点，计算出其熵即可，而无需对任意两个点之间进行判断。使得熵最大的两个点即为阀值点。
计算阀值后，根据阀值的大小，对源数据进行离散化即可。
*/
void CalThresholds(){
	vector<double> height;
	vector<vector<string> > allRecordsSorted;
	for(int i = 0; i < allRecords.size(); i++){
		originalRecords.push_back(allRecords[i]);
	}

	//将Height转化成double型
	for(int i = 1; i < allRecords.size(); i++){
        string str_height = allRecords[i][COL_LEN - 2];
        str_height = str_height.substr(0,str_height.length()-1);
		height.push_back(atof(str_height.c_str()));
	}

	//通过比较Height，对源数据进行排列(按照Height从小到大)
	for(int i = 0; i < height.size(); i++){
		double temp;
		vector<string> tempVec;
		for(int j = i + 1; j< height.size(); j++){
			if(height[j] < height[i]){
				temp = height[i];
				height[i] = height[j];
				height[j] = temp;
				tempVec = allRecords[i+1];
				allRecords[i+1] = allRecords[j+1];
				allRecords[j+1] = tempVec;
			}
		}
	}

	//将排列好的记录(不含表头)保存到allRecordsSorted中
	for(int i =1; i < allRecords.size(); i++){
		allRecordsSorted.push_back(allRecords[i]);
	}

	double sum = allRecordsSorted.size();
	double max = 0.0;
	double info = 0.0;
	int point1 = 0, point2 = 0;
	vector<int> points;

	//记录所有出现ClassType不相同处
	for(int i = 0; i< allRecordsSorted.size() - 1; i++){
		if(allRecordsSorted[i][COL_LEN - 1] != allRecordsSorted[i + 1][COL_LEN - 1]){
			points.push_back(i + 1);
        }
	}

	//计算最大熵，且记录最大熵对应的分割点
	for(int i = 0; i < points.size(); i++){
		for(int j = i + 1; j < points.size(); j++){
			double index1 = i;
			double index2 = j;
			double d1 = -(index1 + 1)/sum * log(double((index1 + 1)/sum))/log(2.0);
			double d2 = -(index2 - index1)/sum * log(double((index2 - index1)/sum))/log(2.0);
			double d3 = -(sum - index2 -1)/sum * log(double((sum - index2 - 1)/sum))/log(2.0);
            double temp = d1 + d2 +d3;
            if(temp > max){
                point1 = points[index1];
                point2 = points[index2];
				max = temp;
			}
		}
	}
	//记录下阀值
	threshold1 = height[point1];
	threshold2 = height[point2];

	//对连续型数据进行离散化，并制定离散化后的value
	string short_str = "<" + allRecordsSorted[point1][COL_LEN - 2];
	string medium_str = allRecordsSorted[point1][COL_LEN - 2] + "<= AND <" + allRecordsSorted[point2][COL_LEN - 2];
	string tall_str = ">=" + allRecordsSorted[point2][COL_LEN - 2];

	//将连续属性Height离散化
	for(int i =1; i < allRecords.size(); i++){
		if(i <= point1)
			allRecords[i][COL_LEN - 2] = short_str;
		else if(i > point2)
			allRecords[i][COL_LEN - 2] = tall_str;
		else
			allRecords[i][COL_LEN - 2] = medium_str;
	}
}

//计算熵
/*
关键算法描述：
首先获取不同ClassType的个数，新建一个vector<int> count来保存对应的ClassType出现的次数。
循环遍历remain_state下所有的属性，即还未被处理的属性，如果属性值等于value，则遍历所有ClassTy，相应的count[k]++。
因为count的index和ClassType的index是一一对应的，所以count[k]的值即为ClassType[k]出现的次数。
然后只需遍历count，算出所有出现次数之和sum。再遍历count，通过信息量计算公式得出熵即可。
*/
double CalEntropy(vector <vector <string> > remain_state, string attribute, string value,bool ifparent){
    int classTypeCount = classType.size();
	vector<int> count (classTypeCount,0);

	bool isDone = false; //判断是否已经计算完毕
	for(int j = 1; j < COL_LEN; j++){
		if(isDone) break;
		if(!attribute_row[j].compare(attribute)){
			for(int i = 1; i < remain_state.size(); i++){
				if((!ifparent&&!remain_state[i][j].compare(value)) || ifparent){  //ifparent记录是否为父节点
				    for(int k = 0; k < classTypeCount; k++){
                        if(!remain_state[i][COL_LEN - 1].compare(classType[k])){
                            count[k]++;
                        }
				    }
				}
			}
			isDone = true;
		}
	}
	//若任一ClassType出现次数为0，则返回0。
	for(int k = 0; k < count.size(); k++){
        if(count[k] == 0){
            return 0;
        }
	}
	//遍历Vector count，从而算出熵。
	double sum = 0;
	for(int k = 0; k < count.size(); k++){
        sum = sum + count[k];
	}
	double entropy = 0.0;
	for(int k = 0; k < count.size(); k++){
        entropy = entropy - count[k]/sum*log(count[k]/sum)/log(2.0);
	}

	return entropy;
}


//计算信息增益
/*
按照属性的划分，通过信息增益公式来计算当前剩余记录的信息增益,以便选取
最优的属性进行再划分。
*/
double CalGain(vector <vector <string> > remain_state, string attribute){

	int j,k,m;
	//首先求不做划分时的熵
	double parent_entropy = CalEntropy(remain_state, attribute, "", true);
	double children_entropy = 0;
	//然后求做划分后各个值的熵
	vector<string> values = attributeValues[attribute];
	vector<double> ratio;
	vector<int> count_values;
	int tempint;
	for(m = 0; m < values.size(); m++){
		tempint = 0;
		for(k = 1; k < COL_LEN - 1; k++){
			if(!attribute_row[k].compare(attribute)){
				for(j = 1; j < remain_state.size(); j++){
					if(!remain_state[j][k].compare(values[m])){
						tempint++;
					}
				}
			}
		}
		count_values.push_back(tempint);
	}

	for(j = 0; j < values.size(); j++){
		ratio.push_back((double)count_values[j] / (double)(remain_state.size()-1));
	}
	double temp_entropy;
	for(j = 0; j < values.size(); j++){
		temp_entropy = CalEntropy(remain_state, attribute, values[j], false);
		children_entropy += ratio[j] * temp_entropy;
	}
	return (parent_entropy - children_entropy);
}

//计算信息增益比
/*
根据信息增益比公式即可算出信息增益比
*/
double CalGainRatio(vector <vector <string> > remain_state, string attribute){
	double entropy = CalEntropy(remain_state, attribute, "", true);
	double gain = CalGain(remain_state, attribute);
	return gain / entropy;
}

//悲观剪枝法
/*
C4.5采用悲观剪枝法，即后剪枝，这种剪枝法不需要而外的测试数据，只需要把训练数据测试一遍即可。
首先将训练数据作为测试数据，经过生成的决策树进行决策后得到测试结果。将测试结果与真实ClassType
对比，计算出每个叶节点(也就是ClassType节点)的错误命中个数。若全部都正确命中，即错误命中个数
为0，则不需要进行悲观剪枝。若有错误命中，则通过悲观剪枝公式算出错误率，当悲观剪枝条件成立时，
即删除此子树，并用此子树中出现次数最多的叶节点代替之。
*/
void PruneTree(Node* root){
	vector<int> realHits(classType.size(), 0); //测试时真正命中各个ClassType的个数
	vector<int> ShouldHits(classType.size(), 0);  //数据中，理论上应该命中各个ClassType的个数
	vector<int> misHits(classType.size(), 0);   //各个ClassType错误命中的个数
	vector<Node*> currentP(classType.size(), 0);  //保存现在所在子节点

	vector<string> tempRow;
	for(int i = 1; i < originalRecords.size(); i++){
		tempRow = originalRecords[i];
		for(int j = 0; j < classType.size(); j++){ //计算理论上命中个数
			if(tempRow[COL_LEN - 1] == classType[j]){
				ShouldHits[j]++;
			}
		}
		//计算实际测试中命中的个数
		string hitClassType = JudgeRecord(root, tempRow);
		for(int k = 0; k < classType.size(); k++){ 
			if(hitClassType == classType[k]){
				realHits[k]++;
			}
		}
	}
	//计算错误命中的个数
	for(int i = 0; i < classType.size(); i++){
		misHits[i] = ShouldHits[i] - realHits[i];
		if(misHits[i] < 0)
			misHits[i] = -misHits[i];
	}
	//若所有数据均正确命中，则无需剪枝
	bool allHits = true;
	for(int i = 0; i < misHits.size(); i++){
		if(misHits[i] != 0){
			allHits = false;
			break;
		}
	}
	if(allHits)
		return;

	//保存现有分支节点
	for(int i = 0; i < currentP.size(); i++){
		currentP[i] = root->childs[i];
	}
	//若所有节点均为叶节点，那么就没必要剪枝了
	bool isAllLeaves = true;
	for(int i = 0; i < currentP.size(); i++){
		if(currentP[i]->childs.empty())
			continue;
		for(int j = 0; j < currentP[i]->childs.size(); j++)  //根据公式可知，若有孩子节点，则misHits需加上孩子节点总数的一半
			misHits[i] += currentP[i]->childs[j]->childs.size() / 2;
	}
	//根据公式判断是否应该剪枝。
	for(int i = 0; i < misHits.size(); i++){
		double left = misHits[i] + 0.5;
		double temp = misHits[i] * (allRecords.size() - 1 - misHits[i]) / (allRecords.size() - 1);
		double right = misHits[i] + sqrt(temp);
		if(left < right){
			//进行剪枝，用出现次数最多的叶节点代替此子树
			currentP[i]->attribute = MostCommonLabel(allRecords);
			currentP[i]->childs.clear(); //清空子树
		}
	}
}

//判断测试数据，返回测试结果ClassType
string JudgeRecord(Node* root, vector<string> aRow){
	string classTypeStr = "";
	int attriNum = GetAttriNum(root->attribute);
	if(attriNum == COL_LEN - 2){ //说明为Height属性，需要将Height转换成double类型
		string str_height = aRow[COL_LEN - 2];
        str_height = str_height.substr(0,str_height.length()-1);
		double tempHeight = atof(str_height.c_str());
		if(tempHeight < threshold1){ //根据height的值，与阀值点进行比较，判断应该走向的决策树分支
			classTypeStr = root->childs[0]->attribute;
		}else if(tempHeight >= threshold2){
			classTypeStr = root->childs[2]->attribute;
		}else
			classTypeStr = root->childs[1]->attribute;
			
	}
	return classTypeStr;
}

//构建决策树，返回树根节点
/*
算法思想为递归建树，遇到满足决策树条件时即返回节点，否则计算最大信息增益比，
选取还未被考虑到的属性，以及还未分类的记录，继续分类。所以样本集一直在减小，
知道无法再分，或者分类完成为止。
其中remain_state为剩余待分类的记录，remian_attribute为剩余还没有考虑的属性。
*/
Node * BulidDecisionTreeDFS(Node * p, vector <vector <string> > remain_state, vector <string> remain_attribute){

	if (p == NULL)
		p = new Node();

	//若所有树叶的classType一样，则分类结束，直接返回节点
    for(int k = 0; k < classType.size(); k++){
        if (IfAllTheSameClassType(remain_state, classType[k])){
            p->attribute = classType[k];
            return p;
        }
    }

    //若所有属性都已考虑完毕，但却没有分完全部，则设为在此情况下出现最多次数的ClassType
	if(remain_attribute.size() == 0){
		string label = MostCommonLabel(remain_state);
		p->attribute = label;
		return p;
	}

	double max_gainRatio = 0; //最大信息增益比
	double temp_gainRatio;
	vector <string>::iterator max_it = remain_attribute.begin();
	vector <string>::iterator it1;  //最大信息增益比指向的属性的迭代器
	for(it1 = remain_attribute.begin(); it1 < remain_attribute.end(); it1++){
		temp_gainRatio = CalGainRatio(remain_state, (*it1));
		if(temp_gainRatio > max_gainRatio) {
			max_gainRatio = temp_gainRatio;
			max_it = it1;
		}
	}

	//根据max_it指向的属性来划分当前样例，更新记录和属性集
	vector <string> new_attribute;
	vector <vector <string> > new_state;
	for(vector <string>::iterator it2 = remain_attribute.begin(); it2 < remain_attribute.end(); it2++){
		if((*it2).compare(*max_it))
			new_attribute.push_back(*it2);
	}

	//已经确定了最佳划分属性，保存下来
	p->attribute = *max_it;
	vector <string> values = attributeValues[*max_it];
	int attribue_num = GetAttriNum(*max_it);
	new_state.push_back(attribute_row);
	for(vector <string>::iterator it3 = values.begin(); it3 < values.end(); it3++){
		for(unsigned int i = 1; i < remain_state.size(); i++){
			if(!remain_state[i][attribue_num].compare(*it3)){
				new_state.push_back(remain_state[i]);
			}
		}
		Node * new_node = new Node();
		new_node->arrived_value = *it3;

		//若当前没有这个分支的样例，则以当前的new_node为叶子节点
		if(new_state.size() == 0){
			new_node->attribute = MostCommonLabel(remain_state);
		}
		else
			//递归建树
			BulidDecisionTreeDFS(new_node, new_state, new_attribute);

		//递归函数返回时将新结点加入父节点孩子容器，再清除new_state容器，以便准备下一个取值样例
		p->childs.push_back(new_node);
		new_state.erase(new_state.begin()+1,new_state.end());
	}
	return p;
}

//从文本文件中读出训练集数据
/*
先读入表头数据，通过表头判断出数据的列数以保证通用性。然后用vector<vector<string>> allRecords来保存所有数据，
allRecords[0]即表头，allRecords[1]即第一行数据。表头属性描述用attribute_row保存起来。
在保存数据的同时，每遇到一个ClassType，即最后一个属性Class，就判断此ClassType是否出现过，若未出现过，
则加入到保存了所有不同类型的ClassType的vector中。这是此算法可以通用的关键所在！
*/
int ReadData(string filename){
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
	COL_LEN = attribute_row.size();

	allRecords.push_back(attribute_row);//包含表头数据

    //读取训练集数据，保存在allRecords中
    while(getline(ifs,line)){
        vector<string> aRow(COL_LEN);
        istringstream strstm(line);
        for(int i=0;i<aRow.size();++i){
            strstm>>item;
            aRow[i]=item;
         
			//将不同的ClassType记录下来
			if(i == COL_LEN - 1){
                vector<string>::iterator it = classType.begin();
                while(it != classType.end()){
                    if(*it == aRow[i])
                        break;
                    it++;
                }
                if(it == classType.end())
                    classType.push_back(aRow[i]);
			}
        }
        allRecords.push_back(aRow);
    }   
    ifs.close();
    return 0;
}

//打印树(DFS，深度优先)
void PrintTree(Node *p, int depth){
	for (int i = 0; i < depth; i++) cout << '\t';//按照树的深度先输出tab
	if(!p->arrived_value.empty()){
		cout<<p->arrived_value<<endl;
		for (int i = 0; i < depth+1; i++) cout << '\t';//按照树的深度先输出tab
	}
	cout<<p->attribute<<endl;
	for (vector<Node*>::iterator it = p->childs.begin(); it != p->childs.end(); it++){
		PrintTree(*it, depth + 1);
	}
}

//释放树的所有节点
void FreeTree(Node *p){
	if (p == NULL)
		return;
	for (vector<Node*>::iterator it = p->childs.begin(); it != p->childs.end(); it++){
		FreeTree(*it);
	}
	delete p;
}

