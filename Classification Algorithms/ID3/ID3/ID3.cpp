#include <iostream>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
using namespace std;

int COL_LEN = 0;   //输入每行的数据个数(动态换取，先赋值为0)

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
double CalEntropy(vector <vector <string> > remain_state, string attribute, string value,bool ifparent);
double CalGain(vector <vector <string> > remain_state, string attribute);
Node * BulidDecisionTreeDFS(Node * p, vector <vector <string> > remain_state, vector <string> remain_attribute);
int ReadData(string filename);
void PrintTree(Node *p, int depth);
void FreeTree(Node *p);

//全局变量
vector <vector <string> > allRecords;  //所有数据的集合(包含表头数据)
vector <string> aRow(COL_LEN);   //保存每行的数据
map<string,vector < string > > attributeValues;//保存属性所对应的值

//此程序为通用程序，即此ID3算法同样适用于除此训练集以外其他类型的训练集。
vector <string> attribute_row; //保存表头数据
vector <string> classType; //保存Class的数据类型

int main(){

	ReadData("test.txt");
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
	PrintTree(root,0);
	FreeTree(root);
	return 0;
}


//初始化数据，即将属性与其值一一匹配
/*
因为第一列为姓名，ID3算法无法解析这一行，这是ID3算法的局限性之一，
所以必须人为地去除姓名这一列。但这并不妨碍此程序的通用性。
*/
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
	double entropy = 0;
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

//构建决策树，返回树根节点
/*
算法思想为递归建树，遇到满足决策树条件时即返回节点，否则计算最大信息增益，
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

	double max_gain = 0; //最大信息增益
	double temp_gain;
	vector <string>::iterator max_it = remain_attribute.begin();
	vector <string>::iterator it1;  //最大信息增益指向的属性的迭代器
	for(it1 = remain_attribute.begin(); it1 < remain_attribute.end(); it1++){
		temp_gain = CalGain(remain_state, (*it1));
		if(temp_gain > max_gain) {
			max_gain = temp_gain;
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

