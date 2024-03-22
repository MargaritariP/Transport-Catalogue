#include "transport_catalogue.h"

namespace transport_catalogue {

	void TransportCatalogue::AddStop(domain::Stop stop)
	{
		
		stops.push_back(stop);
		stops_data[stops.back().stop_name] = &stops.back();
		
	}

	void TransportCatalogue::AddBus(domain::Bus bus)
	{
		buses.push_back(bus);
		buses_data[buses.back().bus_num] = &buses.back();
		for (auto& bus_stop_ptr : bus.stops) {
			for (auto& stop : stops) {
				if (FindStop(stop.stop_name) == bus_stop_ptr) {
					buses_to_stop[bus_stop_ptr->stop_name].insert(FindBus(bus.bus_num));
				}
			}
		}
	}

	const domain::Stop* TransportCatalogue::FindStop(std::string_view stop_name) const
	{
		return stops_data.count(stop_name) ? stops_data.at(stop_name) : nullptr;
	}

	const domain::Bus* TransportCatalogue::FindBus(std::string_view bus_num) const
	{
		return buses_data.count(bus_num) ? buses_data.at(bus_num) : nullptr;
	}

	domain::RouteStat TransportCatalogue::FindRouteStat(std::string_view bus_num)
	{
		domain::RouteStat route_stat{};
		domain::BusPtr bus = FindBus(bus_num);

		if (!bus) throw std::invalid_argument("bus not found");
		if (bus->is_circle) {
			route_stat.stops_on_route = bus->stops.size();
		}
		else {
			route_stat.stops_on_route = bus->stops.size() * 2 - 1;
		}
		int route_length = 0;
		double geographic_length = 0.0;
		for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
			domain::StopPtr from = bus->stops[i];
			domain::StopPtr to = bus->stops[i + 1];
			if (bus->is_circle) {
				route_length += GetDistance(from, to);
				geographic_length += geo::ComputeDistance(from->coordnts,
					to->coordnts);
			}
			else {
				route_length += GetDistance(from, to) + GetDistance(to, from);
				geographic_length += geo::ComputeDistance(from->coordnts,
					to->coordnts) * 2;
			}
		}
		route_stat.unique_stops = GetUniqueStops(bus);
		route_stat.route_length = route_length;
		route_stat.curvature = route_length / geographic_length;
		return route_stat;
	}

	std::set<domain::BusPtr> TransportCatalogue::FindBusesOfTheStop(domain::StopPtr stop)
	{
		return buses_to_stop.at(stop->stop_name);
	}

	void TransportCatalogue::SetDistance(const domain::Stop* from, const domain::Stop* to, int distance)
	{
		distances_between_stops[{from, to}] = distance;
	}

	int TransportCatalogue::GetDistance(const domain::Stop* from, const domain::Stop* to) const
	{
		if (distances_between_stops.count({ from, to })) {
			return distances_between_stops.at({ from, to });
		}
		else if (distances_between_stops.count({ to, from })) {
			return distances_between_stops.at({ to, from });
		}
		else
		{
			return 0;
		}
	}

	size_t TransportCatalogue::GetUniqueStops(const domain::Bus* bus) const
	{
		std::set<domain::StopPtr> result;
		for (auto& stop : bus->stops) {
			result.insert(stop);
		}
		return result.size();
	}

	const std::unordered_map<std::string_view, domain::BusPtr> TransportCatalogue::GetBusesData() const
	{
		return buses_data;
	}

	const std::deque<domain::Stop> TransportCatalogue::GetStopsData() const
	{
		return stops;
	}

	const std::map<std::string_view, domain::BusPtr> TransportCatalogue::GetSortBusesData() const
	{
		std::map<std::string_view, domain::BusPtr> result;
		for (const auto& bus_data : buses_data) {
			result.emplace(bus_data);
		}
		return result;
	}

	StopsDistances TransportCatalogue::GetStopDistancesData() const
	{
		return distances_between_stops;
	}

}





