#include <performance_log.hpp>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cassert>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include LOG_INCLUDE

int main()
{
    unlink("log.txt");

    auto const full_access =
        S_IRUSR | S_IWUSR | S_IXUSR |
        S_IRGRP | S_IWGRP | S_IXGRP |
        S_IROTH | S_IWOTH | S_IXOTH;
    mkdir("data", full_access);
    char data[1024*1024];
    std::memset(data, 0xcd, sizeof(data));

    performance_log::rdtscp_cpuid_clock::bind_cpu(0);
    performance_log::logger<512, performance_log::rdtscp_cpuid_clock, std::uint32_t> performance_log;

    {
        LOG_INIT();

        for(unsigned number=0; number!=256u; ++number) {
            std::ostringstream ostr;
            ostr << "data/" << number;
            int fd = open(ostr.str().c_str(), O_WRONLY | O_CREAT, full_access);
            assert(fd != -1);

            auto start = performance_log.start();
            LOG_FILE_WRITE(number, 100.0*number/256.0);
            performance_log.stop(start);

            for(std::size_t i=0; i!=4*1024/256; ++i) {
                auto res = write(fd, data, sizeof(data));
                assert(res == sizeof(data));
            }
            close(fd);
        }

        LOG_CLEANUP();
    }
    performance_log::rdtscp_cpuid_clock::unbind_cpu();

    for(auto sample : performance_log) {
        std::cout << sample << std::endl;
    }
    
    return 0;
    
}
