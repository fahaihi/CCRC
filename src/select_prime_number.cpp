/* *****************************************************
date:2022/09/25
author:SH
describe: A C++ file to calculate the max prime numer
g++ select_prime_number.cpp -O3 -std=c++11 -o select_prime_number.out
./select_prime_number.out [simlar replacement num]
*************************************************** */
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdint.h>
#include <sstream>
using namespace std;

uint64_t get_numer = 10;
float expand_size = 1.3;

int main(int argc, char** argv){
    string get_numer_str = std::string(argv[1]);
    get_numer = strtol(get_numer_str.c_str(),nullptr,10);
    if(get_numer >= 4294967296/expand_size) {
        cout << 4294967295 << endl;
        return 0;
    }
    else if(get_numer < 15485867/expand_size) {
        cout << 15485867 << endl;
        return 0;
    }
    else{
        get_numer = get_numer * expand_size;
        std::ifstream myfile("./prime_numer_list.txt", std::ifstream::in);
        string line;
        string prime_file_name;
        int i = 0;
        int j = 0;
        uint32_t num = 0;
        while(std::getline(myfile, line)){
            switch(i){
                case 0:{ //cout << "beg : " << num << endl;
                    num = strtol(line.c_str(),nullptr,10);
                    if(num>get_numer){
                        string _end;
                        getline(myfile, _end);

                        prime_file_name = line + "_" + _end + ".txt";
                        //cout << prime_file_name << endl;
                        goto label;
                    }

                }break;
                case 1:{ //cout << "end : " << num << endl;
                    num = strtol(line.c_str(),nullptr,10);

                }break;
            }
            i=(i+1)%2;j++;
        }
        label:
        myfile.close();

        // reading file and get the number
        ifstream primefile("./prime_numer/" + prime_file_name, std::ifstream::in);
        string PN;
        while(primefile.peek() != EOF) getline(primefile, PN);
        primefile.close();

        // read number and chick prime number
        istringstream str(PN);
        string out;
        while (str.good()) {
            getline(str, out, ',');
            if (!out.empty()) {
                num = strtol(out.c_str(),nullptr,10);
                if(num >= get_numer){
                    cout << num << endl;
                    return 0;
                }
            }
        }
    }
    return 0;
}