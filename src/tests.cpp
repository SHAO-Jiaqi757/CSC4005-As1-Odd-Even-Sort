#include <gtest/gtest.h>
#include <odd-even-sort.hpp>
#include <random>
#include <mpi.h>

using namespace sort;
std::unique_ptr<Context> context;

/*
TEST(OddEvenSort, WorldSize)
{
    int rank;
    uint64_t cases = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (rank == 0)
    {
        std::vector<std::vector<Element>> data;
        for (int i = 1; i <= world_size; i++)
            data.push_back(std::vector<Element>(i, 0));
        cases = data.size();
        MPI_Bcast(&cases, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
        for (auto &i : data)
        {
            std::vector<Element> a = i;
            std::vector<Element> b = i;
            auto info = context->mpi_sort(a.data(), a.data() + a.size());
            std::sort(b.data(), b.data() + b.size());
            EXPECT_EQ(a, b);
        }
    }
    else
    {
        MPI_Bcast(&cases, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
        for (uint64_t i = 0; i < cases; ++i)
        {
            context->mpi_sort(nullptr, nullptr);
        }
    }
}
TEST(OddEvenSort, Simple)
{
    int rank;
    uint64_t cases = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        std::vector<Element> a(11, 0);
        for (size_t i = 0; i < a.size(); i++)
        {
            a[i] = i;
        }
        reverse(a.begin(), a.end());
        std::vector<Element> b = a;
        auto info = context->mpi_sort(a.data(), a.data() + a.size());
        std::sort(b.data(), b.data() + b.size());
        EXPECT_EQ(a, b);
    }
    else
    {
        context->mpi_sort(nullptr, nullptr);
    }
}
TEST(OddEvenSort, Basic)
{
    int rank;
    uint64_t cases = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        std::vector<std::vector<Element>> data{
            {-1},
            {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
            {7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3},
            {1, 43, 1245, 41235246, 12, 123123, -123, 0},
            std::vector<Element>(1024, 123)};
        cases = data.size();
        MPI_Bcast(&cases, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
        for (auto &i : data)
        {
            std::vector<Element> a = i;
            std::vector<Element> b = i;
            auto info = context->mpi_sort(a.data(), a.data() + a.size());
            std::sort(b.data(), b.data() + b.size());
            EXPECT_EQ(a, b);
        }
    }
    else
    {
        MPI_Bcast(&cases, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
        for (uint64_t i = 0; i < cases; ++i)
        {
            context->mpi_sort(nullptr, nullptr);
        }
    }
}
TEST(OddEvenSort, Random)
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0)
    {
        std::vector<Element> data(48'000);
        auto dev = std::random_device{};
        auto seed = dev();
        auto gen = std::default_random_engine(seed);
        auto dist = std::uniform_int_distribution<Element>{};
        for (auto &i : data)
        {
            i = dist(gen);
        }
        std::vector<Element> a = data;
        std::vector<Element> b = data;
        context->mpi_sort(a.data(), a.data() + a.size());
        std::sort(b.data(), b.data() + b.size());
        EXPECT_EQ(a, b) << " seeded with: " << seed << std::endl;
    }
    else
    {
        context->mpi_sort(nullptr, nullptr);
    }
}
*/
long gen_numbers( long range, long seed )
{ 
      std::mt19937 gen( seed );

      std::uniform_int_distribution<long> distr(0, range);

      return distr( gen );
}


// ----------------------------------------------------------------------------------------------------------------
// Generating array with random numbers
// ----------------------------------------------------------------------------------------------------------------
void gen_data_vector( std::vector<Element> &data, long size )
{

    for ( int i = 0; i < size; i++ )
    {
        data[i] = gen_numbers(size, i);
    }
    
    return;
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
    
TEST(OddEvenSort, Range)
{
    int rank;
    std::vector<long> data_sizes = {};
    // std::vector<long> data_sizes = {10, 50, 100, 200, 500, 800, 1000, 1500, 1800, 2200, 5000, 8000, 10000, 20000, 25000, 50000};
   for (int i = 30000; i<50000; i+=1000) {
       data_sizes.push_back(i);
   }
    // std::vector<long> data_sizes = {10, 50, 100, 200};
    uint64_t cases = data_sizes.size();
    // uint64_t increment = 100;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0)
    {
        std::vector<std::vector<Element>> data;
        for (size_t i = 0; i < cases; i++){
            std::vector<Element> test_data (data_sizes[i], 0);
            gen_data_vector(test_data, data_sizes[i]);
            // printVector(test_data); printf(" [size : %d] \n", test_data.size());
            data.push_back(test_data);
        }
        MPI_Bcast(&cases, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);

        for (auto &i : data)
        {
            {
                std::vector<Element> a = i;
                std::vector<Element> b = i;
                context->mpi_sort(a.data(), a.data() + a.size());
                std::sort(b.data(), b.data() + b.size());
                EXPECT_EQ(a, b);
            }
            
        }
    }
    else
    {
        MPI_Bcast(&cases, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
        for (uint64_t i = 0; i < cases; ++i)
        {
            context->mpi_sort(nullptr, nullptr);
        }
    }
}
int main(int argc, char **argv)
{
    context = std::make_unique<Context>(argc, argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::TestEventListeners &listeners =
        ::testing::UnitTest::GetInstance()->listeners();
    if (rank != 0)
    {
        delete listeners.Release(listeners.default_result_printer());
    }
    auto res = RUN_ALL_TESTS();
    context.reset();
    return res;
}
