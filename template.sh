#!/bin/bash
#SBATCH --account=csc4005
#SBATCH --partition=debug
#SBATCH --qos=normal
#SBATCH --ntasks=4



echo "mainmode: " && /bin/hostname
mpirun /pvfsmnt/119010256/test
