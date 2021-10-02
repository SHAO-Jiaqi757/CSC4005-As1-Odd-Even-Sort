#include <odd-even-sort.hpp>
#include <mpi.h>
#include <iostream>
#include <vector>

// #define _TEST

namespace sort
{
    using namespace std::chrono;

    Context::Context(int &argc, char **&argv) : argc(argc), argv(argv)
    {
        MPI_Init(&argc, &argv);
    }

    Context::~Context()
    {
        MPI_Finalize();
    }

    template <typename Container>
    void printVector(const Container &cont)
    {
        for (auto const &x : cont)
        {
            std::cout << x << " ";
        }
        std::cout << '\n';
    }
    void merge(std::vector<Element> &mine, std::vector<Element> &recived, int recived_num, int low)
    {
        int size = mine.size();
        std::vector<Element> tmp(size, 0);

        int p, my_pointer, rec_pointer;
        if (low)
        {
            p = my_pointer = rec_pointer = 0;
            while (p < size)
            {
                if (rec_pointer >= recived_num)
                    tmp[p++] = mine[my_pointer++];
                else if (mine[my_pointer] < recived[rec_pointer])
                {
                    tmp[p++] = mine[my_pointer++];
                }
                else
                {
                    tmp[p++] = recived[rec_pointer++];
                }
            }
        }
        else
        {
            p = my_pointer = size - 1;
            rec_pointer = recived_num - 1;
            while (p >= 0)
            {
                if (rec_pointer < 0)
                    tmp[p--] = mine[my_pointer--];
                else if (mine[my_pointer] > recived[rec_pointer])
                {
                    tmp[p--] = mine[my_pointer--];
                }
                else
                {
                    tmp[p--] = recived[rec_pointer--];
                }
            }
        }

        for (int i = 0; i < size; i++)
        {
            mine[i] = tmp[i];
        }
    }

    int get_partner(int phase, int rank)
    {
        int p;
        if (phase % 2 == 0)
        {
            rank % 2 == 0 ? p = rank + 1 : p = rank - 1;
        }
        else
        {
            rank % 2 == 0 ? p = rank - 1 : p = rank + 1;
        }

        return p;
    }

    void local_odd_even_sort(std::vector<Element> &target, int size)
    {

        //    int size = target.size();
        for (int phase = 0; phase < size; phase++)
        {
            if (phase % 2 == 0) // even phase;
            {
                for (int i = 1; i < size; i += 2)
                {
                    // swap target[i-1] with target[i]
                    // swap(target, i, i - 1);
                    if (target[i-1] > target[i])
                        std::swap(target[i - 1], target[i]);
                }
            }
            else
            {
                for (int i = 2; i < size; i += 2)
                {
                    // swap(target, i, i - 1);
                    if (target[i-1] > target[i])
                        std::swap(target[i - 1], target[i]);
                }
            }
        }

        return;
    }
    std::unique_ptr<Information> Context::mpi_sort(Element *begin, Element *end) const
    {
        int res;
        int rank, proc_num;
        std::unique_ptr<Information> information{};
 
        res = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        res = MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
        if (MPI_SUCCESS != res)
        {
            throw std::runtime_error("failed to get MPI world rank");
        }

        if (0 == rank)
        {
            information = std::make_unique<Information>();
            information->length = end - begin;
            res = MPI_Comm_size(MPI_COMM_WORLD, &information->num_of_proc);
            if (MPI_SUCCESS != res)
            {
                throw std::runtime_error("failed to get MPI world size");
            };
            information->argc = argc;
            for (auto i = 0; i < argc; ++i)
            {
                information->argv.push_back(argv[i]);
            }
            information->start = high_resolution_clock::now();
        }
        {

            /// now starts the main sorting procedure
            /// @todo: please modify the following code

            int recv_buff_size;
            std::vector<int> sendcnts;
            std::vector<int> displs;
            if (rank == 0)
            {
                int length = end - begin;
                int size = information->num_of_proc;
                int remain = length % size;
                int element_per_proc = length / size;
                int displ = 0;
                for (int i = 0; i < size; i++)
                {
                    int ele = i < remain ? element_per_proc + 1 : element_per_proc;
                    sendcnts.push_back(ele);
                    displs.push_back(displ);
                    displ += ele;
                }
                // sendcnts[size-1]+=length%size;
            }
            MPI_Scatter(sendcnts.data(), 1, MPI_INT, &recv_buff_size, 1, MPI_INT, 0, MPI_COMM_WORLD); // send size;

            std::vector<Element> recv_data(recv_buff_size, 0); // create receive buffer
            MPI_Scatterv(begin, sendcnts.data(), displs.data(), MPI_INT64_T, recv_data.data(), recv_buff_size, MPI_INT64_T, 0, MPI_COMM_WORLD);
            local_odd_even_sort(recv_data, recv_buff_size);
#ifdef _TEST
            if (0 == rank)
            {
                std::cout << "I am " << rank << " and I receive: ";
                printVector(recv_data);
                // std::sort(begin, end);
            }
            else
            {
                std::cout << "I am " << rank << " and I receive: ";
                printVector(recv_data);
            }
#endif

            std::vector<Element> recv_data2(recv_buff_size, 0);

            for (int phase = 0; phase < proc_num + 1; phase++)
            {
                MPI_Status status;
                int partner = get_partner(phase, rank);
                if (partner == -1 || partner == proc_num )
                    {
                        partner = MPI_PROC_NULL;
                    }

                MPI_Sendrecv(recv_data.data(), recv_buff_size, MPI_INT64_T, partner, 1, recv_data2.data(), recv_buff_size + 1, MPI_INT64_T, partner, 1, MPI_COMM_WORLD, &status);
                // printf("In phase %d : I'm rank %d, my partner is %d \n", phase, rank, partner);
                int count = 0;
                MPI_Get_count(&status, MPI_INT64_T, &count);
                // printf("In phase %d : I'm rank % d, I receive %d numbers ", phase, rank, count);
                // printVector(recv_data2);
                if (partner == MPI_PROC_NULL)
                    continue;
                if (rank < partner)
                {
                    // keep smaller ones
                    merge(recv_data, recv_data2, count, 1);
                }
                else
                {
                    // keep larger ones
                    merge(recv_data, recv_data2, count, 0);
                }

                // printf("In phase %d : I'm rank %d, after sort, I keep: ", phase, rank);
                // printVector(recv_data);
            }
           
            MPI_Gatherv(recv_data.data(), recv_buff_size, MPI_INT64_T, begin, sendcnts.data(), displs.data(), MPI_INT64_T, 0, MPI_COMM_WORLD);
        }

        if (0 == rank)
        {
            information->end = high_resolution_clock::now();
        }
        return information;
    }

    std::ostream &Context::print_information(const Information &info, std::ostream &output)
    {
        auto duration = info.end - info.start;
        auto duration_count = duration_cast<nanoseconds>(duration).count();
        auto mem_size = static_cast<double>(info.length) * sizeof(Element) / 1024.0 / 1024.0 / 1024.0;
        output << "input size: " << info.length << std::endl;
        output << "proc number: " << info.num_of_proc << std::endl;
        output << "duration (ns): " << duration_count << std::endl;
        output << "throughput (gb/s): " << mem_size / static_cast<double>(duration_count) * 1'000'000'000.0
               << std::endl;
        return output;
    }
}
