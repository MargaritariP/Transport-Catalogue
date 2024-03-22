#pragma once
#include <fstream>

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "json_builder.h"
#include "transport_router.h"
#include "serialization.h"

namespace json_reader {


	class JsonReader {
	public:
		JsonReader(std::istream& is)
			: input(json::Load(is))
		{}

		void MakeBase(transport_catalogue::TransportCatalogue& catalogue,
			map_renderer::MapRenderer& renderer,
			transport_router::TransportRouter& router);

		std::string GetFileName() const;
		const json::Node& GetRequestsData() const;

	private:
		json::Document input;
		size_t stop_id = 0;

		void AddToCatalogue(transport_catalogue::TransportCatalogue& catalogue);
		domain::Stop ProcessStop(const json::Dict& node) const;
		void ProcessDistance(const json::Dict& node, transport_catalogue::TransportCatalogue& catalogue) const;
		domain::Bus ProcessBus(const json::Dict& node, transport_catalogue::TransportCatalogue& catalogue) const;
		void SetMapRenderer(map_renderer::MapRenderer& renderer);
		svg::Color SetColor(const json::Node& node) const;
		void SetRouter(transport_router::TransportRouter& router);
	};
}


