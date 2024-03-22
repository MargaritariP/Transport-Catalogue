#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include <memory>
#include <variant>



namespace transport_router {
	using Catalogue = const transport_catalogue::TransportCatalogue;
	using Graph = graph::DirectedWeightedGraph<double>;

	struct RouterSettings {
		double bus_velocity_ = 0.0;
		double bus_wait_time_ = 0.;
	};

	class TransportRouter {
	public:
		TransportRouter(Catalogue& catalogue_, RouterSettings& settings)
			:catalogue(catalogue_)
			, router_settings(settings)
		{
		}

		void SetRouterSettings(RouterSettings settings)
		{
			router_settings = std::move(settings);
		}

		void BuildGraph();
		const std::optional<graph::Router<double>::RouteInfo> FindOptimalRoute(const std::string_view stop_from,
			const std::string_view stop_to) const;
		const Graph& GetGraph() const;
		double GetWaitTime() const;
		const RouterSettings& GetSettings() const;

	private:
		Catalogue& catalogue;
		RouterSettings router_settings;

		Graph graph_;
		std::unique_ptr<graph::Router<double>> router_unique_ptr;

		double meters_in_km = 1000.;
		double minutes_in_hr = 60.;

		void AddEdgesToGraph(Graph& graph, const std::vector<domain::StopPtr>& stops, std::string bus_num);
		double CalculateRouteTime(const domain::StopPtr from, const domain::StopPtr to);
	};

}