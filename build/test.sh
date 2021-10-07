cmake --build . -j4
mpirun -n 4 ./gtest_sort 