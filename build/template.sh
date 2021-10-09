#!/bin/bash
#SBATCH --account=csc4005
#SBATCH --partition=debug
#SBATCH --qos=normal
#SBATCH --ntasks=4



tag="1.1"
dt=$(date '+%d-%m-%Y-%H:%M:%S')
export LD_LIBRARY_PATH=/pvfsmnt/119010256/HW/build/
for i in {1..64}
do
    echo "using $i cores"
    mpirun -n $i /pvfsmnt/119010256/HW/build/gtest_sort  | tee /pvfsmnt/119010256/HW/logs/${tag}-${dt}.log
done