#pragma once

#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"

#include "svg.pb.h"

#include "map_renderer.h"
#include "map_renderer.pb.h"

#include "transport_router.h"
#include "transport_router.pb.h"

#include <iostream>


namespace serialization {
	class Serialization {
	public:
		Serialization(transport_catalogue::TransportCatalogue& catalogue_, map_renderer::MapRenderer& renderer_,
			transport_router::TransportRouter& router_)
			:catalogue(catalogue_)
			, renderer(renderer_)
			, router(router_) 
		{}

		void SerializeProgramm(std::ostream& out_file);

		void DeserializeProgramm(std::istream& in_file);

	private:
		transport_catalogue::TransportCatalogue& catalogue;
		map_renderer::MapRenderer& renderer;
		transport_router::TransportRouter& router;
		transport_catalogue_proto::TransportCatalogue proto_catalogue;

		void SerializeStops();
		void SerializeDistance();
		void SerializeBuses();


		void DeserializeBuses();
		void DeserializeStops();
		void DeserializeDistance();

		void SerializeMapRendererSettings();
		svg_proto::Point SerializePoint(const svg::Point& point);
		svg_proto::Color SerializeColor(const svg::Color& color);

		void DeserializeMapRendererSettings();
		svg::Point DeserializePoint(const svg_proto::Point& proto_point);
		svg::Color DeserializeColor(const svg_proto::Color& proto_color);

		void SerializeRouterSettings();

		void DeserializeRouterSettings();

	};
}