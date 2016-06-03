/*

      This file is part of the <goptical/core library.
  
      The <goptical/core library is free software; you can redistribute it
      and/or modify it under the terms of the GNU General Public
      License as published by the Free Software Foundation; either
      version 3 of the License, or (at your option) any later version.
  
      The <goptical/core library is distributed in the hope that it will be
      useful, but WITHOUT ANY WARRANTY; without even the implied
      warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
      See the GNU General Public License for more details.
  
      You should have received a copy of the GNU General Public
      License along with the <goptical/core library; if not, write to the
      Free Software Foundation, Inc., 59 Temple Place, Suite 330,
      Boston, MA 02111-1307 USA
  
      Copyright (C) 2010-2011 Free Software Foundation, Inc
      Author: Alexandre Becoulet

*/

/* -*- indent-tabs-mode: nil -*- */

#include <iostream>
#include <fstream>
#include <vector>

#include <goptical/core/math/Vector>

#include <goptical/core/material/Base>
#include <goptical/core/material/Vacuum>

#include <goptical/core/sys/System>
#include <goptical/core/sys/Surface>
#include <goptical/core/sys/OpticalSurface>
#include <goptical/core/sys/SourcePoint>
#include <goptical/core/sys/Image>
#include <goptical/core/sys/Source>
#include <goptical/core/sys/SourceDisk>

#include <goptical/core/curve/Flat>
#include <goptical/core/shape/Rectangle>
#include <goptical/core/shape/Composer>
#include <goptical/core/shape/Disk>

#include <goptical/core/trace/Tracer>
#include <goptical/core/trace/Result>
#include <goptical/core/trace/Distribution>
#include <goptical/core/trace/Sequence>
#include <goptical/core/trace/Params>
#include <goptical/core/trace/Params>

#include <goptical/core/light/SpectralLine>

#include <goptical/core/analysis/RayFan>
#include <goptical/core/data/Plot>
#include <goptical/core/analysis/Spot>

#include <goptical/core/io/Rgb>
#include <goptical/core/io/RendererSvg>
#include <goptical/core/io/RendererViewport>

#include <goptical/core/math/Vector>
#include <goptical/core/math/Quaternion>
#include <goptical/core/math/Transform>
#include <goptical/core/math/VectorPair>

#include <opencv2/opencv.hpp>

using namespace goptical;

int main()
{
    curve::Flat flat1;
    
    shape::Rectangle main_rect(195);
    shape::Rectangle anti_main_rect(195);
    shape::Rectangle inner_rect(10);

    shape::Rectangle other_shape(220);

    shape::Composer  grid_shape;
    shape::Composer::Attributes &master_attr = grid_shape.add_shape(main_rect).exclude(anti_main_rect);

    for(int i = -80; i <= 80; i += 20)
    {
        for(int j = -80; j <= 80; j += 20)
        {
            master_attr.exclude(inner_rect).translate(math::Vector2(i, j));
        }
    }

    sys::OpticalSurface s1(math::Vector3(0, 0, 100),
                           flat1, grid_shape, material::none, material::none);

//    sys::OpticalSurface s2(math::Vector3(0, 0, 50),
//                           flat1, other_shape, material::none, material::none);

    // sys::SourcePoint source(sys::SourceAtInfinity,
    //                         math::Vector3(0, 0, 1));

    const math::Vector2 lim1(-100, -100);
    const math::Vector2 lim2(100, 100);

    sys::SourceDisk source(sys::SourceAtInfinity,
                            math::Vector3(0, 0, 1),
                            0.2,
                            lim1,
                            lim2);
    //sys::SourcePoint source(sys::SourceAtInfinity,
    //                        math::Vector3(0, 0, 1));

    sys::Image image(math::Vector3(0, 0, 200),
                     200);

    sys::system  sys;

    sys.add(source);
//    sys.add(s2);
    sys.add(s1);
    sys.add(image);

    std::cout << "Performing sequential raytrace" << std::endl;

    trace::tracer tracer(sys);
    trace::Sequence seq(sys);

    tracer.get_params().set_sequential_mode(seq);
    tracer.get_trace_result().clear();
    //tracer.get_params().set_default_distribution(
    //    trace::Distribution(trace::HexaPolarDist, 100));
    tracer.get_params().set_default_distribution(
        trace::Distribution(trace::RandomDist, 100));
    tracer.get_trace_result().set_generated_save_state(source);
    tracer.get_trace_result().set_intercepted_save_state(image);
    tracer.trace();
    auto result = tracer.get_trace_result().pixelate(image);

    cv::Mat picture = cvCreateMat(result.size(), result[0].size(), CV_64FC1);
    for (size_t i = 0; i < result.size(); i++) {
        for (size_t j = 0; j < result[i].size(); j++) {
            picture.at<double>(i, j) = result[i][j];
        }
    }

    analysis::Spot spot(sys);
    {
        io::RendererSvg renderer("spot.svg", 600, 600, io::rgb_black);

        spot.draw_diagram(renderer);
    }

    {
        io::RendererSvg renderer("spot_intensity.svg", 640, 480);

        ref<data::Plot> plot = spot.get_encircled_intensity_plot(50);

        plot->draw(renderer);
    }

    {
        analysis::RayFan fan(sys);
        io::RendererSvg renderer("transverse_fan.svg", 640, 480);

        ref<data::Plot> fan_plot = fan.get_plot(analysis::RayFan::EntranceHeight,
                                                analysis::RayFan::TransverseDistance);

        fan_plot->draw(renderer);

    }

    {
        io::RendererSvg       svg_renderer("layout.svg", 640, 480);
        io::RendererViewport  &renderer = svg_renderer;

        // horizontal page layout
        renderer.set_page_layout(1, 2);

        // 3d system layout on 1st sub-page
        renderer.set_page(0);
        renderer.set_perspective();

        sys.draw_3d_fit(renderer, 300);
        sys.draw_3d(renderer);

        tracer.get_trace_result().draw_3d(renderer);
    }

    cv::imshow("Display window", picture);
    cv::waitKey(0);

    return 0;
}
