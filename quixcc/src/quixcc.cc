// This code is horrible
// ill come back to it later
/// TODO: fix this code

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <quixcc.h>
#include <thread>

static int build_single_source(std::string file_in, std::string file_out, std::vector<std::string> switches)
{
    FILE *in = fopen(file_in.c_str(), "r");

    if (!in)
    {
        std::cerr << "error: failed to open input file" << std::endl;
        return 1;
    }

    FILE *out = fopen(file_out.c_str(), "w");
    if (!out)
    {
        std::cerr << "error: failed to open output file" << std::endl;
        fclose(in);
        return 1;
    }

    quixcc_job_t *job = quixcc_new();
    if (!job)
    {
        std::cerr << "error: failed to create job" << std::endl;
        fclose(in);
        fclose(out);
        return 1;
    }

    quixcc_set_input(job, in, file_in.c_str());
    quixcc_set_output(job, out);

    for (const auto &sw : switches)
        quixcc_add_option(job, sw.c_str(), true);

    bool success = quixcc_run(job);

    fclose(in);
    fclose(out);

    const quixcc_result_t *result = quixcc_result(job);
    if (!result)
    {
        std::cerr << "error: failed to get result" << std::endl;
        quixcc_dispose(job);
        std::remove(file_out.c_str());
        return 1;
    }

    for (uint32_t i = 0; i < result->m_feedback.m_count; i++)
    {
        const quixcc_msg_t *msg = result->m_feedback.m_messages[i];

        std::cout << msg->message << std::endl;
    }

    quixcc_dispose(job);

    if (!success)
    {
        std::remove(file_out.c_str());
        return 1;
    }

    try
    {
        std::filesystem::permissions(file_out, std::filesystem::perms::owner_all | std::filesystem::perms::group_all | std::filesystem::perms::others_read | std::filesystem::perms::others_exec);
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

static int build_many_sources(const std::vector<std::string> &files_in, std::string file_out, std::vector<std::string> switches)
{
    enum mode
    {
        EXECUTABLE,
        STATICLIB,
        SHARED
    };

    mode m = EXECUTABLE;

    std::vector<std::string> copy;
    for (const auto &sw : switches)
    {
        if (sw == "staticlib")
        {
            m = STATICLIB;
            break;
        }
        else if (sw == "shared")
        {
            m = SHARED;
            break;
        }
        else
        {
            copy.push_back(sw);
        }
    }

    switches = copy;

    for (const auto &file : files_in)
    {
        if (!file.ends_with(".q"))
        {
            std::cerr << "error: invalid file type. only .q files are allowed" << std::endl;
            return 1;
        }
    }

    switches.push_back({"c", ""});

    std::vector<std::string> obj_files;
    std::vector<std::thread> jobs;
    std::vector<int> rets;

    // build object files
    for (size_t i = 0; i < files_in.size(); i++)
    {
        std::string obj_file = files_in[i];
        obj_file.replace(obj_file.size() - 2, 2, ".o");
        obj_files.push_back(obj_file);

        if (build_single_source(files_in[i], obj_file, switches))
            return 1;
    }

    for (auto &job : jobs)
        job.join();

    for (const auto &ret : rets)
    {
        if (ret)
        {
            for (const auto &obj : obj_files)
                std::remove(obj.c_str());
            return 1;
        }
    }

    // now link the object files
    // based on the mode
    // -staticlib / -shared / executable

    if (m == EXECUTABLE)
    {
        std::string cmd = "ld -o " + file_out;
        for (const auto &obj : obj_files)
            cmd += " " + obj;
        int ret = std::system(cmd.c_str());
        for (const auto &obj : obj_files)
            std::remove(obj.c_str());
        return ret;
    }
    else if (m == STATICLIB)
    {
        std::string cmd = "ar rcs " + file_out;
        for (const auto &obj : obj_files)
            cmd += " " + obj;
        int ret = std::system(cmd.c_str());
        for (const auto &obj : obj_files)
            std::remove(obj.c_str());
        return ret;
    }
    else if (m == SHARED)
    {
        std::string cmd = "ld -shared -o " + file_out;
        for (const auto &obj : obj_files)
            cmd += " " + obj;
        int ret = std::system(cmd.c_str());
        for (const auto &obj : obj_files)
            std::remove(obj.c_str());
        return ret;
    }

    return 0;
}

static int parse_args(const std::vector<std::string> &args, std::vector<std::string> &files_in, std::string &file_out, std::vector<std::string> &switches)
{
    for (size_t i = 1; i < args.size(); i++)
    {
        if (args[i] == "-o")
        {
            if (i + 1 < args.size())
            {
                file_out = args[i + 1];
                i++;
            }
            else
            {
                std::cerr << "error: missing output file" << std::endl;
                return 1;
            }
        }
        else if (args[i].starts_with("-"))
        {
            if (args[i].size() < 2)
            {
                std::cerr << "error: invalid switch" << std::endl;
                return 1;
            }

            char next = args[i][1];
            if (!std::isalpha(next))
            {
                std::cerr << "error: invalid switch" << std::endl;
                return 1;
            }

            switches.push_back(args[i]);
        }
        else
        {
            try
            {
                if (std::filesystem::exists(args[i]))
                {
                    files_in.push_back(args[i]);
                }
                else
                {
                    std::cerr << "error: file not found" << std::endl;
                    return 1;
                }
            }
            catch (std::exception &e)
            {
                std::cerr << "error: " << e.what() << std::endl;
                return 1;
            }
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv + argc);
    std::vector<std::string> files_in;
    std::string file_out;
    std::vector<std::string> switches;

    if (parse_args(args, files_in, file_out, switches))
        return 1;

    if (files_in.empty())
    {
        std::cerr << "error: missing input file" << std::endl;
        return 1;
    }

    if (file_out.empty())
        file_out = "a.out";

    if (files_in.size() > 1)
        return build_many_sources(files_in, file_out, switches);

    return build_single_source(files_in[0], file_out, switches);
}