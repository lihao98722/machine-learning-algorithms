#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<map>
#include<list>
#include<set>
#include<queue>
#include<utility>
#include<vector>
#include<cmath>
 
using namespace std;
 
//����ˮƽȡ0.95ʱ�Ŀ�����
const double CHI[10]={0.004,0.103,0.352,0.711,1.145,1.635,2.167,
    2.733,3.325,3.94}; 

/*���㿨��ֵ(�����ĸ�������鹫ʽ)*/
template<typename Comparable>
double cal_chi(Comparable **arr,int row,int col){

    vector<Comparable> rowsum(row);
    vector<Comparable> colsum(col);
    Comparable totalsum=static_cast<Comparable>(0);
    for(int i=0;i<row;++i){
        for(int j=0;j<col;++j){
            totalsum+=arr[i][j];
            rowsum[i]+=arr[i][j]; //�еĺ�
            colsum[j]+=arr[i][j];  //�еĺ�
        }
    }
    double rect=0.0;

	//�ĸ�������㹫ʽ���㿨��ֵ
    for(int i=0;i<row;++i){
        for(int j=0;j<col;++j){
            double excep=1.0*rowsum[i]*colsum[j]/totalsum;
            if(excep!=0)
                rect+=pow(arr[i][j]-excep,2.0)/excep;
        }
    }
    return rect;
}
 
//����ÿ���ڵ����Ϣ(alphaֵ(��������ʵ�������)��������Ҷ�ӽڵ�������ڵ�����index)
class BNodeInfo{
public:
    double alphaValue;  //alphaֵ
    int leavesCount;   //����Ҷ�ӽڵ����
    int indexNum;    //�ڵ�����index

    BNodeInfo(){
        alphaValue=0.0;
        leavesCount=0;
        indexNum=0;
    }

    BNodeInfo(double f,int s,int t):alphaValue(f),leavesCount(s),indexNum(t){}
    bool operator < (const BNodeInfo &obj) const{
        int cmp=this->alphaValue-obj.alphaValue;
        if(cmp>0)
            return false;
        else if(cmp<0)
            return true;
        else{
            cmp=obj.leavesCount-this->leavesCount;
            if(cmp<0)
                return true;
            else
                return false;
        }
    }
};


/*
����ͳ����Ϣ�����ݽṹ��
���Ǻܺõ����ݽṹ��ƣ�ͨ��2��Map���Լ�¼���г��ֵ����ͳ�ƣ��ҷ�����¡�
*/
typedef map<string,int> MAP_ATTR_COUNT;
typedef map<string,MAP_ATTR_COUNT> MAP_ATTR_REST;
typedef vector<MAP_ATTR_REST> VEC_STATI;
 
const int ATTR_NUM=2;      //�Ա���(���˵�һ���������Ժ�ClassType���������������)��ά��
const int COL_LEN = 4;   //������
vector<string> AttrRows(ATTR_NUM); //�����Ա�������
int classTypesCount;       //�����������������ClassType�����
vector<pair<string,int>> classType;      //����𡢶�Ӧ�ļ�¼�������һ��������
vector<vector<string>> allRecords;      //ԭʼ��������
int recordCount;       //�ܵļ�¼��

//��¼��ֵ
double threshold1;
double threshold2;
//�Զ������������Height��ɢ���ֵ
string short_str = "��С";
string medium_str = "�е�";
string tall_str = "�ߴ�";

//�������ڵ���
class BNode{
public:
    BNode* parent;       //���ڵ�
    BNode* leftchild;        //���ӽڵ�
    BNode* rightchild;       //�Һ��ӽڵ�
    string cond;        //��֦����
    string decision;        //�ڸýڵ�������������ж�
    
    double precision;      //�ж�����ȷ��
    int record_number;     //�ýڵ��Ϻ��ǵļ�¼����
    int size;      //����������Ҷ�ӽڵ����Ŀ
    int index;   //��α����������ڵ�������
    double alpha;  //��������ʵ�������

    BNode(){
        parent=NULL;
        leftchild=NULL;
        rightchild=NULL;
        precision=0.0;
        record_number=0;
        size=1;
        index=0;
        alpha=1.0;
    }

    BNode(BNode* p){
        parent=p;
        leftchild=NULL;
        rightchild=NULL;
        precision=0.0;
        record_number=0;
        size=1;
        index=0;
        alpha=1.0;
    }

