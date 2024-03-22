#pragma once

#include "domain.h"

#include <iostream>
#include <istream>
#include <sstream>
#include <unordered_map>
#include <deque>
#include <string>
#include <vector>
#include <set>
#include <string_view>
#include <algorithm>
#include <map>



namespace transport_catalogue {
	using StopsDistances = std::unordered_map<std::pair<domain::StopPtr, domain::StopPtr>,
		int, domain::StopDistanceHasher>;


	class TransportCatalogue {
	public:


		void AddStop(domain::Stop stop);
		void AddBus(domain::Bus bus);
		domain::StopPtr FindStop(std::string_view stop_name) const;
		const domain::Bus* FindBus(std::string_view bus_num) const;
		domain::RouteStat FindRouteStat(std::string_view bus_num);
		std::set<domain::BusPtr> FindBusesOfTheStop(domain::StopPtr stop);
		void SetDistance(domain::StopPtr from, domain::StopPtr to, int distance);
		int GetDistance(domain::StopPtr from, domain::StopPtr to) const;
		size_t GetUniqueStops(const domain::Bus* bus) const;
		const std::unordered_map<std::string_view, domain::BusPtr> GetBusesData() const;
		const std::deque<domain::Stop> GetStopsData() const;
		const std::map<std::string_view, domain::BusPtr> GetSortBusesData() const;
		StopsDistances GetStopDistancesData() const;
		

	private:
		std::deque<domain::Stop> stops;
		std::deque<domain::Bus> buses;
		std::unordered_map<std::string_view, domain::StopPtr> stops_data;
		std::unordered_map<std::string_view, domain::BusPtr> buses_data;
		std::unordered_map<std::string_view, std::set<domain::BusPtr>> buses_to_stop;
		StopsDistances distances_between_stops;

	};
}