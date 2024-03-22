#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "json_builder.h"
#include <optional>

namespace request_handler {
    using Catalogue = const transport_catalogue::TransportCatalogue;

    class RequestHandler {
    public:
        RequestHandler(transport_catalogue::TransportCatalogue& catalogue,
            map_renderer::MapRenderer& renderer, transport_router::TransportRouter& router)
            :catalogue_(catalogue)
            ,renderer_(renderer)
            ,router_(router)
        {

        }

        void ProcessRequests(const json::Node& request_data_);


    private:
        transport_catalogue::TransportCatalogue& catalogue_;
        map_renderer::MapRenderer& renderer_;
        transport_router::TransportRouter& router_;

        const json::Node PrintBusRequest(const json::Dict& map) const;
        const json::Node PrintStopRequest(const json::Dict& map) const;
        const std::set<std::string_view> GetBusesByStop(std::string_view name) const;
        const json::Node PrintMapRequests(const json::Dict& map) const;
        const json::Node PrintRouteRequests(const json::Dict& map) const;
    };
}