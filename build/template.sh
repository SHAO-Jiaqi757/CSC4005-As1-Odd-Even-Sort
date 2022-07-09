#!/bin/bash
#SBATCH --account=csc4005
#SBATCH --partition=debug
#SBATCH --qos=normal
#SBATCH --ntasks=4



tag="1.1"
dt=$(date '+%d-%m-%Y-%H:%M:%S')
export LD_LIBRARY_PATH=/pvfsmnt/119010256/build/
for i in {1..64}
do
    echo "using $i cores"
    mpirun -n $i /pvfsmnt/119010256/build/gtest_sort >> /pvfsmnt/119010256/logs/${tag}-${dt}.log
done