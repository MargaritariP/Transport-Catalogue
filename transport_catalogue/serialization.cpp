#include "serialization.h"

namespace serialization {

	void Serialization::SerializeProgramm(std::ostream& out_file)
	{
		SerializeStops();
		SerializeDistance();
		SerializeBuses();
		SerializeMapRendererSettings();
		SerializeRouterSettings();
		proto_catalogue.SerializeToOstream(&out_file);
	}

	void Serialization::DeserializeProgramm(std::istream& in_file)
	{
		proto_catalogue.ParseFromIstream(&in_file);
		DeserializeStops();
		DeserializeDistance();
		DeserializeBuses();
		DeserializeMapRendererSettings();
		DeserializeRouterSettings();
	}

	void Serialization::SerializeStops()
	{
		for (const auto& stop : catalogue.GetStopsData()) {
			transport_catalogue_proto::Stop* proto_stop = proto_catalogue.add_stops();
			proto_stop->set_stop_name(stop.stop_name);
			proto_stop->mutable_coordnts()->set_lat(stop.coordnts.lat);
			proto_stop->mutable_coordnts()->set_lng(stop.coordnts.lng);
			proto_stop->set_id(stop.id);
		}
	}

	void Serialization::SerializeDistance()
	{
		for (const auto& [pair, dis] : catalogue.GetStopDistancesData()) {
			transport_catalogue_proto::DistanceBetweenStops* proto_distance_data = proto_catalogue.add_stop_distances();
			proto_distance_data->set_from(pair.first->stop_name);
			proto_distance_data->set_to(pair.second->stop_name);
			proto_distance_data->set_distance(dis);
		}
	}

	void Serialization::SerializeBuses()
	{
		for (const auto& [str, bus_ptr] : catalogue.GetBusesData()) {
			transport_catalogue_proto::Bus* proto_bus = proto_catalogue.add_buses();
			proto_bus->set_bus_num(bus_ptr->bus_num);
			size_t s = bus_ptr->stops.size();
			for (size_t i = 0; i < s; ++i) {
				proto_bus->add_route(bus_ptr->stops[i]->stop_name);
			}
			proto_bus->set_is_circle(bus_ptr->is_circle);
		}
	}

	void Serialization::DeserializeBuses()
	{
		for (size_t i = 0; i < proto_catalogue.buses_size(); ++i) {
			const transport_catalogue_proto::Bus& proto_bus = proto_catalogue.buses(i);
			domain::Bus bus_;
			bus_.bus_num = proto_bus.bus_num();
			bus_.stops.reserve(proto_bus.route_size());
			for (size_t i = 0; i < proto_bus.route_size(); ++i) {
				bus_.stops.push_back(catalogue.FindStop(proto_bus.route(i)));
			}
			bus_.is_circle = proto_bus.is_circle();
			catalogue.AddBus(std::move(bus_));
		}
	}


	void Serialization::DeserializeStops()
	{
		for (size_t i = 0; i < proto_catalogue.stops_size(); ++i) {
			const transport_catalogue_proto::Stop& proto_stop = proto_catalogue.stops(i);
		    domain::Stop stop_;
			stop_.stop_name = proto_stop.stop_name();
			stop_.coordnts = { proto_stop.coordnts().lat(), proto_stop.coordnts().lng() };
			stop_.id = proto_stop.id();
			catalogue.AddStop(std::move(stop_));
		}
	}

	void Serialization::DeserializeDistance()
	{
		for (size_t i = 0; i < proto_catalogue.stop_distances_size(); ++i) {
			const transport_catalogue_proto::DistanceBetweenStops& proto_distance = proto_catalogue.stop_distances(i);
			catalogue.SetDistance(catalogue.FindStop(proto_distance.from()), catalogue.FindStop(proto_distance.to()), proto_distance.distance());
		}
	}

	void Serialization::SerializeMapRendererSettings()
	{
		map_renderer_proto::RendererSettings* proto_settings = proto_catalogue.mutable_renderer();
		map_renderer::RendererSettings renderer_settings = renderer.GetSettings();
		proto_settings->set_width(renderer_settings.width);
		proto_settings->set_height(renderer_settings.height);
		proto_settings->set_padding(renderer_settings.padding);
		proto_settings->set_line_width(renderer_settings.line_width);
		proto_settings->set_stop_radius(renderer_settings.stop_radius);
		proto_settings->set_bus_label_font_size(renderer_settings.bus_label_font_size);
		*proto_settings->mutable_bus_label_offset() = std::move(SerializePoint(renderer_settings.bus_label_offset));
		proto_settings->set_stop_label_font_size(renderer_settings.stop_label_font_size);
		*proto_settings->mutable_stop_label_offset() = std::move(SerializePoint(renderer_settings.stop_label_offset));
		*proto_settings->mutable_underlayer_color() = std::move(SerializeColor(renderer_settings.underlayer_color));
		proto_settings->set_underlayer_width(renderer_settings.underlayer_width);
		const std::vector<svg::Color>& color_palette = renderer_settings.color_palette;
		for (size_t i = 0; i < color_palette.size(); ++i) {
			*proto_settings->add_color_palette() = std::move(SerializeColor(color_palette[i]));
		}
	}