    BNode(BNode* p,string c,string d):cond(c),decision(d){
        parent=p;
        leftchild=NULL;
        rightchild=NULL;
        precision=0.0;
        record_number=0;
        size=1;
        index=0;
        alpha=1.0;
    }

    //��ӡ�ڵ���Ϣ
    void printInfo(){
        cout<<"index:"<<index<<"\tdecisoin:"<<decision<<"\tprecision:"<<precision<<"\tcondition:"<<cond<<"\tsize:"<<size;
        if(parent!=NULL)
            cout<<"\tparent index:"<<parent->index;
        if(leftchild!=NULL)
            cout<<"\tleftchild:"<<leftchild->index<<"\trightchild��"<<rightchild->index;
        cout<<endl;
    }

    //��ӡ��
    void printTree(){
        printInfo();
        if(leftchild!=NULL)
            leftchild->printTree();
        if(rightchild!=NULL)
            rightchild->printTree();
    }
};
 
//���ı��ļ��ж���ѵ��������
int ReadData(string filename){
    ifstream ifs(filename.c_str());
    if(!ifs){
        cerr<<"open inputfile failed!"<<endl;
        return -1;
    }
    map<string,int> catg;
    string line;
    getline(ifs,line);
    string item;
    istringstream strstm(line);
    strstm>>item;

    //��ȡ��ͷ����(������ClassType)
    for(int i=0;i<AttrRows.size();++i){
        strstm>>item;
        AttrRows[i]=item;
    }
    //��ȡѵ�������ݣ�������allRecords��
    while(getline(ifs,line)){
        vector<string> conts(ATTR_NUM+2);
        istringstream strstm(line);
        for(int i=0;i<conts.size();++i){
            strstm>>item;
            conts[i]=item;
            if(i==conts.size()-1)
                catg[item]++;
        }
        allRecords.push_back(conts);
    }
    recordCount=allRecords.size();
    ifs.close();

    //��ȡClassType������
    map<string,int>::const_iterator itr=catg.begin();
    while(itr!=catg.end()){
        classType.push_back(make_pair(itr->first,itr->second));
        itr++;
    }
    classTypesCount=classType.size();
    return 0;
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
    /*for(int i = 0; i < allRecords.size(); i++){
        originalRecords.push_back(allRecords[i]);
    }*/

    //��Heightת����double��(��ʱallRecords������ͷ)
    for(int i = 0; i < allRecords.size(); i++){
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
                tempVec = allRecords[i];
                allRecords[i] = allRecords[j];
                allRecords[j] = tempVec;
            }
        }
    }

    //�����кõļ�¼(������ͷ)���浽allRecordsSorted��
    for(int i =0; i < allRecords.size(); i++){
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

    //����������Height��ɢ��
    for(int i =0; i < allRecords.size(); i++){
        if(i < point1)
            allRecords[i][COL_LEN - 2] = short_str;
        else if(i >= point2)
            allRecords[i][COL_LEN - 2] = tall_str;
        else
            allRecords[i][COL_LEN - 2] = medium_str;
    }
}
 

/*
����allRecords����һ��ͳ����Ϣ�����汣��������
ԭʼ���ݲ��Ұ����ֿ��������ͳ�ơ�
*/
void Statistic(vector<vector<string>> &allRecords,VEC_STATI &stati){
    for(int i=1;i<ATTR_NUM+1;++i){
        MAP_ATTR_REST attr_rest;
        for(int j=0;j<allRecords.size();++j){
            string attr_value=allRecords[j][i];
            string rest=allRecords[j][ATTR_NUM+1];
            MAP_ATTR_REST::iterator itr=attr_rest.find(attr_value);
            if(itr==attr_rest.end()){
                MAP_ATTR_COUNT rest_count;
                rest_count[rest]=1;
                attr_rest[attr_value]=rest_count;
            }
            else{
                MAP_ATTR_COUNT::iterator iter=(itr->second).find(rest);
                if(iter==(itr->second).end()){
                    (itr->second).insert(make_pair(rest,1));
                }
                else{
                    iter->second+=1;
                }
            }
        }
        stati.push_back(attr_rest);
    }
}
 
/*
CART�㷨�ĺ���˼�룺
����ĳ����������֦ʱ��allRecords���ֳ������֡��������ɵľ�����
һ���Ƕ�������
*/
void SplitInput(vector<vector<string>> &allRecords,int fitIndex,string cond,vector<vector<string> > &LallRecords,vector<vector<string> > &RallRecords){
    for(int i=0;i<allRecords.size();++i){
        //����cond�����������֣�������������������������������������������
        if(allRecords[i][fitIndex+1]==cond)
            LallRecords.push_back(allRecords[i]);
        else
            RallRecords.push_back(allRecords[i]);
    }
}

