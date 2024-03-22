#pragma once

#include <map>
#include <algorithm>

#include "svg.h"
#include "geo.h"
#include "domain.h"

namespace map_renderer {
    inline const double EPSILON = 1e-6;
    bool IsZero(double value);
    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
            double max_height, double padding)
            : padding_(padding) {
            if (points_begin == points_end) {
                return;
            }

            const auto [left_it, right_it]
                = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                return lhs->coordnts.lng < rhs->coordnts.lng;
                    });
            min_lon_ = (*left_it)->coordnts.lng;
            const double max_lon = (*right_it)->coordnts.lng;

            const auto [bottom_it, top_it]
                = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                return lhs->coordnts.lat < rhs->coordnts.lat;
                    });
            const double min_lat = (*bottom_it)->coordnts.lat;
            max_lat_ = (*top_it)->coordnts.lat;

            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            }
        }

        svg::Point operator()(geo::Coordinates coords) const {
            return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    struct RendererSettings {
        double width;
        double height;
        double padding;
        double line_width;
        double stop_radius;
        int bus_label_font_size;
        svg::Point bus_label_offset;
        int stop_label_font_size;
        svg::Point stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width;
        std::vector<svg::Color> color_palette;
    };

    class MapRenderer {
    public:
        MapRenderer() {

        }

        void SetSettings(RendererSettings settings) {
            render_settings_ = std::move(settings);
        }

        svg::Document RenderBusRoutes(const std::map<std::string_view, domain::BusPtr>& buses_dict) const;
        const RendererSettings& GetSettings() const;

    private:
        RendererSettings render_settings_;
        
        svg::Polyline RenderBusLine(domain::BusPtr bus_ptr, SphereProjector& projector, int color_number) const;
        svg::Text RenderBusNameBackgound(domain::BusPtr bus_ptr, svg::Point position) const;
        svg::Text RenderBusNumber(domain::BusPtr bus_ptr, svg::Point position, int color_number) const;
        svg::Circle RenderStopSymbol(svg::Point position) const;
        svg::Text RenderStopNameBackgound(domain::StopPtr stop_ptr, svg::Point position) const;
        svg::Text RenderStopName(domain::StopPtr stop_ptr, svg::Point position) const;

        
    };

}