/*
      This file is part of the Goptical library.
  
      The Goptical library is free software; you can redistribute it
      and/or modify it under the terms of the GNU General Public
      License as published by the Free Software Foundation; either
      version 3 of the License, or (at your option) any later version.
  
      The Goptical library is distributed in the hope that it will be
      useful, but WITHOUT ANY WARRANTY; without even the implied
      warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
      See the GNU General Public License for more details.
  
      You should have received a copy of the GNU General Public
      License along with the Goptical library; if not, write to the
      Free Software Foundation, Inc., 59 Temple Place, Suite 330,
      Boston, MA 02111-1307 USA
  
      Copyright (C) 2010-2011 Free Software Foundation, Inc
      Author: Alexandre Becoulet
*/

#include <goptical/core/sys/System>
#include <goptical/core/sys/SourcePoint>
#include <goptical/core/sys/Image>
#include <goptical/core/sys/OpticalSurface>

#include <goptical/core/material/Base>

#include <goptical/core/curve/Flat>
#include <goptical/core/shape/Rectangle>
#include <goptical/core/shape/Composer>
#include <goptical/core/shape/Disk>

#include <goptical/core/trace/Tracer>
#include <goptical/core/trace/Distribution>
#include <goptical/core/trace/Params>
#include <goptical/core/trace/Result>
#include <goptical/core/trace/Sequence>

#include <goptical/core/math/Vector>
#include <goptical/core/math/Quaternion>
#include <goptical/core/math/Transform>
#include <goptical/core/math/VectorPair>

#include <goptical/design/telescope/Newton>

using namespace goptical;

template <class X>
class CompositeTest : public X
{

public:
    CompositeTest();

    void redraw();
    void resize(int width, int height);

    sys::system sys;
    trace::tracer tracer;
};

template <class X>
CompositeTest<X>::CompositeTest()
  : X(),
    sys(),
    tracer(sys)
{

    
    // optical system
    curve::Flat flat1;
        
    shape::Rectangle outer_shape(80);
    shape::Rectangle inner_shape(10);
    shape::Composer  master_shape;
    shape::Composer::Attributes attrs = master_shape.add_shape(outer_shape);

    std::cout << "composer" << std::endl << "--------" << std::endl;
    std::cout << "max_raduis: " << master_shape.max_radius() << std::endl;
    std::cout << "min_raduis: " << master_shape.min_radius() << std::endl;
    math::VectorPair2 master_bbox = master_shape.get_bounding_box();
    std::cout << "bbox x1: " << master_bbox.x1() << std::endl;
    std::cout << "bbox y1: " << master_bbox.y1() << std::endl;    
    std::cout << "--------" << std::endl << std::endl;
    
    std::cout << "rectangle" << std::endl << "--------" << std::endl;
    std::cout << "max_raduis: " << outer_shape.max_radius() << std::endl;
    std::cout << "min_raduis: " << outer_shape.min_radius() << std::endl;
    math::VectorPair2 rect_bbox = outer_shape.get_bounding_box();
    std::cout << "bbox x1: " << rect_bbox.x1() << std::endl;
    std::cout << "bbox y1: " << rect_bbox.y1() << std::endl;    
    std::cout << "--------" << std::endl << std::endl;
    
    sys::OpticalSurface s1(math::Vector3(0, 0, 0), // position,
                           flat1, master_shape, material::none, material::none);


    
    sys::SourcePoint source(sys::SourceAtInfinity,
                            math::Vector3(0, 0, 1));
    
    sys.add(source);

    sys.add(s1);

    sys::Image    image(math::Vector3(0, 0, 500),  // position
                      60);                       // square size,

    sys.add(image);

    //sys.set_entrance_pupil(newton.get_primary());

    // ray trace

    
/*
      // optical system

    sys::SourcePoint      source(sys::SourceAtInfinity, math::vector3_001);
    sys.add(source);

    Design::telescope::Newton     newton(math::vector3_0, 750, 250);
    sys.add(newton);

    sys::Image            image(newton.get_focal_plane(), 15);
    sys.add(image);

    sys.set_entrance_pupil(newton.get_primary());
  
*/
    
    tracer.get_params().set_default_distribution(
        trace::Distribution(trace::HexaPolarDist, 3));

    tracer.get_trace_result().set_generated_save_state(source);
    tracer.trace();

    // viewport setup

    sys.draw_3d_fit(*X::renderer);

    X::translation = X::renderer->get_camera_transform().get_translation();
    X::rotation = math::Vector3(0, 90, 0);

    X::main_loop();
}

template <class X>
void CompositeTest<X>::redraw()
{
  X::renderer->clear();

  math::Transform<3> t(
         math::Quaternion::angle(math::vector3_100, X::rotation.x()) * 
         math::Quaternion::angle(math::vector3_010, X::rotation.y()) *
         math::Quaternion::angle(math::vector3_001, X::rotation.z()),
         X::translation);

  X::renderer->set_camera_transform(t);

  // draw rays
  tracer.get_trace_result().draw_3d(*X::renderer);
  // draw system
  sys.draw_3d(*X::renderer);

  X::renderer->flush();
}

template <class X>
void CompositeTest<X>::resize(int width, int height)
{
  // set output window size
  X::renderer->set_2d_size(width, height);

#if 1
  // use perspective projection
  X::renderer->set_perspective();
#else
  // use orthographic projection
  sys.draw_2d_fit(*X::renderer);
#endif
}