//��ӡ��ͳ����Ϣ(��Ҫ��Ϊ�˵���ʱ������Ϣ�仯)
void PrintStati(VEC_STATI &stati){
    for(int i=0;i<stati.size();i++){
        MAP_ATTR_REST::const_iterator
        itr=stati[i].begin();
        while(itr!=stati[i].end()){
            cout<<itr->first;
            MAP_ATTR_COUNT::const_iterator
            iter=(itr->second).begin();
            while(iter!=(itr->second).end()){
                cout<<"\t"<<iter->first<<"\t"<<iter->second;
                iter++;
            }
            itr++;
            cout<<endl;
        }
        cout<<endl;
    }
}
 
//���ɶ��������
/*
CART�㷨���ģ�
��ÿ���жϹ����У����ǶԹ۲�������ж��֣��ҽ��е������ָÿ�����Ż��ֶ�����Ե���������
Ȼ������Gini Index(GINIָ��)������������ֵĺû���
ͨ��ɨ��ԭʼ�������ɵ���Ϣ���е�ÿһ���Ա�����ÿһ��ȡֵ���ж�condition
�õ����Һ��ӽڵ���ClassType���ͼ���Ŀ���������ݡ����ż���ÿһ���Ա������µķָ�
��GINI���棬�Ƚϵó������Ա����ָ�õ�GINI��������С��һ�������Ѵ�ʱ�ķָ������¼�������Ա�
�����������ݼ�������Ӧ�ķָ
�������ɾ��߶�����ʱ��ͨ������ֵ�����м�֦������һ��ǰ��֦��
��ȷ������СGINI���漰���Ӧ�ķָ����������㿨��ֵ����ȷ���Ƿ��������зָ
������ֵ���Ϸָ�Ҫ�����ڷָ��ͬʱ��Ҫ���½ڵ��info��Ϣ��Ȼ��ֱ𴴽�����������֧��
ͨ���ж�ClassType���ֵĴ�������������ж���condition�����ݹ�����Һ��ӽ��зָ
*/
void MakeDecisionTreeBySplit(BNode *root,vector<vector<string> > &allRecords,vector<pair<string,int>> classType){

    root->record_number=allRecords.size();
    VEC_STATI stati;
	//�ȸ�������ͳ�������Ϣ(��Ϊ��������һֱ�ڱ仯������ÿ�ζ���Ҫ����ͳ����Ϣ)
    Statistic(allRecords,stati);
    //PrintStati(stati);

    /*�ҵ����GINIָ��Ļ���*/
    double minGain=1.0;    //��С��GINI����
    int fitIndex=-1;
    string fitCond;
    vector<pair<string,int>> fitleftclasses;
    vector<pair<string,int>> fitrightclasses;
    int fitleftnumber;
    int fitrightnumber;
    for(int i=0;i<stati.size();++i){    //ɨ��ÿһ���Ա���
        MAP_ATTR_REST::const_iterator itr=stati[i].begin();
        while(itr!=stati[i].end()){        //ɨ���Ա����ϵ�ÿһ��ȡֵ
            string condition=itr->first;     //�ж������������������ӵ�����
            vector<pair<string,int>> leftclasses(classType);     //���ӽڵ�����𡢼���Ӧ����Ŀ
            vector<pair<string,int>> rightclasses(classType);    //�Һ��ӽڵ�����𡢼���Ӧ����Ŀ
            int leftnumber=0;      //���ӽڵ��ϰ����������Ŀ
            int rightnumber=0;     //�Һ��ӽڵ��ϰ����������Ŀ
            for(int j=0;j<leftclasses.size();++j){      //��������Ӧ����Ŀ
                string rest=leftclasses[j].first;
                MAP_ATTR_COUNT::const_iterator iter2;
                iter2=(itr->second).find(rest);
                if(iter2==(itr->second).end()){     //û�ҵ�
                    leftclasses[j].second=0;
                    rightnumber+=rightclasses[j].second;
                }
                else{      //�ҵ�
                    leftclasses[j].second=iter2->second;
                    leftnumber+=leftclasses[j].second;
                    rightclasses[j].second-=(iter2->second);
                    rightnumber+=rightclasses[j].second;
                }
            }

            double gain1=1.0;  //����GINI����
            double gain2=1.0;
            if(leftnumber==0)
                gain1=0.0;
            else
                for(int j=0;j<leftclasses.size();++j)       
                    gain1-=pow(1.0*leftclasses[j].second/leftnumber,2.0);
            if(rightnumber==0)
                gain2=0.0;
            else
                for(int j=0;j<rightclasses.size();++j)
                    gain2-=pow(1.0*rightclasses[j].second/rightnumber,2.0);
            double gain=1.0*leftnumber/(leftnumber+rightnumber)*gain1+1.0*rightnumber/(leftnumber+rightnumber)*gain2;

            if(gain<minGain){  //ȷ����С��GINI���漰����ȷ���ķָ����
                fitIndex=i;
                fitCond=condition;
                fitleftclasses=leftclasses;
                fitrightclasses=rightclasses;
                fitleftnumber=leftnumber;
                fitrightnumber=rightnumber;
                minGain=gain;
            }
            itr++;
        }
    }
 
    /*���㿨��ֵ������û�б�Ҫ���з���*/
    int **arr=new int*[2];
    for(int i=0;i<2;i++)
        arr[i]=new int[classTypesCount];
    for(int i=0;i<classTypesCount;i++){
        arr[0][i]=fitleftclasses[i].second;
        arr[1][i]=fitrightclasses[i].second;
    }
    double chi=cal_chi(arr,2,classTypesCount);

	//������ֵС�ڱ�׼ֵ����ô�����ж����Զ�����û��Ҫ�ٷ�����
    if(chi<CHI[classTypesCount-2]){     //����ֵ�ӽ�[K - 1]ʱ�Ŀ����ֲ�ֵ����[ά�� - 1]����������Ҫ classTypesCount-2
        delete []arr[0];   
        delete []arr[1];   
        delete []arr;
        return;      //����Ҫ���Ѻ����ͷ���
    }
    delete []arr[0];   
    delete []arr[1];   
    delete []arr;
     
    /*���з���*/
    root->cond=AttrRows[fitIndex]+"="+fitCond;     //root�ķ�֦����
    BNode *travel=root;      //root�������Ƚڵ��size��Ҫ��1
    while(travel!=NULL){
        (travel->size)++;
        travel=travel->parent;
    }
    //�������Һ���
    BNode *LChild=new BNode(root);       
    BNode *RChild=new BNode(root);
    root->leftchild=LChild;
    root->rightchild=RChild;
    int maxLcount=0;
    int maxRcount=0;
    string Ldicision,Rdicision;
    for(int i=0;i<classTypesCount;++i){     //ͳ�����������ֵ���࣬�Ӷ���������ж�
        if(fitleftclasses[i].second>maxLcount){
            maxLcount=fitleftclasses[i].second;
            Ldicision=fitleftclasses[i].first;
        }
        if(fitrightclasses[i].second>maxRcount){
            maxRcount=fitrightclasses[i].second;
            Rdicision=fitrightclasses[i].first;
        }
    }
    LChild->decision=Ldicision;
    RChild->decision=Rdicision;
    LChild->precision=1.0*maxLcount/fitleftnumber;
    RChild->precision=1.0*maxRcount/fitrightnumber;
     
    /*�ݹ�����Һ��ӽ��з���*/
    vector<vector<string>> LallRecords,RallRecords;
    SplitInput(allRecords,fitIndex,fitCond,LallRecords,RallRecords);
    MakeDecisionTreeBySplit(LChild,LallRecords,fitleftclasses);
    MakeDecisionTreeBySplit(RChild,RallRecords,fitrightclasses);
}
 
