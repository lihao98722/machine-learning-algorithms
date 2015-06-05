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
 
//置信水平取0.95时的卡方表
const double CHI[10]={0.004,0.103,0.352,0.711,1.145,1.635,2.167,
    2.733,3.325,3.94}; 

/*计算卡方值(利用四格表卡方检验公式)*/
template<typename Comparable>
double cal_chi(Comparable **arr,int row,int col){

    vector<Comparable> rowsum(row);
    vector<Comparable> colsum(col);
    Comparable totalsum=static_cast<Comparable>(0);
    for(int i=0;i<row;++i){
        for(int j=0;j<col;++j){
            totalsum+=arr[i][j];
            rowsum[i]+=arr[i][j]; //行的和
            colsum[j]+=arr[i][j];  //列的和
        }
    }
    double rect=0.0;

	//四格表卡方计算公式计算卡方值
    for(int i=0;i<row;++i){
        for(int j=0;j<col;++j){
            double excep=1.0*rowsum[i]*colsum[j]/totalsum;
            if(excep!=0)
                rect+=pow(arr[i][j]-excep,2.0)/excep;
        }
    }
    return rect;
}
 
//保存每个节点的信息(alpha值(表面误差率的增加量)、子树中叶子节点个数、节点所在index)
class BNodeInfo{
public:
    double alphaValue;  //alpha值
    int leavesCount;   //树中叶子节点个数
    int indexNum;    //节点所在index

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
保存统计信息的数据结构。
这是很好的数据结构设计，通过2层Map可以记录所有出现的类别统计，且方便更新。
*/
typedef map<string,int> MAP_ATTR_COUNT;
typedef map<string,MAP_ATTR_COUNT> MAP_ATTR_REST;
typedef vector<MAP_ATTR_REST> VEC_STATI;
 
const int ATTR_NUM=2;      //自变量(除了第一列无用属性和ClassType以外的其余所有列)的维度
const int COL_LEN = 4;   //总列数
vector<string> AttrRows(ATTR_NUM); //保存自变量属性
int classTypesCount;       //因变量的种类数，即ClassType类别数
vector<pair<string,int>> classType;      //把类别、对应的记录数存放在一个数组中
vector<vector<string>> allRecords;      //原始输入数据
int recordCount;       //总的记录数

//记录阀值
double threshold1;
double threshold2;
//自定义对连续变量Height离散后的值
string short_str = "矮小";
string medium_str = "中等";
string tall_str = "高大";

//二叉树节点类
class BNode{
public:
    BNode* parent;       //父节点
    BNode* leftchild;        //左孩子节点
    BNode* rightchild;       //右孩子节点
    string cond;        //分枝条件
    string decision;        //在该节点上作出的类别判定
    
    double precision;      //判定的正确率
    int record_number;     //该节点上涵盖的记录个数
    int size;      //子树包含的叶子节点的数目
    int index;   //层次遍历树，给节点标上序号
    double alpha;  //表面误差率的增加量

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

    //打印节点信息
    void printInfo(){
        cout<<"index:"<<index<<"\tdecisoin:"<<decision<<"\tprecision:"<<precision<<"\tcondition:"<<cond<<"\tsize:"<<size;
        if(parent!=NULL)
            cout<<"\tparent index:"<<parent->index;
        if(leftchild!=NULL)
            cout<<"\tleftchild:"<<leftchild->index<<"\trightchild："<<rightchild->index;
        cout<<endl;
    }

    //打印树
    void printTree(){
        printInfo();
        if(leftchild!=NULL)
            leftchild->printTree();
        if(rightchild!=NULL)
            rightchild->printTree();
    }
};
 
//从文本文件中读出训练集数据
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

    //读取表头数据(不包含ClassType)
    for(int i=0;i<AttrRows.size();++i){
        strstm>>item;
        AttrRows[i]=item;
    }
    //读取训练集数据，保存在allRecords中
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

    //获取ClassType并保存
    map<string,int>::const_iterator itr=catg.begin();
    while(itr!=catg.end()){
        classType.push_back(make_pair(itr->first,itr->second));
        itr++;
    }
    classTypesCount=classType.size();
    return 0;
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
    /*for(int i = 0; i < allRecords.size(); i++){
        originalRecords.push_back(allRecords[i]);
    }*/

