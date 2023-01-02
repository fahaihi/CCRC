/* ***************************************************************
date:2022/09/25
update:2022/12/10
author:SH
describe: A cpp file for simlarity FastQ Files Clustering
*************************************************************** */
#include<iostream>
#include<vector>
#include<string>
#include<unordered_map>
#include<set>
#include<stdio.h>
#include<dirent.h>
#include<math.h>
#include<time.h>
#include<string.h>
#include<omp.h>
#include<algorithm>
#include<fstream>
#include<ctime>
using namespace std;

std::vector<std::string>filenamelist;

typedef struct Node{
    vector<int>features;    // file feature vector
    std::string name;       // file name
    int id;                 // file id
}node;

typedef struct simNode{
}simNode;
void getFiles(std::string path, std::vector<std::string> &files);
float DiceIndex(vector<int>nums1, vector<int>nums2);

int main(int argc, char** argv){

    cout << "   get parameters [dir] [thread-num] [K]" << endl;
    std::string basedir = std::string(argv[1]);
    int thread_num = atoi(argv[2]);
    int K = atoi(argv[3]);

    cout << "   get FastQ files vectors and names" << endl;
    getFiles(basedir, filenamelist);
    std::vector<Node*>data;
    #pragma omp parallel for num_threads(thread_num)
    for(int i=0; i<filenamelist.size();i++){
        std::vector<int>tempVector;
        std::string file_name = filenamelist[i];
        ifstream inputFile;
        inputFile.open(file_name, std::ios::in);
        if(!inputFile) std::cout << "   Error" << std::endl;;
        string str_num;
        while(getline(inputFile, str_num)){
            tempVector.push_back(stoi(str_num));
        }
        inputFile.close();
        Node *n = new Node();
        n->name = filenamelist[i];
        n->id = i;
        n->features = tempVector;

        #pragma omp critical
        {
            data.push_back(n);
        }

    }

    cout << "   begin to clustering..." << endl;
    int N = data.size();              // total files number
    int C = N/K;                      // how many files in a cluster
    if(C==1) {cout << " the number of N/K must >= 2!" << endl; return 2;}

    cout << "define a kind of data structure for calculation simlarity for each pair files." << endl;
    typedef struct resNode{
        std::string setA;    // file A
        std::string setB;    // file B
        float simValue;      // sim for A and B
        int setA_num;
        int setB_num;
    }resnode;

    cout << "  get simlarity value sets s." << endl;
    vector<resNode*>resMatrix;
    for(int i=0; i<N; i++){
    vector<int>setA = data[i]->features;
        #pragma omp parallel for num_threads(thread_num)
        for(int j=0; j<i; j++){
            vector<int>setB = data[j]->features;
            if(data[i]->id != data[j]->id){
                float res=DiceIndex(setA,setB);
                resNode *rs = new resNode();
                rs->setA = data[i]->name;
                rs->setB = data[j]->name;
                rs->simValue = res;
                rs->setA_num = data[i]->features.size();
                rs->setB_num = data[j]->features.size();
                resMatrix.push_back(rs);
            }
        }
    }

    cout << "   rearrange s in descending order" << endl;
    vector<resNode*>sortedMatrix;
    int flag = resMatrix.size();
    while(flag!=0){
        float maxValue = -1;
        int index = 0;
        for(int i=0; i<flag;i++){
            if(resMatrix[i]->simValue >= maxValue) {
                maxValue = resMatrix[i]->simValue;
                index = i;
            }

        }
        sortedMatrix.push_back(resMatrix[index]);
        resMatrix.erase(resMatrix.begin()+index);
        flag--;
    }

    cout << "   for clustering" << endl;
    int index = 0;
    while(index<K){
        int read_cluster_sum_num = 0;
        index ++;
        set<std::string>temp;
        vector<int>temp_int;
        temp.insert(sortedMatrix[0]->setA);
        temp_int.push_back(sortedMatrix[0]->setA_num);
        read_cluster_sum_num = read_cluster_sum_num + sortedMatrix[0]->setA_num;
        temp.insert(sortedMatrix[0]->setB);
        temp_int.push_back(sortedMatrix[0]->setB_num);
        read_cluster_sum_num = read_cluster_sum_num + sortedMatrix[0]->setB_num;
        for(int i=0; i<sortedMatrix.size();i++){
            if((temp.size()==C) && (index<K)){
                string cluster_file_name_save = basedir + "/Cluster_" + to_string(index) + ".info";
                cout << cluster_file_name_save << endl;
                std::ofstream cluster_file_stream;
                cluster_file_stream.open(cluster_file_name_save);
                cluster_file_stream << read_cluster_sum_num << "\n";
                int aaa=0;
                for(set<string>::iterator j=temp.begin();j!=temp.end();j++){
                    string ff_name = *j;
                    cluster_file_stream << ff_name.substr(0, ff_name.find(".vec")) +".dna" << "\n";
                    cluster_file_stream << temp_int[aaa] << "\n";
                    cout << ff_name.substr(0, ff_name.find(".vec")) +".dna" << endl << temp_int[aaa] << endl;
                    aaa++;
                }
                cout <<"______________"<< aaa << "____________" << endl;
                cluster_file_stream.close();
                break;
            }

            auto it =  temp.find(sortedMatrix[i]->setA);
            if(it!=temp.end()){
                temp.insert(sortedMatrix[i]->setB);
                temp_int.push_back(sortedMatrix[0]->setB_num);
                read_cluster_sum_num = read_cluster_sum_num + sortedMatrix[i]->setB_num;
                continue;
            }
            it = temp.find(sortedMatrix[i]->setB);
            if(it!=temp.end()){
                temp.insert(sortedMatrix[i]->setA);
                temp_int.push_back(sortedMatrix[0]->setA_num);
                read_cluster_sum_num = read_cluster_sum_num + sortedMatrix[i]->setA_num;
                continue;
            }
        }
        // delate clusted files
        int i=0;
        while(i!=sortedMatrix.size()){
            auto it1 = temp.find(sortedMatrix[i]->setA);
            auto it2 = temp.find(sortedMatrix[i]->setB);
            if(it1!=temp.end() || it2!=temp.end()){
                sortedMatrix.erase(sortedMatrix.begin()+i);
            }
            else i++;
        }
        if((sortedMatrix.size()==0) && (index==K)){
                string cluster_file_name_save = basedir + "/Cluster_" + to_string(index) + ".info";
                std::ofstream cluster_file_stream;
                cluster_file_stream.open(cluster_file_name_save);
                cluster_file_stream << read_cluster_sum_num << "\n";
                int aaa=0;
                for(set<string>::iterator j=temp.begin();j!=temp.end();j++){
                    string ff_name = *j;
                    cluster_file_stream << ff_name.substr(0, ff_name.find(".vec")) +".dna" << "\n";
                    cluster_file_stream << temp_int[aaa] << "\n";
                    aaa++;
                }
                cluster_file_stream.close();
                break;
            }

    }


}


void getFiles(std::string path, std::vector<std::string> &files) {
    DIR *dir;
    struct dirent *ptr;
    if ((dir = opendir(path.c_str())) == NULL) {
        perror("    open dir error...");
        return;
    }

    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    ///current dir OR parrent dir
            continue;
        else if (ptr->d_type == 8)    ///file
        {
            std::string strFile;
            strFile = path;
            strFile += "/";
            strFile += ptr->d_name;
            string::size_type idx;
            idx = strFile.find(".vec");
            if(idx != string::npos)
                files.push_back(strFile);
        } else {
            continue;
        }
    }
    closedir(dir);
}
float DiceIndex(vector<int>nums1, vector<int>nums2){
    float A = nums1.size() + nums2.size();
    set<int>s1(nums1.begin(),nums1.end());
    nums1.resize(0);
    for(int i=0;i<nums2.size();i++){
        auto it=s1.find(nums2[i]);
        if(it!=s1.end()){
            nums1.push_back(nums2[i]);
            s1.erase(nums2[i]);
        }
    }
    return (2 * nums1.size())/A;
}