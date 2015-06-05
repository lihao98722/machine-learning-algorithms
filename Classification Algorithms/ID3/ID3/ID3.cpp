#include <iostream>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
using namespace std;

int COL_LEN = 0;   //����ÿ�е����ݸ���(��̬��ȡ���ȸ�ֵΪ0)

//�������ڵ�ṹ
struct Node{
	string attribute;  //����ֵ
	string arrived_value;  //���������ֵ
	vector<Node *> childs;  //���еĺ���
	Node(){
		attribute = "";
		arrived_value = "";
	}
} *root; //�����ڵ�

//��������
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

//ȫ�ֱ���
vector <vector <string> > allRecords;  //�������ݵļ���(������ͷ����)
vector <string> aRow(COL_LEN);   //����ÿ�е�����
map<string,vector < string > > attributeValues;//������������Ӧ��ֵ

//�˳���Ϊͨ�ó��򣬼���ID3�㷨ͬ�������ڳ���ѵ���������������͵�ѵ������
vector <string> attribute_row; //�����ͷ����
vector <string> classType; //����Class����������

int main(){

	ReadData("test.txt");
	vector <string> remain_attribute;

	//����������������ͷ����remain_attribute��˵����Щ��ͷ��Ӧ���л�δ������
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


//��ʼ�����ݣ�������������ֵһһƥ��
/*
��Ϊ��һ��Ϊ������ID3�㷨�޷�������һ�У�����ID3�㷨�ľ�����֮һ��
���Ա�����Ϊ��ȥ��������һ�С����Ⲣ�������˳����ͨ���ԡ�
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

//��ȡ�������ڵ���
/*
��Ϊ�����������У���vector<vector<string>> allRecords���������м�¼��
�������м�¼���Կ�����һ�Ŵ��б�ͷ�ı�ÿһ����һ����¼(���˵�һ��)��
ÿһ����һ�����Ե�����ֵ����ȡ�������ڵ��п��Է�������Խ��в�����
*/
int GetAttriNum(string attri){
	for(int i = 0; i < COL_LEN; i++){
		if(!allRecords[0][i].compare(attri)) return i;
	}
	return 0;
}

//��ȡʣ���¼�У��������ε�ClassType��
/*
ͨ��ѭ�������õ�ÿ��ClassType���ֵĴ�������¼��Vector<int> count�У�
Ȼ�����count�ɵ�������������ClassType��index����Ϊcount��ClassType
һһ��Ӧ�����Է���ClassType[maxIndex]��Ϊ������������Class���͡�
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

//�ж�remain_state�е�ClassType�Ƿ�Ϊͬ��
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


//������
/*
�ؼ��㷨������
���Ȼ�ȡ��ͬClassType�ĸ������½�һ��vector<int> count�������Ӧ��ClassType���ֵĴ�����
ѭ������remain_state�����е����ԣ�����δ����������ԣ��������ֵ����value�����������ClassTy����Ӧ��count[k]++��
��Ϊcount��index��ClassType��index��һһ��Ӧ�ģ�����count[k]��ֵ��ΪClassType[k]���ֵĴ�����
Ȼ��ֻ�����count��������г��ִ���֮��sum���ٱ���count��ͨ����Ϣ�����㹫ʽ�ó��ؼ��ɡ�
*/
double CalEntropy(vector <vector <string> > remain_state, string attribute, string value,bool ifparent){
    int classTypeCount = classType.size();
	vector<int> count (classTypeCount,0);

	bool isDone = false; //�ж��Ƿ��Ѿ��������
	for(int j = 1; j < COL_LEN; j++){
		if(isDone) break;
		if(!attribute_row[j].compare(attribute)){
			for(int i = 1; i < remain_state.size(); i++){
				if((!ifparent&&!remain_state[i][j].compare(value)) || ifparent){  //ifparent��¼�Ƿ�Ϊ���ڵ�
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
	//����һClassType���ִ���Ϊ0���򷵻�0��
	for(int k = 0; k < count.size(); k++){
        if(count[k] == 0){
            return 0;
        }
	}
	//����Vector count���Ӷ�����ء�
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


//������Ϣ����
/*
�������ԵĻ��֣�ͨ����Ϣ���湫ʽ�����㵱ǰʣ���¼����Ϣ����,�Ա�ѡȡ
���ŵ����Խ����ٻ��֡�
*/
double CalGain(vector <vector <string> > remain_state, string attribute){

	int j,k,m;
	//������������ʱ����
	double parent_entropy = CalEntropy(remain_state, attribute, "", true);
	double children_entropy = 0;
	//Ȼ���������ֺ����ֵ����
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

//���������������������ڵ�
/*
�㷨˼��Ϊ�ݹ齨���������������������ʱ�����ؽڵ㣬������������Ϣ���棬
ѡȡ��δ�����ǵ������ԣ��Լ���δ����ļ�¼���������ࡣ����������һֱ�ڼ�С��
֪���޷��ٷ֣����߷������Ϊֹ��
����remain_stateΪʣ�������ļ�¼��remian_attributeΪʣ�໹û�п��ǵ����ԡ�
*/
Node * BulidDecisionTreeDFS(Node * p, vector <vector <string> > remain_state, vector <string> remain_attribute){

	if (p == NULL)
		p = new Node();
	
	//��������Ҷ��classTypeһ��������������ֱ�ӷ��ؽڵ�
    for(int k = 0; k < classType.size(); k++){
        if (IfAllTheSameClassType(remain_state, classType[k])){
            p->attribute = classType[k];
            return p;
        }
    }

    //���������Զ��ѿ�����ϣ���ȴû�з���ȫ��������Ϊ�ڴ�����³�����������ClassType
	if(remain_attribute.size() == 0){
		string label = MostCommonLabel(remain_state);
		p->attribute = label;
		return p;
	}

	double max_gain = 0; //�����Ϣ����
	double temp_gain;
	vector <string>::iterator max_it = remain_attribute.begin();
	vector <string>::iterator it1;  //�����Ϣ����ָ������Եĵ�����
	for(it1 = remain_attribute.begin(); it1 < remain_attribute.end(); it1++){
		temp_gain = CalGain(remain_state, (*it1));
		if(temp_gain > max_gain) {
			max_gain = temp_gain;
			max_it = it1;
		}
	}

	//����max_itָ������������ֵ�ǰ���������¼�¼�����Լ�
	vector <string> new_attribute;
	vector <vector <string> > new_state;
	for(vector <string>::iterator it2 = remain_attribute.begin(); it2 < remain_attribute.end(); it2++){
		if((*it2).compare(*max_it)) 
			new_attribute.push_back(*it2);
	}

	//�Ѿ�ȷ������ѻ������ԣ���������
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

		//����ǰû�������֧�����������Ե�ǰ��new_nodeΪҶ�ӽڵ�
		if(new_state.size() == 0){
			new_node->attribute = MostCommonLabel(remain_state);
		}
		else
			//�ݹ齨��
			BulidDecisionTreeDFS(new_node, new_state, new_attribute);

		//�ݹ麯������ʱ���½����븸�ڵ㺢�������������new_state�������Ա�׼����һ��ȡֵ����
		p->childs.push_back(new_node);
		new_state.erase(new_state.begin()+1,new_state.end());
	}
	return p;
}

//���ı��ļ��ж���ѵ��������
/*
�ȶ����ͷ���ݣ�ͨ����ͷ�жϳ����ݵ������Ա�֤ͨ���ԡ�Ȼ����vector<vector<string>> allRecords�������������ݣ�
allRecords[0]����ͷ��allRecords[1]����һ�����ݡ���ͷ����������attribute_row����������
�ڱ������ݵ�ͬʱ��ÿ����һ��ClassType�������һ������Class�����жϴ�ClassType�Ƿ���ֹ�����δ���ֹ���
����뵽���������в�ͬ���͵�ClassType��vector�С����Ǵ��㷨����ͨ�õĹؼ����ڣ�
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

	allRecords.push_back(attribute_row);//������ͷ����

    //��ȡѵ�������ݣ�������allRecords��
    while(getline(ifs,line)){
        vector<string> aRow(COL_LEN);
        istringstream strstm(line);
        for(int i=0;i<aRow.size();++i){
            strstm>>item;
            aRow[i]=item;
         
			//����ͬ��ClassType��¼����
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

//��ӡ��(DFS���������)
void PrintTree(Node *p, int depth){
	for (int i = 0; i < depth; i++) cout << '\t';//����������������tab
	if(!p->arrived_value.empty()){
		cout<<p->arrived_value<<endl;
		for (int i = 0; i < depth+1; i++) cout << '\t';//����������������tab
	}
	cout<<p->attribute<<endl;
	for (vector<Node*>::iterator it = p->childs.begin(); it != p->childs.end(); it++){
		PrintTree(*it, depth + 1);
	}
}

//�ͷ��������нڵ�
void FreeTree(Node *p){
	if (p == NULL)
		return;
	for (vector<Node*>::iterator it = p->childs.begin(); it != p->childs.end(); it++){
		FreeTree(*it);
	}
	delete p;
}

