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
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
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

constexpr size_t num_iterations = 50;

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
    GeneralDistributionStats speed;
    GeneralDistributionStats time_per_token;
    GeneralDistributionStats throughput;
};

static bool do_bench(size_t *total_toks)
{
    size_t outlen, tokens = 0;
    quixcc_tok_t tok;
    char *outbuf;
    FILE *outf, *code;
    quixcc_job_t *job;

    if ((outf = open_memstream(&outbuf, &outlen)) == nullptr)
        return false;

    if ((code = fmemopen((void *)libquixcc::benchmark_lexer_source, libquixcc::benchmark_lexer_source_len, "r")) == nullptr)
        throw std::runtime_error("Failed to open code source"); /* Mem leak is ignored */

    job = quixcc_new();
    if (!job)
        throw std::runtime_error("Failed to create job"); /* Mem leak is ignored */

    quixcc_output(job, outf, nullptr);
    quixcc_source(job, code, "bench");
    while (1)
    {
        tok = quixcc_next(job);
        if (quixcc_lex_is(&tok, QUIXCC_LEX_EOF))
            break;

        quixcc_tok_release(job, &tok);

        tokens++;
    }

    quixcc_dispose(job);

    fclose(outf);
    fclose(code);
    free(outbuf);

    *total_toks = tokens;

    return true;
}

static BenchStats run_bench()
{
    BenchStats bench;

    std::vector<double> ct_data;
    size_t total_toks = 0;

    for (size_t i = 0; i < num_iterations; i++)
    {
        auto start = std::chrono::high_resolution_clock::now();
        if (!do_bench(&total_toks))
            throw std::runtime_error("Failed to compile source");
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        ct_data.push_back(duration);

        if (num_iterations >= 20 && i % (num_iterations / 20) == 0)
        {
            int percent = ((i * 100) / num_iterations) + 5;
            std::cout << "\x1b[36;49;1m*\x1b[0m \x1b[36;4mProgress: \x1b[0m\x1b[36;1;4m" << percent << "%\x1b[0m\tIteration " << i << "\t took " << duration << " nanoseconds" << std::endl;
        }
    }

    auto copy_ct_data = ct_data;

    /* Tokens per second */
    bench.speed.dist_name = "Tokens Per Second";
    for (auto &d : copy_ct_data)
        d = total_toks / (d / 1e9);
    bench.speed.compute(copy_ct_data);

    /* Time per token */
    copy_ct_data = ct_data;
    bench.time_per_token.dist_name = "Time Per Token (ns)";
    for (auto &d : copy_ct_data)
        d = d / total_toks;
    bench.time_per_token.compute(copy_ct_data);

    /* Throughput Mbps */
    bench.throughput.dist_name = "Throughput (MBps)";
    copy_ct_data = ct_data;
    for (auto &d : copy_ct_data)
        d = (libquixcc::benchmark_lexer_source_len) / (d / 1e9) / 1e6;
    bench.throughput.compute(copy_ct_data);

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

    bench.speed.print();
    bench.time_per_token.print();
    bench.throughput.print();

    std::cout << "\x1b[36;49;1m*\x1b[0m \x1b[32;49;1mBenchmark results printed\x1b[0m" << std::endl;

    std::cout << "\x1b[36;49;1m*\x1b[0m Writing datasets to \x1b[36;1;4mct_data.json, ct_per_source_byte_data.json, ct_per_target_byte_data.json, ct_throughput_data.json\x1b[0m" << std::endl;
    bench.speed.write_dataset_json("lexer-speed.json");
    bench.time_per_token.write_dataset_json("lexer-time-per-token.json");
    bench.throughput.write_dataset_json("lexer-throughput.json");
    return 0;
}