/*����������������*/
double CalR2(BNode *root){
    if(root->leftchild==NULL)
        return (1-root->precision)*root->record_number/recordCount;
    else
        return CalR2(root->leftchild)+CalR2(root->rightchild);
}
 
//��α����������ڵ������š�ͬʱ����alpha�������������������
void CalAlphaValue(BNode *root,priority_queue<BNodeInfo> &pq){
    int i=1;
    queue<BNode*> que;
    que.push(root);
    while(!que.empty()){
        BNode* n=que.front();
        que.pop();
        n->index=i++;
        if(n->leftchild!=NULL){
            que.push(n->leftchild);
            que.push(n->rightchild);
            //�����������ʵ�����
            double r1=(1-n->precision)*n->record_number/recordCount;     //�ڵ��������
            double r2=CalR2(n);
            n->alpha=(r1-r2)/(n->size-1);
            pq.push(BNodeInfo(n->alpha,n->size,n->index));
        }
    }
}
 
/*
Cost-Complexity Pruning(���ݴ��۸��Ӷȼ�֦)
����һ�ֺ��֦��ʽ����Ҫ���Լ���(�ڱ��������ѵ�����������Լ����в���)
����˼�룺
���ռ��㹫ʽ�����ڷ���ع����е�ÿһ����Ҷ�ӽڵ�������ı������������ֵ����
Ȼ���ҵ���ֵ��С�ķ�Ҷ�ӽڵ㣬�������Һ���ΪNULL���������Ҷ�ӽڵ�Ħ�ֵͬʱ
�ﵽ��Сʱ��ȡ���Ľ��м�֦��
���Բ������ּ�֦�������ض����֦����ֵ��С�ķ�Ҷ�ӽڵ㡣
*/
void CCPruning(BNode *root,priority_queue<BNodeInfo> &pq){
    BNodeInfo triple=pq.top();
    int i=triple.indexNum;
    queue<BNode*> que;
    que.push(root);
    while(!que.empty()){
        BNode* n=que.front();
        que.pop();
        if(n->index == i){
            cout<<"��Ҫ����"<<i<<"����������"<<endl;
            n->leftchild=NULL;
            n->rightchild=NULL;
            int s=n->size-1;
            BNode *trav=n;
            while(trav!=NULL){
                trav->size-=s;
                trav=trav->parent;
            }
            break;
        }
        else if(n->leftchild!=NULL){
            que.push(n->leftchild);
            que.push(n->rightchild);
        }
    }
}

