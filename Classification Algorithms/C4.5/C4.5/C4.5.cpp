#include <iostream>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
using namespace std;

int COL_LEN = 0;   //����ÿ�е����ݸ���(��̬�жϣ��ȸ�ֵΪ0)

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
double CalGain(vector <vector <string> > remain_state, string attribute);
double CalGainRatio(vector <vector <string> > remain_state, string attribute);
Node * BulidDecisionTreeDFS(Node * p, vector <vector <string> > remain_state, vector <string> remain_attribute);
int ReadData(string filename);
void PrintTree(Node *p, int depth);
void FreeTree(Node *p);
void CalThresholds();
string JudgeRecord(Node* root, vector<string> aRow);
void PruneTree(Node* root);

//ȫ�ֱ���
vector <vector <string> > allRecords;  //�������ݵļ���(������ͷ����)
vector <string> aRow(COL_LEN);   //����ÿ�е�����
map<string,vector < string > > attributeValues;//������������Ӧ��ֵ

vector <string> attribute_row; //�����ͷ����
vector <string> classType; //����Class����������
vector <vector<string> > originalRecords;  //�����ڼ��㷧ֵ֮ǰ��ԭʼ����(������ͷ����)

//��¼��ֵ
double threshold1;
double threshold2;

int main(){
	
	ReadData("test.txt");
	CalThresholds(); //���㷧ֵ�����������ͱ���ת��Ϊ��ɢ�ͱ���
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
	PruneTree(root); //���б��ۼ�֦

	PrintTree(root,0);
	FreeTree(root);
	return 0;
}


//��ʼ�����ݣ�������������ֵһһƥ��
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

