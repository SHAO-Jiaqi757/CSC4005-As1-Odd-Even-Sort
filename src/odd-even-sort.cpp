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
            int length;
            if (rank == 0)
            {
                length = end - begin;
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
            }
            MPI_Bcast(&length, 1, MPI_INT, 0, MPI_COMM_WORLD);

            // sendcnts[size-1]+=length%size;

            recv_buff_size = rank < length % proc_num ? length / proc_num + 1 : length / proc_num;
            // MPI_Scatter(sendcnts.data(), 1, MPI_INT, &recv_buff_size, 1, MPI_INT, 0, MPI_COMM_WORLD); // send size;

            std::vector<Element> recv_data(recv_buff_size, 0); // create receive buffer
            MPI_Scatterv(begin, sendcnts.data(), displs.data(), MPI_INT64_T, recv_data.data(), recv_buff_size, MPI_INT64_T, 0, MPI_COMM_WORLD);
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
            MPI_Status status;
            Element nxt_partner;
            for (int phase = 0; phase < length; phase++)
            {
                if (recv_buff_size == 0)
                    continue;
                int m = length / proc_num;
                int rem = length % proc_num;
                int offset = rank < rem ? (m + 1) * rank : rem + rank * m; // calculate each rank where the number (index) begin from.
                int boundary = offset + recv_buff_size;                    // boundary numbering of each rank;
                // printf("%d rank's offset is %d \n", rank, offset);

                for (int i = offset; i < offset + recv_buff_size; i++)
                {
                    int partner = get_partner(phase, i);
#ifdef _TEST
                    printf("In phase %d, %d's partner is %d (boundary = %d) \n", phase, i, partner, offset + recv_buff_size);
#endif
                    if (partner >= length || partner < 0)
                        continue;
                    if (partner >= offset + recv_buff_size || partner < offset)
                    {
                        // printf("overflow");
                        Element senout = recv_data[i - offset];
                        int sendrank = partner < offset ? rank - 1 : rank + 1;
                        MPI_Sendrecv(&senout, 1, MPI_INT64_T, sendrank, 1, &nxt_partner, 1, MPI_INT64_T, sendrank, 1, MPI_COMM_WORLD, &status);
#ifdef _TEST
                        printf(">>> (%d) rank send to (%d) rank: send %d,  received %d \n", rank, sendrank, senout, nxt_partner);
#endif
                        if ((rank > sendrank && recv_data[i - offset] < nxt_partner) || (rank < sendrank && recv_data[i - offset] > nxt_partner))
                        {
#ifdef _TEST
                            printf("%d swap with %d \n", recv_data[i - offset], nxt_partner);
#endif
                            recv_data[i - offset] = nxt_partner;
                        }
                        // else if (rank < sendrank && recv_data[i-offset] > nxt_partner)
                    }

                    else if (i < partner && recv_data[i - offset] > recv_data[partner - offset])
                    {
#ifdef _TEST
                        printf("%d swap with %d \n", recv_data[i - offset], recv_data[partner - offset]);
#endif
                        std::swap(recv_data[i - offset], recv_data[partner - offset]);
                    }
#ifdef _TEST
                    printf("After phase %d, rank(%d) keeps: ", phase, rank);
                    printVector(recv_data);
#endif
                }
            }

            MPI_Gatherv(recv_data.data(), recv_buff_size, MPI_INT64_T, begin, sendcnts.data(), displs.data(), MPI_INT64_T, 0, MPI_COMM_WORLD);
        }

        if (0 == rank)
        {
            information->end = high_resolution_clock::now();
            print_information(*information, std::cout);
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
