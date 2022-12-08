/* **********************************************
date:2022/08/19
author:shubhamchandak94(source author),SH
describe:this file is used to parse fastq file. reference from:
https://github.com/shubhamchandak94
[input_filename] [output_dir] [preserve_quality] [readLen]
********************************************** */
#include <iostream>
#include <fstream>
#include <string>

std::string infile;
std::string outfileclean;
std::string outfilequality;
std::string outfileid;
std::string outfilenumreads;
std::string outfilevector;
std::string preserve_quality;
std::string outfile_N;
std::string outfile_N_order;

int readlen;


int preprocess();

int main(int argc, char** argv)
{
    // 1 get parameters
	infile = std::string(argv[1]);
	std::string basedir = std::string(argv[2]);
	std::string infile1 = infile;
	infile = basedir + "/" + infile1;
	preserve_quality = std::string(argv[3]);
	readlen = atoi(argv[4]);

	// 2 parse output files name
	int a = infile1.find('.');
	infile1 = infile1.erase(a);
	outfileclean = basedir + "/output/" + infile1 + ".dna";       // pure dna reads file
	outfilequality = basedir + "/output/" + infile1 + ".quality"; // quality socres file
	outfileid = basedir + "/output/" + infile1 + ".id";           // header files
	outfilenumreads = basedir + "/output/" + infile1 + ".num";    // total number of reads
	outfilevector = basedir + "/output/" + infile1 + ".vec";      // feature of each pure read
	outfile_N = basedir + "/output/" + infile1 + ".dnan";         // dirty dna reads with base n
	outfile_N_order = basedir + "/output/" + infile1 + ".norder"; // order of diry dna base

	// 3 begin to parse fastq file
	int status = preprocess();
	if(status != 0)
		return -1;
	//std::cout << "Preprocessing Done!\n";
	return 0;
}

int preprocess()
{
	std::string line;
	std::ifstream myfile(infile, std::ifstream::in);
	std::ofstream f_clean(outfileclean);
	std::ofstream f_quality;
	std::ofstream f_id;
	std::ofstream f_vector(outfilevector);
	std::ofstream f_n_dna(outfile_N);
	std::ofstream f_n_order(outfile_N_order,std::ios::binary);

	if(preserve_quality == "True") {
        f_quality.open(outfilequality);
        f_id.open(outfileid);
    }
	int i = 0;
    uint64_t readnum = 0;
    int A=0;
    int C=0;
    int G=0;
    int T=0;
    int res=0;
    int num_read_clean = 0;
    int num_read_n = 0;


	while(std::getline(myfile, line))
	{
		switch(i)
		{
			case 0:	if(preserve_quality == "True")
						f_id << line << "\n";
				break;
			case 1: if(line.length() != readlen)
				{
					std::cout << "Read length not fixed. Found two different read lengths: "<<
						  readlen << " and " << line.length() << "\n";
					return -1;
				}
				else{
				    if(line.find('N')!=std::string::npos){ // input dirty dna and their position
				    	f_n_dna << line << "\n";
					    f_n_order.write((char*)&readnum,sizeof(uint32_t));
					    num_read_n++;
				    }
				    else{
                        A=0; C=0; G=0; T=0; res=0;
                        for(int i=0; i<readlen; i++){
                            if(line[i]=='A') A++;
                            if(line[i]=='C') C++;
                            if(line[i]=='G') G++;
                            if(line[i]=='T') T++;
                        }
                        f_clean << line << "\n";
                        f_vector << A*C*G*T/readlen << "\n";
                        num_read_clean++;
				    }

				    readnum++;
				}
				break;
			case 2: break;
			case 3: if(preserve_quality == "True")
						f_quality << line << "\n";
				break;
		}
		i = (i+1)%4;
	}
	if(preserve_quality == "True") {
        f_quality.close();
        f_id.close();
    }
	f_clean.close();
	f_vector.close();

	if(readnum > 4294967290)
	{
		std::cout << "Too many reads. HARC supports at most 4294967290 reads\n";
		return -1;
	}
	else
	{
	    // save numbers of reads
	    std::ofstream f_numreads(outfilenumreads);
		// clean read number and dirty read number
		f_numreads << num_read_clean << "\n";
		f_numreads << num_read_n << "\n";
		f_numreads << readlen << "\n";
		f_numreads.close();

		// std::cout << "Read length: " << readlen << "\n";
		// std::cout << "Total number of reads: " << readnum <<"\n";
		// std::cout << "Total number of reads without N: " << num_read_clean <<"\n";
		// std::cout << "Total number of reads with N: " << num_read_n <<"\n";


	}
	return 0;
}
