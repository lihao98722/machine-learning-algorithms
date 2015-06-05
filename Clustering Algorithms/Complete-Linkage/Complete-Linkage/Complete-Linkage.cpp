#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<sstream>

using namespace std;

//ÿ��ԭʼ����Ϊһ��Record
class Record{
public:
	string itemName; //������
	int typeIndex;  
	vector<double> distances;  //�����������֮��ľ���(�������Լ�����)
	vector<string> partners;   //��������ͬ���itemName
	Record(string name){
		itemName = name;
	}
	Record(){
		itemName = "";
	}
};

int COL_NUM = 0;  //��ȡ�ļ�ʱ��̬�ж������Ӷ���֤ͨ����
int classTypeNum;  //������
vector<string> AttributeRow; //��ͷ����
vector<Record*> originalRecords;  //ԭʼ��¼(������ͷ)
vector<Record*> currentRecords;  //��ǰ��¼(������ͷ)
double threshold = 0.0;

//���ı��ļ��ж�ȡ����
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

	//��ȡ��ͷ���ݲ���¼����
	while(!strstm.eof()){
		strstm>>item;
		AttributeRow.push_back(item);
	}   
	COL_NUM = AttributeRow.size();

	//һ��ʼ��ÿ����¼���Ƕ�����һ����
	classTypeNum = AttributeRow.size();  

    //��ȡ���ݣ�������originalRecords��
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

//�������ֵ�������ж��Ƿ����
/*
��������֮����ܾ��룬�����������������Ӷ��õ���ֵ��
�㷨�Ż���
��Ϊÿ�����������֮��ľ��붼Ϊ0�����ж������ݱ��ĶԽ���
�ض���Ϊ0�����Բ�������㡣�ұ�������ݱ�Ȼ����ڶԽ��߶Գƣ�
��Ϊ���1�����2֮��ľ��������2�����1֮��ľ����Ȼ��ȡ�
���Դ��㷨ֻ��Ҫ��һ������������ɡ�
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

//ȫ�����㷨
/*
ȫ���ӣ�CompleteLinkage ������ȡ�������о���
��Զ�����������ľ�����Ϊ���������ϵľ��롣

�㷨˼·��
ѭ������currentRecords���ڱ�����ʱ��ֻ����currentRecords����ļ�¼������
�벿�֣���Ϊ����ֵ�ضԽ��߶Գƣ�ԭ��ͼ��㷧ֵʱԭ����ͬ���ҵ�������������Сֵ��
����¼�����������index���Ա㽫��������ϲ���һ���ࡣ�ھ���֮ǰ�������ж������С
ֵ�Ƿ�Ⱥϲ���ķ�ֵС����������㣬��������(��Ϊ����С���붼�����㣬��������Ȼ�޷����㷧ֵ������)
�ڽ��о��࣬���ϲ�ͬ��ʱ�������ϲ�����(��index�ϴ����)���ϲ���index��С�����С�
ʵ�ʲ���Ϊ�������ϲ������itemName���������У��Ҹ��������������������С���룬��������
distances��Ȼ��ɾ�����ϲ�����(�������currentRecords��ɾ�����ҽ������Ӧ��index������ʣ��
���е�distances��ɾ���������ͱ�֤��currentRecords������ΪN * N)��Ȼ�������ʣ��������
���ֱ࣬��ȫ����Ϊһ����޷���������Ϊֹ��

�㷨�Ż���
���㷨������֮�����ھ������Ҳ��ʹ���ݼ���С�Ĺ��̣������ھ����ͬʱ��ͬ������������Ϣ��
���Կ��������������Ϣʱ��ֻ������һ����Ϣ�����Խ������������Ϣ����Ϊ��֮��ľ����ڱ����
�ǶԳƵġ�����������ĺϲ���ʽ������Ҫȥ�ı䱻�ϲ������Ϣ(����Ҫ�ı䣬�㷨ʱ�临�ӶȽ�Ϊn * n��
��Ϊ��������кͱ��ϲ������item�ҳ���Ȼ���޸�������ԣ�����һ������С���롣)�������㷨
�ںϲ����ʱ��ʱ�临�Ӷ�ΪN����������С���ݼ���ʱ�䡣(���ⷽ�����˶�̬�滮��˼��)
*/
void CompleteLinkage(vector<Record*> &currentRecords){ //һ��Ҫ�����ã�����ݹ���̲�û�иı�currentRecords

	//��ֻʣһ�����ͣ��򷵻�
	if(currentRecords.size() == 1)
		return;
	//��ȥ��һ��distance������¼��index
	double minDistance = currentRecords.front()->distances[1];
	int index1 = 0;
	int index2 = 1;

	//�ҳ�������̵�������
	for(int i = 0; i < currentRecords.size(); i++){
		for(int j = i + 1; j < currentRecords[i]->distances.size(); j++){
			//���minDistanceΪ0��˵�����ǵ�����ľ��룬������
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

	//�ж��Ƿ�Ҫ�ϲ���������
	if(minDistance <= threshold){ //С�ڷ�ֵ��ϲ���

		//���������Ϣ(����������֮��ľ�����Ϣ)
		for(int i = 0; i < currentRecords[index1]->distances.size(); i++){
			if(currentRecords[index1]->distances[i] < currentRecords[index2]->distances[i]){
				currentRecords[index1]->distances[i] = currentRecords[index2]->distances[i];
			}
		}

		//��Ҫ���ϲ�����ȫ���ϲ���index1������
		currentRecords[index1]->partners.push_back(currentRecords[index2]->itemName);
		for(int k = 0; k < currentRecords[index2]->partners.size(); k++)
			currentRecords[index1]->partners.push_back(currentRecords[index2]->partners[k]);

		//�����ϲ�����ɾ������ɾ����
		vector<Record*>::iterator it = currentRecords.begin();
		for(int i = 0; i < index2; i++)
			it++;
		currentRecords.erase(it);

		//�����ϲ������͵������Ϣɾ������ɾ����
		for(int i = 0; i < currentRecords.size(); i++){
			currentRecords[i]->distances.erase(currentRecords[i]->distances.begin() + index2);		
		}
		classTypeNum--;
	}else  //�����㷧ֵ������������
		return;
	//�������о���
	CompleteLinkage(currentRecords); 
}

int main(){
	ReadData("table.txt");
	CalThreshold();
	CompleteLinkage(currentRecords);
	cout<<"����ȫ���Ӳ�ξ��࣬";
	cout<<"�ڷ�ֵΪ "<<threshold<<" �������£�";
	cout<<"�ܹ��� "<<classTypeNum<<" ������"<<endl;
	cout<<"���������£�"<<endl;
	for(int i = 0; i < currentRecords.size(); i++){
		cout<<"��"<<i+1<<"��Ϊ�� ";
		cout<<currentRecords[i]->itemName<<" ";
		for(int j = 0; j < currentRecords[i]->partners.size(); j++)
			cout<<currentRecords[i]->partners[j]<<" ";
		cout<<endl;
	}
	return 0;
}