#!/bin/bash
set -e
# describe: Ordinary Compressor WorkFlow.
# date: 2022/09/28
# author: sunhui

echo "1. get parameters."

dirPath=$1
preserve_quality=$2
num_thr=$3
C_ram=$4
clean_flag=$5
echo "  dirPath: ${dirPath}"
echo "  preserve_quality: ${preserve_quality}"
echo "  num_thr: ${num_thr}"
echo "  user_set_ram: ${C_ram} GB"
echo "  clean_flag: ${clean_flag}"

echo "2. chick output files path"
if [ -d "$dirPath/output" ]; then
  rm -rf $dirPath/output
fi
if [ $? -ne 0 ]; then
    echo "  please remove the ${dirPath}/output directory manually."
    rm -rf ${test_files_dir}/output
    exit 0
fi
mkdir -p $dirPath/output/

echo "3. get files' name and pre-processing each file"
files=$(ls $dirPath)
for tempFile in $files; do
  {
    if [[ ${tempFile:$((${#tempFile} - 3))} == ".fq" ]] || [[ ${tempFile:$((${#tempFile} - 6))} == ".fastq" ]]; then
      readLen=$(head -2 ${dirPath}/${tempFile} | wc -L)
      ./preprocess_cluster_without_n.out $tempFile $dirPath $preserve_quality $readLen
      if [ $? -ne 0 ]; then
        echo "  preprocess_cluster_without_n.out error! try it again!"
        rm -rf ${test_files_dir}/output
        exit 0
      fi
    fi

  } &
done
wait

echo "4. calculation clustering parameters"
./get_clustering_parameter_K.out ${dirPath}/output ${num_thr} ${C_ram}
if [ $? -ne 0 ]; then
  echo "  get_clustering_parameter_K.out error! try it again!"
  rm -rf ${test_files_dir}/output
  exit 0
fi

echo "5. begin to clustering"
k_cluster=$(tail -1 ${dirPath}/output/Cluster.config)
read_len=$(head -1 ${dirPath}/output/Cluster.config)
./clustering.out ${dirPath}/output ${num_thr} ${k_cluster}
if [ $? -ne 0 ]; then
  echo "  clustering.out error! try it again!"
  rm -rf ${test_files_dir}/output
  exit 0
fi

# remove some files
dir=$(pwd)
cd $dirPath/output
rm *vec
rm Cluster.config
cd ${dir}

echo "6. begin sim replacement by CHL"
# 在这里编译去重文件
for k in $(seq 1 ${k_cluster}); do
  name=${dirPath}/output/Cluster_${k}.info
  cluster_num=$(head -1 "${name}")
  prime_num=$(src/select_prime_number.out ${cluster_num})
  echo "  Cluster ${k} ........."
  echo "  ${read_len}"
  echo "  ${cluster_num}"
  echo "  ${prime_num}"
  # 在这里调用去重文件
done