	svg_proto::Point Serialization::SerializePoint(const svg::Point& point)
	{
		svg_proto::Point proto_point;
		proto_point.set_x(point.x);
		proto_point.set_y(point.y);
		return proto_point;
	}

	svg_proto::Color Serialization::SerializeColor(const svg::Color& color)
	{
		svg_proto::Color proto_color;
		//if (std::holds_alternative<std::monostate>(color)) {
			//proto_color.set_has_color(false);
		//}else 
		if (std::holds_alternative<svg::Rgb>(color)) {
			svg::Rgb rgb = std::get<svg::Rgb>(color);
			proto_color.set_red(rgb.red);
			proto_color.set_green(rgb.green);
			proto_color.set_blue(rgb.blue);
			proto_color.set_is_rgb(true);
		}
		else if (std::holds_alternative<svg::Rgba>(color)) {
			svg::Rgba rgba = std::get<svg::Rgba>(color);
			proto_color.set_red(rgba.red);
			proto_color.set_green(rgba.green);
			proto_color.set_blue(rgba.blue);
			proto_color.set_opacity(rgba.opacity);
			proto_color.set_is_rgba(true);
		}
		else if (std::holds_alternative<std::string>(color)) {
			proto_color.set_color_name(std::get<std::string>(color));
			proto_color.set_is_string(true);
		}
		return proto_color;
	}

	void Serialization::DeserializeMapRendererSettings()
	{
		map_renderer_proto::RendererSettings proto_sett = proto_catalogue.renderer();
		map_renderer::RendererSettings renderer_settings;
		renderer_settings.width = proto_sett.width();
		renderer_settings.height = proto_sett.height();
		renderer_settings.padding = proto_sett.padding();
		renderer_settings.line_width = proto_sett.line_width();
		renderer_settings.stop_radius = proto_sett.stop_radius();
		renderer_settings.bus_label_font_size = proto_sett.bus_label_font_size();
		renderer_settings.bus_label_offset = DeserializePoint(proto_sett.bus_label_offset());
		renderer_settings.stop_label_font_size = proto_sett.stop_label_font_size();
		renderer_settings.stop_label_offset = DeserializePoint(proto_sett.stop_label_offset());
		renderer_settings.underlayer_color = DeserializeColor(proto_sett.underlayer_color());
		renderer_settings.underlayer_width = proto_sett.underlayer_width();
		renderer_settings.color_palette.reserve(proto_sett.color_palette_size());
		for (const auto& proto_color : proto_sett.color_palette()) {
			renderer_settings.color_palette.push_back(DeserializeColor(proto_color));
		}
		renderer.SetSettings(renderer_settings);
	}

	svg::Point Serialization::DeserializePoint(const svg_proto::Point& proto_point)
	{
		svg::Point point;
		point.x = proto_point.x();
		point.y = proto_point.y();
		return point;
	}

	svg::Color Serialization::DeserializeColor(const svg_proto::Color& proto_color)
	{
		svg::Color color;
		if (proto_color.is_rgb()) {
			svg::Rgb rgb;
			rgb.red = proto_color.red();
			rgb.green = proto_color.green();
			rgb.blue = proto_color.blue();
			color = rgb;
		}
		else if (proto_color.is_rgba()) {
			svg::Rgba rgba;
			rgba.red = proto_color.red();
			rgba.green = proto_color.green();
			rgba.blue = proto_color.blue();
			rgba.opacity = proto_color.opacity();
			color = rgba;
		}
		else if (proto_color.is_string()) {
			std::string color_;
			color_ = proto_color.color_name();
			color = color_;
		}
		return color;
	}

	void Serialization::SerializeRouterSettings()
	{
		transport_router_proto::RouterSettings proto_router;
		transport_router::RouterSettings router_settings = router.GetSettings();
		proto_router.set_bus_wait_time(router_settings.bus_wait_time_);
		proto_router.set_bus_velocity(router_settings.bus_velocity_);
		*proto_catalogue.mutable_router() = std::move(proto_router);
	}

	void Serialization::DeserializeRouterSettings()
	{
		transport_router::RouterSettings router_settings;
		router_settings.bus_velocity_ = proto_catalogue.router().bus_velocity();
		router_settings.bus_wait_time_ = proto_catalogue.router().bus_wait_time();
		router.SetRouterSettings(router_settings);
	}
	
}