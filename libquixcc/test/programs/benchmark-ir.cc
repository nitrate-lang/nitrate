////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (c) 2024, Wesley C. Jones. All rights reserved.              ///
///     * License terms may be found in the LICENSE file.                        ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#include <quixcc.hpp>
#include <string>
#include <chrono>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <code-example.hpp>

constexpr size_t num_iterations = 10000;

struct GeneralDistributionStats
{
    std::string dist_name;

    double mean;
    double median;
    double mode;
    double stddev;
    double min;
    double max;
    double q1;
    double q3;
    double iqr;
    double range;
    double skewness;
    double kurtosis;
    double variance;
    double sum;

    std::vector<double> data;

    GeneralDistributionStats()
        : mean(0.0), median(0.0), mode(0.0), stddev(0.0), min(0.0), max(0.0), q1(0.0), q3(0.0), iqr(0.0), range(0.0), skewness(0.0), kurtosis(0.0), variance(0.0), sum(0.0)
    {
    }

    void compute(const std::vector<double> &data)
    {
        this->data = data;

        mean = median = mode = stddev = min = max = q1 = q3 = iqr = range = skewness = kurtosis = variance = sum = 0.0;

        if (data.size() == 0)
            return;

        for (auto &d : data)
            sum += d;

        mean = sum / data.size();

        std::vector<double> sorted_data = data;
        std::sort(sorted_data.begin(), sorted_data.end());

        min = sorted_data.front();
        max = sorted_data.back();

        if (data.size() % 2 == 0)
            median = (sorted_data[data.size() / 2 - 1] + sorted_data[data.size() / 2]) / 2;
        else
            median = sorted_data[data.size() / 2];

        q1 = sorted_data[data.size() / 4];
        q3 = sorted_data[data.size() * 3 / 4];
        iqr = q3 - q1;
        range = max - min;

        double sum_dev = 0.0;
        for (auto &d : data)
            sum_dev += (d - mean) * (d - mean);

        variance = sum_dev / data.size();
        stddev = std::sqrt(variance);

        double sum_cubed_dev = 0.0;
        for (auto &d : data)
            sum_cubed_dev += (d - mean) * (d - mean) * (d - mean);

        skewness = sum_cubed_dev / (data.size() * stddev * stddev * stddev);

        double sum_fourth_dev = 0.0;
        for (auto &d : data)
            sum_fourth_dev += (d - mean) * (d - mean) * (d - mean) * (d - mean);

        kurtosis = sum_fourth_dev / (data.size() * variance * variance) - 3;

        mode = sorted_data[0];
        size_t mode_count = 1;
        size_t max_mode_count = 1;
        for (size_t i = 1; i < sorted_data.size(); i++)
        {
            if (sorted_data[i] == sorted_data[i - 1])
            {
                mode_count++;
                if (mode_count > max_mode_count)
                {
                    mode = sorted_data[i];
                    max_mode_count = mode_count;
                }
            }
            else
            {
                mode_count = 1;
            }
        }
    }

    void write_dataset_json(const std::string &filename)
    {
        std::ofstream file(filename);
        if (!file.is_open())
            throw std::runtime_error("Failed to open file for writing");

        file << "[";
        for (size_t i = 0; i < data.size(); i++)
        {
            file << std::setprecision(15) << data[i];
            if (i != data.size() - 1)
                file << ",";
        }
        file << "]";

        file.close();
    }

    void print()
    {
        std::cout << "\x1b[34;49;1m== " << dist_name << " Distribution Stats ==\x1b[0m" << std::endl;
        std::cout << "\x1b[33;49mMean\x1b[0m:  \t\t\x1b[36;49m" << mean << "\x1b[0m\n";
        std::cout << "\x1b[33;49mMedian\x1b[0m: \t\x1b[36;49m" << median << "\x1b[0m\n";
        std::cout << "\x1b[33;49mMode\x1b[0m: \t\t\x1b[36;49m" << mode << "\x1b[0m\n";
        std::cout << "\x1b[33;49mStdDev\x1b[0m: \t\x1b[36;49m" << stddev << "\x1b[0m\n";
        std::cout << "\x1b[33;49mMin\x1b[0m: \t\t\x1b[36;49m" << min << "\x1b[0m\n";
        std::cout << "\x1b[33;49mMax\x1b[0m: \t\t\x1b[36;49m" << max << "\x1b[0m\n";
        std::cout << "\x1b[33;49mQ1\x1b[0m: \t\t\x1b[36;49m" << q1 << "\x1b[0m\n";
        std::cout << "\x1b[33;49mQ3\x1b[0m: \t\t\x1b[36;49m" << q3 << "\x1b[0m\n";
        std::cout << "\x1b[33;49mIQR\x1b[0m: \t\t\x1b[36;49m" << iqr << "\x1b[0m\n";
        std::cout << "\x1b[33;49mRange\x1b[0m: \t\t\x1b[36;49m" << range << "\x1b[0m\n";
        std::cout << "\x1b[33;49mSkewness\x1b[0m: \t\x1b[36;49m" << skewness << "\x1b[0m\n";
        std::cout << "\x1b[33;49mKurtosis\x1b[0m: \t\x1b[36;49m" << kurtosis << "\x1b[0m\n";
        std::cout << "\x1b[33;49mVariance\x1b[0m: \t\x1b[36;49m" << variance << "\x1b[0m\n";
        std::cout << "\x1b[33;49mSum\x1b[0m: \t\t\x1b[36;49m" << sum << "\x1b[0m\n";
        std::cout << "\x1b[34;49;1m============================\x1b[0m\n"
                  << std::endl;
    }
};

