#pragma once

#include "geo.h"
#include <string>
#include <vector>

namespace domain {
	struct Stop;
	struct Bus;

	using StopPtr = const Stop*;
	using BusPtr = const Bus*;

	struct Stop
	{
		std::string stop_name;
		geo::Coordinates coordnts;
		size_t id;
	};

	struct Bus
	{
		std::string bus_num;
		std::vector<StopPtr> stops;
		bool is_circle;
	};

	struct RouteStat {
		size_t stops_on_route;
		size_t unique_stops;
		double route_length;
		double curvature;
	};

	class StopDistanceHasher {
	public:
		size_t operator()(const std::pair<StopPtr, StopPtr> pair_of_pointers) const noexcept {
			return hasher_(static_cast<const void*>(pair_of_pointers.first)) * 37 + hasher_(static_cast<const void*>(pair_of_pointers.second));
		}
		size_t operator()(StopPtr stop) const noexcept {
			return hasher_(static_cast<const void*>(stop)) * 37;
		}
	private:
		std::hash<const void*> hasher_;
	};
}

