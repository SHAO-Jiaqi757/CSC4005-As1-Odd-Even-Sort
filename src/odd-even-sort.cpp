#include <odd-even-sort.hpp>
#include <mpi.h>
#include <iostream>
#include <vector>

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
    std::unique_ptr<Information> Context::mpi_sort(Element *begin, Element *end) const
    {
        int res;
        int rank;
        std::unique_ptr<Information> information{};

        res = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
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
            if (rank == 0) {
                int length = end-begin;
                int size = information->num_of_proc;;
                int element_per_proc = length/size; 
                for (int i = 0; i < size; i++) {
                    sendcnts.push_back(element_per_proc);
                    displs.push_back(element_per_proc*i);
                }
                sendcnts[size-1]+=length%size;
            }
            MPI_Scatter( sendcnts.data() , 1 , MPI_INT , &recv_buff_size , 1 , MPI_INT , 0 , MPI_COMM_WORLD); // send size;

            std::vector<Element> recv_data(recv_buff_size, 0);  // create receive buffer
            MPI_Scatterv(begin,sendcnts.data(),displs.data(), MPI_UINT64_T, recv_data.data(), recv_buff_size, MPI_UINT64_T, 0, MPI_COMM_WORLD);
            if (0 == rank)
            {
                printVector(recv_data);
                std::cout << "I am 0 and I am sorting." << std::endl;
                std::sort(begin, end);
            }
            else
            {
                // std::cout << "I am " << rank << " and I am doing nothing." << std::endl;
                printVector(recv_data);
                std::cout << "I receive length " << recv_buff_size << std::endl;
            }
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