//���㷧ֵ�����������ͱ���HeightתΪ��ɢ�ͱ���
/*
�㷨˼�룺
���ڴ洢��HeightΪstring���ͣ�������Ҫȥ���䵥λm���ٽ�Heightת��Ϊdouble���ͣ���Vector<double> height���档
��ѡȡ��ֵ��ʱ����һ��һ��ѡȡ�жϣ��Ứ�Ѻܶ�ʱ�䣬��Ϊ�ܶ������������ġ�
(�����ص�ԭ��ֻ��������֮��ClassType��һ��������������塣)
�����������㷨�Ż���
�Ƚ����м�¼�а�height�Ĵ�С���У���Ϊheight��index��allRecords��index�Ƕ�Ӧ��(ȥ����ͷ֮��)��
���Կ���ͨ���Ƚ�height�е�����(��ΪallRecords��string�����޷�ʵ����߱Ƚ�)������ӵرȽ�allRecords
�е����ݣ������д�С�������к󱣴浽allRecordsSorted���С�
����ֻ�轫ÿ�������ڵ�row����ClassType��һ���ĵ��index��¼�����浽Vector<int> points�У���Щ�㶼�ǿ���ȥ��ֵ�ĵ㡣
Ȼ���points������ȡ�������㣬��������ؼ��ɣ������������������֮������жϡ�ʹ�������������㼴Ϊ��ֵ�㡣
���㷧ֵ�󣬸��ݷ�ֵ�Ĵ�С����Դ���ݽ�����ɢ�����ɡ�
*/
void CalThresholds(){
	vector<double> height;
	vector<vector<string> > allRecordsSorted;
	for(int i = 0; i < allRecords.size(); i++){
		originalRecords.push_back(allRecords[i]);
	}

	//��Heightת����double��
	for(int i = 1; i < allRecords.size(); i++){
        string str_height = allRecords[i][COL_LEN - 2];
        str_height = str_height.substr(0,str_height.length()-1);
		height.push_back(atof(str_height.c_str()));
	}

	//ͨ���Ƚ�Height����Դ���ݽ�������(����Height��С����)
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

	//�����кõļ�¼(������ͷ)���浽allRecordsSorted��
	for(int i =1; i < allRecords.size(); i++){
		allRecordsSorted.push_back(allRecords[i]);
	}

	double sum = allRecordsSorted.size();
	double max = 0.0;
	double info = 0.0;
	int point1 = 0, point2 = 0;
	vector<int> points;

	//��¼���г���ClassType����ͬ��
	for(int i = 0; i< allRecordsSorted.size() - 1; i++){
		if(allRecordsSorted[i][COL_LEN - 1] != allRecordsSorted[i + 1][COL_LEN - 1]){
			points.push_back(i + 1);
        }
	}

	//��������أ��Ҽ�¼����ض�Ӧ�ķָ��
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
	//��¼�·�ֵ
	threshold1 = height[point1];
	threshold2 = height[point2];

	//�����������ݽ�����ɢ�������ƶ���ɢ�����value
	string short_str = "<" + allRecordsSorted[point1][COL_LEN - 2];
	string medium_str = allRecordsSorted[point1][COL_LEN - 2] + "<= AND <" + allRecordsSorted[point2][COL_LEN - 2];
	string tall_str = ">=" + allRecordsSorted[point2][COL_LEN - 2];

	//����������Height��ɢ��
	for(int i =1; i < allRecords.size(); i++){
		if(i <= point1)
			allRecords[i][COL_LEN - 2] = short_str;
		else if(i > point2)
			allRecords[i][COL_LEN - 2] = tall_str;
		else
			allRecords[i][COL_LEN - 2] = medium_str;
	}
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
	double entropy = 0.0;
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

//������Ϣ�����
/*
������Ϣ����ȹ�ʽ���������Ϣ�����
*/
double CalGainRatio(vector <vector <string> > remain_state, string attribute){
	double entropy = CalEntropy(remain_state, attribute, "", true);
	double gain = CalGain(remain_state, attribute);
	return gain / entropy;
}

//���ۼ�֦��
/*
C4.5���ñ��ۼ�֦���������֦�����ּ�֦������Ҫ����Ĳ������ݣ�ֻ��Ҫ��ѵ�����ݲ���һ�鼴�ɡ�
���Ƚ�ѵ��������Ϊ�������ݣ��������ɵľ��������о��ߺ�õ����Խ���������Խ������ʵClassType
�Աȣ������ÿ��Ҷ�ڵ�(Ҳ����ClassType�ڵ�)�Ĵ������и�������ȫ������ȷ���У����������и���
Ϊ0������Ҫ���б��ۼ�֦�����д������У���ͨ�����ۼ�֦��ʽ��������ʣ������ۼ�֦��������ʱ��
��ɾ�������������ô������г��ִ�������Ҷ�ڵ����֮��
*/
void PruneTree(Node* root){
	vector<int> realHits(classType.size(), 0); //����ʱ�������и���ClassType�ĸ���
	vector<int> ShouldHits(classType.size(), 0);  //�����У�������Ӧ�����и���ClassType�ĸ���
	vector<int> misHits(classType.size(), 0);   //����ClassType�������еĸ���
	vector<Node*> currentP(classType.size(), 0);  //�������������ӽڵ�

	vector<string> tempRow;
	for(int i = 1; i < originalRecords.size(); i++){
		tempRow = originalRecords[i];
		for(int j = 0; j < classType.size(); j++){ //�������������и���
			if(tempRow[COL_LEN - 1] == classType[j]){
				ShouldHits[j]++;
			}
		}
		//����ʵ�ʲ��������еĸ���
		string hitClassType = JudgeRecord(root, tempRow);
		for(int k = 0; k < classType.size(); k++){ 
			if(hitClassType == classType[k]){
				realHits[k]++;
			}
		}
	}
	//����������еĸ���
	for(int i = 0; i < classType.size(); i++){
		misHits[i] = ShouldHits[i] - realHits[i];
		if(misHits[i] < 0)
			misHits[i] = -misHits[i];
	}
	//���������ݾ���ȷ���У��������֦
	bool allHits = true;
	for(int i = 0; i < misHits.size(); i++){
		if(misHits[i] != 0){
			allHits = false;
			break;
		}
	}
	if(allHits)
		return;

	//�������з�֧�ڵ�
	for(int i = 0; i < currentP.size(); i++){
		currentP[i] = root->childs[i];
	}
	//�����нڵ��ΪҶ�ڵ㣬��ô��û��Ҫ��֦��
	bool isAllLeaves = true;
	for(int i = 0; i < currentP.size(); i++){
		if(currentP[i]->childs.empty())
			continue;
		for(int j = 0; j < currentP[i]->childs.size(); j++)  //���ݹ�ʽ��֪�����к��ӽڵ㣬��misHits����Ϻ��ӽڵ�������һ��
			misHits[i] += currentP[i]->childs[j]->childs.size() / 2;
	}
	//���ݹ�ʽ�ж��Ƿ�Ӧ�ü�֦��
	for(int i = 0; i < misHits.size(); i++){
		double left = misHits[i] + 0.5;
		double temp = misHits[i] * (allRecords.size() - 1 - misHits[i]) / (allRecords.size() - 1);
		double right = misHits[i] + sqrt(temp);
		if(left < right){
			//���м�֦���ó��ִ�������Ҷ�ڵ���������
			currentP[i]->attribute = MostCommonLabel(allRecords);
			currentP[i]->childs.clear(); //�������
		}
	}
}

//�жϲ������ݣ����ز��Խ��ClassType
string JudgeRecord(Node* root, vector<string> aRow){
	string classTypeStr = "";
	int attriNum = GetAttriNum(root->attribute);
	if(attriNum == COL_LEN - 2){ //˵��ΪHeight���ԣ���Ҫ��Heightת����double����
		string str_height = aRow[COL_LEN - 2];
        str_height = str_height.substr(0,str_height.length()-1);
		double tempHeight = atof(str_height.c_str());
		if(tempHeight < threshold1){ //����height��ֵ���뷧ֵ����бȽϣ��ж�Ӧ������ľ�������֧
			classTypeStr = root->childs[0]->attribute;
		}else if(tempHeight >= threshold2){
			classTypeStr = root->childs[2]->attribute;
		}else
			classTypeStr = root->childs[1]->attribute;
			
	}
	return classTypeStr;
}

//���������������������ڵ�
/*
�㷨˼��Ϊ�ݹ齨���������������������ʱ�����ؽڵ㣬������������Ϣ����ȣ�
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

	double max_gainRatio = 0; //�����Ϣ�����
	double temp_gainRatio;
	vector <string>::iterator max_it = remain_attribute.begin();
	vector <string>::iterator it1;  //�����Ϣ�����ָ������Եĵ�����
	for(it1 = remain_attribute.begin(); it1 < remain_attribute.end(); it1++){
		temp_gainRatio = CalGainRatio(remain_state, (*it1));
		if(temp_gainRatio > max_gainRatio) {
			max_gainRatio = temp_gainRatio;
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

