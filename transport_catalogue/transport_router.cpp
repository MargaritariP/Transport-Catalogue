#include "transport_router.h"

namespace transport_router {


    void transport_router::TransportRouter::BuildGraph()
    {
        Graph graph(catalogue.GetStopsData().size());
        const auto& buses = catalogue.GetBusesData();
        for (const auto& [str, bus_data] : buses) {
            const auto& stops = bus_data->stops;
            AddEdgesToGraph(graph, stops, bus_data->bus_num);
            if (!bus_data->is_circle) {
                const std::vector<domain::StopPtr> rev_st(stops.rbegin(), stops.rend());
                AddEdgesToGraph(graph, rev_st, bus_data->bus_num);
            }
        }

        graph_ = std::move(graph);
        router_unique_ptr = std::make_unique<graph::Router<double>>(graph_);
    }

    void TransportRouter::AddEdgesToGraph(Graph& graph, const std::vector<domain::StopPtr>& stops, std::string bus_num)
    {
        for (auto it_from = stops.begin(); it_from != stops.end(); ++it_from) {
            domain::StopPtr stop_from = *it_from;
            domain::StopPtr prev_stop = stop_from;
            double weight = 0.;
            for (auto it_to = std::next(it_from); it_to != stops.end(); ++it_to) {
                domain::StopPtr stop_to = *it_to;
                weight += CalculateRouteTime(prev_stop, stop_to);
                prev_stop = stop_to;
                graph.AddEdge({ stop_from->id,
                        stop_to->id,
                         weight + router_settings.bus_wait_time_,
                         static_cast<size_t>(std::distance(it_from, it_to)),
                    bus_num
                    });
            }
        }
    }

    double TransportRouter::CalculateRouteTime(const domain::StopPtr from, const domain::StopPtr to)
    {
        double distance = static_cast<double>(catalogue.GetDistance(from, to)) / meters_in_km;
        return (distance / router_settings.bus_velocity_) * minutes_in_hr;
    }

    const std::optional<graph::Router<double>::RouteInfo> TransportRouter::FindOptimalRoute(const std::string_view stop_from,
        const std::string_view stop_to) const
    {

        return router_unique_ptr->BuildRoute(catalogue.FindStop(stop_from)->id,
            catalogue.FindStop(stop_to)->id);
    }

    const Graph& TransportRouter::GetGraph() const
    {
        return graph_;
    }

    double TransportRouter::GetWaitTime() const
    {
        return router_settings.bus_wait_time_;
    }

    const RouterSettings& TransportRouter::GetSettings() const
    {
        return router_settings;
    }


}