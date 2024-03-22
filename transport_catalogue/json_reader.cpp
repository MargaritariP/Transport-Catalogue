#include "json_reader.h"

namespace json_reader {
	void JsonReader::MakeBase(transport_catalogue::TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer, transport_router::TransportRouter& router)
	{
		AddToCatalogue(catalogue);
		SetMapRenderer(renderer);
		SetRouter(router);
	}

	void JsonReader::AddToCatalogue(transport_catalogue::TransportCatalogue& catalogue)
	{
		json::Array stops;
		const json::Array& request_data = input.GetRoot().AsDict().at("base_requests").AsArray();
		for (auto& req : request_data) {
			const json::Dict& map = req.AsDict();
			const auto& type = map.at("type").AsString();
			if (type == "Stop") {
				catalogue.AddStop(ProcessStop(map));
				stop_id++;
				stops.push_back(map);
			}
		}
		for (auto& stop_map : stops) {
			ProcessDistance(stop_map.AsDict(), catalogue);
		}
		for (auto& req : request_data) {
			const json::Dict& map = req.AsDict();
			const auto& type = map.at("type").AsString();
			if (type == "Bus") {
				catalogue.AddBus(ProcessBus(map, catalogue));
			}
		}
	}

	domain::Stop JsonReader::ProcessStop(const json::Dict& node) const
	{
		domain::Stop stop_;
		stop_.stop_name = node.at("name").AsString();
		double lat = node.at("latitude").AsDouble();
		double lng = node.at("longitude").AsDouble();
		geo::Coordinates coordnts = { lat, lng };
		stop_.coordnts = coordnts;
		stop_.id = stop_id;
		return stop_;
	}

	void JsonReader::ProcessDistance(const json::Dict& node, transport_catalogue::TransportCatalogue& catalogue) const
	{
		const auto& from_str = node.at("name").AsString();
		const domain::Stop* from = catalogue.FindStop(from_str);
		for (const auto& [to_str, distance_node] : node.at("road_distances").AsDict()) {
			const domain::Stop* to = catalogue.FindStop(to_str);
			int distance = distance_node.AsInt();
			catalogue.SetDistance(from, to, distance);
		}
	}

	domain::Bus JsonReader::ProcessBus(const json::Dict& node, transport_catalogue::TransportCatalogue& catalogue) const
	{
		domain::Bus bus;
		bus.bus_num = node.at("name").AsString();
		json::Array stops_ = node.at("stops").AsArray();
		for (const auto& stop_name : stops_) {
			bus.stops.push_back(catalogue.FindStop(stop_name.AsString()));
		}
		bus.is_circle = node.at("is_roundtrip").AsBool();
		return bus;
	}

	void JsonReader::SetMapRenderer(map_renderer::MapRenderer& renderer) 
	{
		std::vector<svg::Color> color_palette;
		const json::Dict& data = input.GetRoot().AsDict().at("render_settings").AsDict();
		for (const auto& raw_color : data.at("color_palette").AsArray()) {
			color_palette.push_back(SetColor(raw_color));
		}
		map_renderer::RendererSettings render_settings{
			data.at("width").AsDouble(),
			data.at("height").AsDouble(),
			data.at("padding").AsDouble(),
			data.at("line_width").AsDouble(),
			data.at("stop_radius").AsDouble(),
			data.at("bus_label_font_size").AsInt(),
			{data.at("bus_label_offset").AsArray()[0].AsDouble(), data.at("bus_label_offset").AsArray()[1].AsDouble()},
			data.at("stop_label_font_size").AsInt(),
			{data.at("stop_label_offset").AsArray()[0].AsDouble(), data.at("stop_label_offset").AsArray()[1].AsDouble()},
			SetColor(data.at("underlayer_color")),
			data.at("underlayer_width").AsDouble(),
			color_palette
		};
		renderer.SetSettings(render_settings);
	}

	svg::Color JsonReader::SetColor(const json::Node& color) const
	{
		if (color.IsString()) {
			return color.AsString();
		}
		else {
			const auto& arr = color.AsArray();
			if (arr.size() == 3) {
				return svg::Rgb{ static_cast<uint8_t>(arr[0].AsInt()),
								static_cast<uint8_t>(arr[1].AsInt()),
								static_cast<uint8_t>(arr[2].AsInt()) };
			}
			else {
				return svg::Rgba{ static_cast<uint8_t>(arr[0].AsInt()),
								 static_cast<uint8_t>(arr[1].AsInt()),
								 static_cast<uint8_t>(arr[2].AsInt()),
								 arr[3].AsDouble() };
			}
		}
	}

	void JsonReader::SetRouter(transport_router::TransportRouter& router) 
	{
		const json::Dict data = input.GetRoot().AsDict().at("routing_settings").AsDict();
		transport_router::RouterSettings settings{
			data.at("bus_velocity").AsDouble(),
			data.at("bus_wait_time").AsDouble()
		};
		router.SetRouterSettings(settings);
	}

	std::string JsonReader::GetFileName() const 
	{
		const json::Dict& serial_sett = input.GetRoot().AsDict().at("serialization_settings").AsDict();
		std::string file = serial_sett.at("file").AsString();
		return file;
	}

	const json::Node& JsonReader::GetRequestsData() const
	{
		const json::Node& request_data = input.GetRoot().AsDict().at("stat_requests");
		return request_data;
	}

}