//���ò������ݣ������ɵľ��������в��ԣ��ó�������
/*
�ڱ����У�����û�в������ݣ����Ծ͸���ѵ�������ݡ�
���ȣ���Ҫ�������ͱ���Height��ɢ����������condition�Ƚϣ�
֪���ߵ�Ҷ�ӽڵ㣬���ó���ClassTypeΪֹ��Ȼ�������
���Ŷȡ�
*/ 
void DoTest(string filename,BNode *root){
    ifstream ifs(filename.c_str());
    if(!ifs){
        cerr<<"open inputfile failed!"<<endl;
        return;
    }
    string line;
    getline(ifs,line);
    string item;
    istringstream strstm(line);     //������ͷ
    map<string,string> independent; //�Ա����������������
    while(getline(ifs,line)){
        istringstream strstm(line);
        //strstm.str(line);
        strstm>>item;
        cout<<item<<"\t";
        for(int i=0;i<ATTR_NUM;++i){
            strstm>>item;
            if(i == ATTR_NUM - 1){
                string str_height = item;
                str_height = str_height.substr(0,str_height.length()-1);
                double tempHeight = atof(str_height.c_str());

                //����������Height��ɢ��
                if(tempHeight < threshold1)
                    item = short_str;
                else if(tempHeight >= threshold2)
                    item = tall_str;
                else
                    item = medium_str;
            }
            independent[AttrRows[i]]=item;
        }
        BNode *trav=root;
        while(trav!=NULL){
            if(trav->leftchild==NULL){
                cout<<(trav->decision)<<"\t���Ŷ�:"<<(trav->precision)<<endl;;
                break;
            }
            string cond=trav->cond;
            string::size_type pos=cond.find("=");
            string pre=cond.substr(0,pos);
            string post=cond.substr(pos+1);
            if(independent[pre]==post)
                trav=trav->leftchild;
            else
                trav=trav->rightchild;
        }
    }
    ifs.close();
}

int main(){
    string inputFile="test.txt";
    ReadData(inputFile);
    CalThresholds();
    VEC_STATI stati;        //��ԭʼ��ͳ��
    Statistic(allRecords,stati);
    BNode *root=new BNode();
    MakeDecisionTreeBySplit(root,allRecords,classType);     //���Ѹ��ڵ�
    priority_queue<BNodeInfo> pq;
    CalAlphaValue(root,pq);
    root->printTree();

    int judgeCount = root->size - 1;  
    cout<<"��֦ǰʹ�øþ�����������"<<judgeCount<<"�������ж�"<<endl;

    //DoTest(inputFile,root); //�������ݲ���ӡ�����Ŷ�
    CCPruning(root,pq);
    cout<<"��֦��ʹ�øþ�����������"<<root->size-1<<"�������ж�"<<endl;
    DoTest(inputFile,root);

	//PrintStati(stati);
    return 0;
}