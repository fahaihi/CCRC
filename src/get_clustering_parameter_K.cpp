/* ****************************************************
date:2022/09/26
authors:SH
describe: A cpp file for getting clustering parameter K
g++ get_clustering_parameter_K.cpp -fopenmp -std=c++11 -O3 -o get_clustering_parameter_K.out
./get_clustering_parameter_K.out [path] [thread_num] [C_ram]
****************************************************** */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <omp.h>
#include <dirent.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

using namespace std;
string file_path;
string out_path;
uint64_t total_num = 0;     // m * v
uint64_t K = 0;
uint64_t R_size = 0;
int C_ram = 20;               // GB
int thread_num = 8;
int files_num = 0;
double alpha = 1.05;
int readlen = 0;


vector<string>files_name_list;

void getFiles(std::string path, std::vector<std::string> &files_name_list) {
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
            idx = strFile.find(".num");
            if(idx != string::npos)
                files_name_list.push_back(strFile);
        } else {
            continue;
        }
    }
    closedir(dir);
}

int main(int argc, char** argv){
    file_path = std::string(argv[1]);
    thread_num = atoi(argv[2]);
    C_ram = atoi(argv[3]);
    out_path = file_path + "/Cluster.config";



    // 1. get files name and caculation total_num
    getFiles(file_path, files_name_list);
    files_num = files_name_list.size();

    vector<int>readlen_list;
    #pragma omp parallel for num_threads(thread_num)
    for(int i=0; i<files_name_list.size(); i++){
        ifstream input_file;
        input_file.open(files_name_list[i], std::ios::in);
        string line1, line2, line3;
        getline(input_file, line1);
        getline(input_file, line2);
        getline(input_file, line3);
        #pragma omp critical
        {
            // global data plus
            total_num = total_num + strtol(line1.c_str(),nullptr,10);
            readlen_list.push_back(atoi(line3.c_str()));
        }

        input_file.close();
    }

    // 2.chick reads lenth for each file.
    for(int i=1; i<readlen_list.size(); i++){
        if(readlen_list[0]!=readlen_list[i]){
            cout << "  CCRC support same length reads, and not more than 256bp!" << endl;
            return 0;
        }
    }
    readlen = readlen_list[0];
    R_size = (readlen*2)+1.2*(32+16+8); // bits
    total_num = total_num * 2;


    K = ceil(alpha*total_num*R_size/8/1024/1024/1024/C_ram);




    cout << "  files_num: " << files_num << endl;
    cout << "  C_ram: " << C_ram << endl;
    cout << "  m*v: " << total_num << endl;
    cout << "  readLen: " << readlen << endl;
    cout << "  K : " << K << endl;

    ofstream out_stream;
    out_stream.open(out_path);
    out_stream << readlen << "\n" << K << "\n";
    out_stream.close();
    return 0;
}