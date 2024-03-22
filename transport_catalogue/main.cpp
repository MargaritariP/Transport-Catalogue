#include <fstream>
#include <iostream>
#include <string_view>
#include <filesystem>
#include "json_reader.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    transport_catalogue::TransportCatalogue catalogue;
    map_renderer::MapRenderer renderer;
    transport_router::RouterSettings settings;
    transport_router::TransportRouter router(catalogue, settings);
    serialization::Serialization serializer(catalogue, renderer, router);
    json_reader::JsonReader j_reader(std::cin);
    std::filesystem::path file = j_reader.GetFileName();
    
    if (mode == "make_base"sv) {
        j_reader.MakeBase(catalogue, renderer, router);

        std::ofstream out(file, std::ios::binary);
        serializer.SerializeProgramm(out);
    }
    else if (mode == "process_requests"sv) {
        std::ifstream in(file, std::ios::binary);
        serializer.DeserializeProgramm(in);

        request_handler::RequestHandler req_handler(catalogue, renderer, router);
        const json::Node& requests = j_reader.GetRequestsData();
        req_handler.ProcessRequests(requests);
    }
    else {
        PrintUsage();
        return 1;
    }
}



/*
#include "json_reader.h"
#include "request_handler.h"
#include "transport_router.h"

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    map_renderer::MapRenderer renderer;
    transport_router::RouterSettings settings;
    transport_router::TransportRouter router(catalogue, settings);
    request_handler::RequestHandler req_handler(catalogue, renderer);
    json_reader::JsonReader document(std::cin);
    document.ProcessDataFromInput(catalogue, req_handler, renderer, router);
}*/