struct BenchStats
{
    GeneralDistributionStats ct;
    GeneralDistributionStats ct_per_source_byte;
    GeneralDistributionStats ct_per_target_byte;
};

static bool compile_single_source(std::vector<uint8_t> &out)
{
    char *outbuf;
    size_t outlen;

    FILE *outf = open_memstream(&outbuf, &outlen);
    if (!outf)
        return false;

    auto &compiler = quixcc::CompilerBuilder()
                         .add_code(libquixcc::benchmark_source, libquixcc::benchmark_source_len)
                         .set_output(outf)
                         .opt("-emit-ir")
                         .build()
                         .run()
                         .puts();

    if (!compiler.ok())
    {
        fclose(outf);
        free(outbuf);
        return false;
    }

    fclose(outf);

    out = std::vector<uint8_t>(outbuf, outbuf + outlen);

    free(outbuf);

    return true;
}

static BenchStats run_bench()
{
    BenchStats bench;

    std::vector<double> ct_data;
    std::vector<uint8_t> out;

    for (size_t i = 0; i < num_iterations; i++)
    {

        auto start = std::chrono::high_resolution_clock::now();
        if (!compile_single_source(out))
            throw std::runtime_error("Failed to compile source");
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        ct_data.push_back(duration);

        if (i % (num_iterations / 20) == 0)
        {
            int percent = ((i * 100) / num_iterations) + 5;
            std::cout << "\x1b[36;49;1m*\x1b[0m \x1b[36;4mProgress: \x1b[0m\x1b[36;1;4m" << percent << "%\x1b[0m\tIteration " << i << "\t took " << duration << " nanoseconds" << std::endl;
        }
    }

    /* Overall */
    bench.ct.dist_name = "Compilation Time";
    bench.ct.compute(ct_data);

    auto copy_ct_data = ct_data;

    /* Per source byte */
    bench.ct_per_source_byte.dist_name = "Compilation Time Per Source Bit";
    for (auto &d : copy_ct_data)
        d /= libquixcc::benchmark_source_len * 8;
    bench.ct_per_source_byte.compute(copy_ct_data);

    /* Per target byte */
    bench.ct_per_target_byte.dist_name = "Compilation Time Per Target Bit";
    for (auto &d : ct_data)
        d /= out.size() * 8;
    bench.ct_per_target_byte.compute(ct_data);

    std::cout << std::endl;

    return bench;
}

int main()
{
    if (!quixcc_init())
    {
        std::cerr << "Failed to initialize QUIXCC" << std::endl;
        return 1;
    }

    std::cout << "\x1b[36;49;1m*\x1b[0m Running benchmarks for \x1b[36;1;4mlibquixcc\x1b[0m..." << std::endl;
    std::cout << "\x1b[36;49;1m*\x1b[0m Doing " << num_iterations << " iterations" << std::endl;
    std::cout << "\x1b[36;49;1m*\x1b[0m This may take a while. Status updates will be printed every 5% of the way\n"
              << std::endl;
    auto bench = run_bench();

    std::cout << "\x1b[36;49;1m*\x1b[0m \x1b[32;49;1mBenchmarks complete\x1b[0m" << std::endl;

    std::cout << "\x1b[36;49;1m*\x1b[0m Printing benchmark results..."
              << std::endl;
    std::cout << "\x1b[36;49;1m*\x1b[0m \x1b[32;49;1mTimes are measured in nanoseconds\x1b[0m\n"
              << std::endl;

    bench.ct.print();
    bench.ct_per_source_byte.print();
    bench.ct_per_target_byte.print();

    std::cout << "\x1b[36;49;1m*\x1b[0m \x1b[32;49;1mBenchmark results printed\x1b[0m" << std::endl;

    std::cout << "\x1b[36;49;1m*\x1b[0m Writing datasets to \x1b[36;1;4mct_data.json, ct_per_source_byte_data.json, ct_per_target_byte_data.json\x1b[0m" << std::endl;
    bench.ct.write_dataset_json("ct_data.json");
    bench.ct_per_source_byte.write_dataset_json("ct_per_source_byte_data.json");
    bench.ct_per_target_byte.write_dataset_json("ct_per_target_byte_data.json");
    return 0;
}
