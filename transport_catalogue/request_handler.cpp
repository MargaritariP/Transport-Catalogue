#include "request_handler.h"


namespace request_handler {

	void RequestHandler::ProcessRequests(const json::Node& request_data_)
	{
		router_.BuildGraph();
		json::Array result;
		const json::Array& data_ = request_data_.AsArray();
		result.reserve(data_.size());
		for (const auto& node : data_) {
			const json::Dict& map = node.AsDict();
			const auto& type = map.at("type").AsString();
			if (type == "Bus") {
				result.push_back(PrintBusRequest(map).AsDict());
			}
			else if (type == "Stop") {
				result.push_back(PrintStopRequest(map).AsDict());
			}
			else if (type == "Map") {
				result.push_back(PrintMapRequests(map).AsDict());
			}
			else if (type == "Route") {
				result.push_back(PrintRouteRequests(map).AsDict());
			}
		}
		json::Print(json::Document{ std::move(result) }, std::cout);
	}

	
	const json::Node RequestHandler::PrintBusRequest(const json::Dict& map) const
	{
		json::Node result;
		const auto& bus_num = map.at("name").AsString();
		const int id = map.at("id").AsInt();
		if (!catalogue_.FindBus(bus_num)) {
			result = json::Builder{}
				.StartDict()
				.Key("request_id").Value(id)
				.Key("error_message").Value("not found")
				.EndDict()
				.Build();
		}
		else {
			std::optional<domain::RouteStat> stat = catalogue_.FindRouteStat(bus_num);
			result = json::Builder{}
				.StartDict()
				.Key("request_id").Value(id)
				.Key("curvature").Value(stat->curvature)
				.Key("route_length").Value(stat->route_length)
				.Key("stop_count").Value(static_cast<int>(stat->stops_on_route))
				.Key("unique_stop_count").Value(static_cast<int>(stat->unique_stops))
				.EndDict()
				.Build();
		}
		return result;
	}

	const json::Node RequestHandler::PrintStopRequest(const json::Dict& map) const
	{
		json::Node result;
		const int id = map.at("id").AsInt();
		const auto& stop_name = map.at("name").AsString();
		if (!catalogue_.FindStop(stop_name)) {
			result = json::Builder{}
				.StartDict()
				.Key("request_id").Value(id)
				.Key("error_message").Value("not found")
				.EndDict()
				.Build();
		}
		else {
			json::Array buses;
			for (const auto& bus : GetBusesByStop(stop_name)) {
				buses.push_back(std::string(bus));
			}
			result = json::Builder{}
				.StartDict()
				.Key("request_id").Value(id)
				.Key("buses").Value(buses)
				.EndDict()
				.Build();
		}
		return result;

	}

	const std::set<std::string_view> RequestHandler::GetBusesByStop(std::string_view name) const {
		std::set<std::string_view> result;
		auto buses_data = catalogue_.GetBusesData();
		if (nullptr != catalogue_.FindStop(name)) {
			std::for_each(
				buses_data.begin(), buses_data.end(),
				[&](const auto& buses) {
					for (const auto& stop : buses.second->stops) {
						if (name == stop->stop_name) {
							result.insert(buses.second->bus_num);
							break;
						}
					}
				});
		}
		return result;
	}

	const json::Node RequestHandler::PrintMapRequests(const json::Dict& map) const
	{
		json::Node result;
		const int id = map.at("id").AsInt();
		std::ostringstream strm;
		svg::Document map_ = renderer_.RenderBusRoutes(catalogue_.GetSortBusesData());
		map_.Render(strm);
		result = json::Builder{}
			.StartDict()
			.Key("request_id").Value(id)
			.Key("map").Value(strm.str())
			.EndDict()
			.Build();
		return result;
	}

	const json::Node RequestHandler::PrintRouteRequests(const json::Dict& map) const
	{
		json::Node result;
		const int id = map.at("id").AsInt();
		const std::string_view stop_from = map.at("from").AsString();
		const std::string_view stop_to = map.at("to").AsString();
		const auto optimal_route = router_.FindOptimalRoute(stop_from, stop_to);
		if (!optimal_route) {
			result = json::Builder{}
				.StartDict()
				.Key("request_id").Value(id)
				.Key("error_message").Value("not found")
				.EndDict()
				.Build();
		}
		else {
			json::Array items;
			for (const auto& edge_ : optimal_route.value().edges) {
				const auto& edge = router_.GetGraph().GetEdge(edge_);
				std::string stop_name = catalogue_.GetStopsData()[edge.from].stop_name;
				double weight = router_.GetWaitTime();
				json::Dict wait;
				json::Dict route;
				wait = json::Builder{}
					.StartDict()
					.Key("time").Value(weight)
					.Key("type").Value("Wait")
					.Key("stop_name").Value(stop_name)
					.EndDict()
					.Build()
					.AsDict();
				items.push_back(std::move(wait));
				route = json::Builder{}
					.StartDict()
					.Key("time").Value(edge.weight - weight)
					.Key("span_count").Value(static_cast<int>(edge.span_count))
					.Key("bus").Value(edge.bus_num)
					.Key("type").Value("Bus")
					.EndDict()
					.Build()
					.AsDict();
				items.push_back(std::move(route));
			}
			result = json::Builder{}
				.StartDict()
				.Key("items").Value(items)
				.Key("total_time").Value(optimal_route.value().weight)
				.Key("request_id").Value(id)
				.EndDict()
				.Build()
				.AsDict();
		}
		return result;
	}
}