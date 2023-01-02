#!/bin/bash
set -e
# describe: For Multi-FastQ Files Clustering, At Least 2 Files
# date: 2022/09/21
# author: sunhui

echo "1. get parameters."
g++ src/preprocess_cluster_without_n.cpp -O3 -march=native -fopenmp -std=c++11 -o src/preprocess_cluster_without_n.out
dirPath="/public/home/jd_sunhui/genCompressor/Data/NextSeq-550_Homo_sapiens_SE"
# DataSet1 MgiSeq-2000RS_Mus-musculus_PE    read_len=100bp num=53,196,342  reads_size=5,372,850KB
# DataSet2 Dnb-Seq-T7_human_metagenome_SE   read_len=50bp  num=50,454,509  reads_size=2 573 180KB
# DataSet3 NextSeq-550_Homo_sapiens_SE      read_len=75bp  num=507,400,829 reads_size=38,562,464KB
echo "  testPath: ${dirPath}"
preserve_quality="False"
num_thr=8


echo "2. chick output files path"
if [ -d "$dirPath/output" ]; then
  	# echo "Directory named output already exists in directory containing FASTQ file. Removing directory."
  	rm -rf $dirPath/output
fi
mkdir -p $dirPath/output/


echo "3. get files' name and pre-processing each file"
files=`ls $dirPath`
for tempFile in $files
do
  {
  if [[ ${tempFile:$((${#tempFile}-3))} == ".fq" ]] || [[ ${tempFile:$((${#tempFile}-6))} == ".fastq" ]]
  then
    readLen=`head -2 ${dirPath}/${tempFile} | wc -L`
    ./src/preprocess_cluster_without_n.out $tempFile $dirPath $preserve_quality $readLen
  fi
  }&
done
wait

echo "4. calculation clustering parameters"
C_ram=10;
g++ src/get_clustering_parameter_K.cpp -fopenmp -std=c++11 -O3 -o src/get_clustering_parameter_K.out
src/get_clustering_parameter_K.out ${dirPath}/output ${num_thr} ${C_ram}

echo "5. begin to clustering"
g++ -std=c++11 -fopenmp -O3 src/clustering.cpp -o src/clustering.out
k_cluster=`tail -1 ${dirPath}/output/Cluster.config`
read_len=`head -1 ${dirPath}/output/Cluster.config`
src/clustering.out ${dirPath}/output ${num_thr} ${k_cluster}

# remove some files
dir=`pwd`
cd $dirPath/output
rm *vec
rm Cluster.config
cd ${dir}

echo "6. begin sim replacement by CHL"
g++ src/select_prime_number.cpp -O3 -std=c++11 -o src/select_prime_number.out
# 在这里编译去重文件
for k in `seq 1 ${k_cluster}`
do
  name=${dirPath}/output/Cluster_${k}.info
  cluster_num=`head -1 "${name}"`
  prime_num=`src/select_prime_number.out ${cluster_num}`
  echo "  Cluster ${k} ........."
  echo "  ${read_len}"
  echo "  ${cluster_num}"
  echo "  ${prime_num}"
  # 在这里调用去重文件
done