    //将Height转化成double型(此时allRecords不含表头)
    for(int i = 0; i < allRecords.size(); i++){
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
                tempVec = allRecords[i];
                allRecords[i] = allRecords[j];
                allRecords[j] = tempVec;
            }
        }
    }

    //将排列好的记录(不含表头)保存到allRecordsSorted中
    for(int i =0; i < allRecords.size(); i++){
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

    //将连续属性Height离散化
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
根据allRecords作出一个统计信息表，里面保存着所有
原始数据并且按类别分开，按类别统计。
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
CART算法的核心思想：
依据某条件作出分枝时，allRecords被分成两部分。所有生成的决策树
一定是二叉树。
*/
void SplitInput(vector<vector<string>> &allRecords,int fitIndex,string cond,vector<vector<string> > &LallRecords,vector<vector<string> > &RallRecords){
    for(int i=0;i<allRecords.size();++i){
        //根据cond即条件来区分，若满足条件，则生成左子树，否则生成右子树。
        if(allRecords[i][fitIndex+1]==cond)
            LallRecords.push_back(allRecords[i]);
        else
            RallRecords.push_back(allRecords[i]);
    }
}

//打印出统计信息(主要是为了调试时跟踪信息变化)
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
 
//生成二叉决策树
/*
CART算法核心：
在每次判断过程中，都是对观察变量进行二分，且进行单变量分割，每次最优划分都是针对单个变量，
然后利用Gini Index(GINI指数)来定义衡量划分的好坏。
通过扫描原始数据生成的信息表中的每一个自变量的每一个取值，判断condition
得到左右孩子节点上ClassType类型及数目并更新数据。接着计算每一个自变量导致的分割
的GINI增益，比较得出所有自变量分割得到GINI增益中最小的一个，并把此时的分割情况记录下来，以便
接下来对数据集进行相应的分割。
且在生成决策二叉树时，通过卡方值来进行剪枝，这是一种前剪枝。
再确定了最小GINI增益及其对应的分割情况后，需计算卡方值后再确定是否真正进行分割。
若卡方值符合分割要求，则在分割的同时，要更新节点的info信息，然后分别创建左右两个分支，
通过判断ClassType出现的次数来决定类别判定的condition，最后递归对左右孩子进行分割。
*/
void MakeDecisionTreeBySplit(BNode *root,vector<vector<string> > &allRecords,vector<pair<string,int>> classType){

    root->record_number=allRecords.size();
    VEC_STATI stati;
	//先根据数据统计类别信息(因为总数据量一直在变化，所以每次都需要更新统计信息)
    Statistic(allRecords,stati);
    //PrintStati(stati);

    /*找到最大化GINI指标的划分*/
    double minGain=1.0;    //最小的GINI增益
    int fitIndex=-1;
    string fitCond;
    vector<pair<string,int>> fitleftclasses;
    vector<pair<string,int>> fitrightclasses;
    int fitleftnumber;
    int fitrightnumber;
    for(int i=0;i<stati.size();++i){    //扫描每一个自变量
        MAP_ATTR_REST::const_iterator itr=stati[i].begin();
        while(itr!=stati[i].end()){        //扫描自变量上的每一个取值
            string condition=itr->first;     //判定的条件，即到达左孩子的条件
            vector<pair<string,int>> leftclasses(classType);     //左孩子节点上类别、及对应的数目
            vector<pair<string,int>> rightclasses(classType);    //右孩子节点上类别、及对应的数目
            int leftnumber=0;      //左孩子节点上包含的类别数目
            int rightnumber=0;     //右孩子节点上包含的类别数目
            for(int j=0;j<leftclasses.size();++j){      //更新类别对应的数目
                string rest=leftclasses[j].first;
                MAP_ATTR_COUNT::const_iterator iter2;
                iter2=(itr->second).find(rest);
                if(iter2==(itr->second).end()){     //没找到
                    leftclasses[j].second=0;
                    rightnumber+=rightclasses[j].second;
                }
                else{      //找到
                    leftclasses[j].second=iter2->second;
                    leftnumber+=leftclasses[j].second;
                    rightclasses[j].second-=(iter2->second);
                    rightnumber+=rightclasses[j].second;
                }
            }

            double gain1=1.0;  //计算GINI增益
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

            if(gain<minGain){  //确定最小的GINI增益及其所确定的分割情况
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
 
    /*计算卡方值，看有没有必要进行分裂*/
    int **arr=new int*[2];
    for(int i=0;i<2;i++)
        arr[i]=new int[classTypesCount];
    for(int i=0;i<classTypesCount;i++){
        arr[0][i]=fitleftclasses[i].second;
        arr[1][i]=fitrightclasses[i].second;
    }
    double chi=cal_chi(arr,2,classTypesCount);

	//若卡方值小于标准值，那么可以判断属性独立，没必要再分裂了
    if(chi<CHI[classTypesCount-2]){     //卡方值接近[K - 1]时的卡方分布值，即[维度 - 1]，所以这里要 classTypesCount-2
        delete []arr[0];   
        delete []arr[1];   
        delete []arr;
        return;      //不需要分裂函数就返回
    }
    delete []arr[0];   
    delete []arr[1];   
    delete []arr;
     
    /*进行分裂*/
    root->cond=AttrRows[fitIndex]+"="+fitCond;     //root的分枝条件
    BNode *travel=root;      //root及其祖先节点的size都要加1
    while(travel!=NULL){
        (travel->size)++;
        travel=travel->parent;
    }
    //创建左右孩子
    BNode *LChild=new BNode(root);       
    BNode *RChild=new BNode(root);
    root->leftchild=LChild;
    root->rightchild=RChild;
    int maxLcount=0;
    int maxRcount=0;
    string Ldicision,Rdicision;
    for(int i=0;i<classTypesCount;++i){     //统计哪种类别出现的最多，从而作出类别判定
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
     
    /*递归对左右孩子进行分裂*/
    vector<vector<string>> LallRecords,RallRecords;
    SplitInput(allRecords,fitIndex,fitCond,LallRecords,RallRecords);
    MakeDecisionTreeBySplit(LChild,LallRecords,fitleftclasses);
    MakeDecisionTreeBySplit(RChild,RallRecords,fitrightclasses);
}
 
/*计算子树的误差代价*/
double CalR2(BNode *root){
    if(root->leftchild==NULL)
        return (1-root->precision)*root->record_number/recordCount;
    else
        return CalR2(root->leftchild)+CalR2(root->rightchild);
}
 
//层次遍历树，给节点标上序号。同时计算alpha，即表面误差率增量。
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
            //计算表面误差率的增量
            double r1=(1-n->precision)*n->record_number/recordCount;     //节点的误差代价
            double r2=CalR2(n);
            n->alpha=(r1-r2)/(n->size-1);
            pq.push(BNodeInfo(n->alpha,n->size,n->index));
        }
    }
}
 
/*
Cost-Complexity Pruning(根据代价复杂度剪枝)
这是一种后剪枝方式，需要测试集。(在本程序成用训练集当做测试集进行测试)
核心思想：
按照计算公式，对于分类回归树中的每一个非叶子节点计算它的表面误差率增益值α，
然后找到α值最小的非叶子节点，令其左右孩子为NULL。当多个非叶子节点的α值同时
达到最小时，取最大的进行剪枝。
所以采用这种剪枝方法，必定会剪枝掉α值最小的非叶子节点。
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
            cout<<"将要剪掉"<<i<<"的左右子树"<<endl;
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

//利用测试数据，对生成的决策树进行测试，得出命中率
/*
在本例中，由于没有测试数据，所以就复用训练集数据。
首先，需要将连续型变量Height离散化，接着与condition比较，
知道走到叶子节点，即得出其ClassType为止。然后计算其
置信度。
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
    istringstream strstm(line);     //跳过表头
    map<string,string> independent; //自变量，即分类的依据
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

                //将连续属性Height离散化
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
                cout<<(trav->decision)<<"\t置信度:"<<(trav->precision)<<endl;;
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
    VEC_STATI stati;        //最原始的统计
    Statistic(allRecords,stati);
    BNode *root=new BNode();
    MakeDecisionTreeBySplit(root,allRecords,classType);     //分裂根节点
    priority_queue<BNodeInfo> pq;
    CalAlphaValue(root,pq);
    root->printTree();

    int judgeCount = root->size - 1;  
    cout<<"剪枝前使用该决策树最多进行"<<judgeCount<<"次条件判断"<<endl;

    //DoTest(inputFile,root); //测试数据并打印出置信度
    CCPruning(root,pq);
    cout<<"剪枝后使用该决策树最多进行"<<root->size-1<<"次条件判断"<<endl;
    DoTest(inputFile,root);

	//PrintStati(stati);
    return 0;
}