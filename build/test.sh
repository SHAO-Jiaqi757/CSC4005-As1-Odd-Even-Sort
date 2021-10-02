cmake --build . -j4
mpirun -c 4 ./gtest_sort 