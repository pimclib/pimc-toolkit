#include <tuple>
#include <string>
#include <iostream>

#include <fmt/format.h>

#include "pimc/net/IPv4Address.hpp"
#include "pimc/net/IPv4Prefix.hpp"
#include "pimc/parsers/IPv4Parsers.hpp"
#include "pimc/formatters/IPv4Formatters.hpp"
#include "pimc/formatters/IPv4Ostream.hpp"

#include "pimc-experiment-utils/Prompt.hpp"

using namespace pimc;

int main() {
    std::string line;

    Prompt::show();
    while (std::getline(std::cin, line)) {
        Prompt p{};

        auto ra = parse<net::IPv4Address>(line);
        if (std::get<bool>(ra)) {
            auto addr = std::get<net::IPv4Address>(ra);
            std::cout << " address\n";
            std::cout << "  via ostream: " << addr << "\n";
            std::cout << "  via fmt    : " << fmt::format("{}", addr) << std::endl;
            continue;
        }

        auto rp = parse<net::IPv4Prefix>(line);
        if (std::get<bool>(rp)) {
            auto pfx = std::get<net::IPv4Prefix>(rp);
            std::cout << " prefix\n";
            std::cout << "  via ostream: " << pfx << "\n";
            std::cout << "  via fmt    : " << fmt::format("{}", pfx) << std::endl;
            continue;
        }

        std::cout << "unrecognized input" << std::endl;
    }

    std::cout << std::endl;
    return 0;
}