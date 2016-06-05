/*

      This file is part of the <goptical/core Core library.
  
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
      Author: Dmitry Galchinsky

*/

#include <limits>

#include <goptical/core/math/Vector>
#include <goptical/core/math/Matrix>

#include <goptical/core/sys/System>
#include <goptical/core/sys/Source>
#include <goptical/core/sys/SourceDisk>
#include <goptical/core/sys/Surface>

#include <goptical/core/trace/Ray>
#include <goptical/core/trace/Result>
#include <goptical/core/trace/Params>

namespace _goptical {

  namespace sys {

      SourceDisk::SourceDisk(SourceInfinityMode m, 
                             const math::Vector3& pos_dir, 
                             double size,
                             const math::Vector2& limit1,
                             const math::Vector2& limit2,
                             double density)
          : Source(m == SourceAtInfinity
                   // position of infinity source is only used for trace::Sequence sort
                   ? 
                   /* math::VectorPair3(-pos_dir, math::vector3_001) */ math::VectorPair3(pos_dir * -1e9, pos_dir)
//                   math::VectorPair3(pos_dir, math::vector3_001)
                   : math::VectorPair3(math::Vector3(0,0,0)/*pos_dir * -1e9*/, pos_dir)),
            _mode(m),
            _size(size),
            _halfsize(size / 2),
            _direction(pos_dir),
            _limit1(limit1),
            _limit2(limit2),
            _density(density)
      {
      }
      
    template <SourceInfinityMode mode>
    void SourceDisk::get_lightrays_(trace::Result &result,
                                     const Element &target) const
    {
      const Surface *starget = dynamic_cast<const Surface*>(&target);

      if (!starget)
        return;

      double rlen = result.get_params().get_lost_ray_length();
      const trace::Distribution &d = result.get_params().get_distribution(*starget);

      // i is point on target surface
      auto de = [&]( const math::Vector3 &i ) {
          math::Vector3 pp = starget->get_transform_to(*this).transform(i);  // pattern point on target surface

          if (i[0] < _limit1[0] ||
              i[0] > _limit2[0] ||
              i[1] < _limit1[1] ||
              i[1] > _limit2[1]) {
              return;
          }

          math::Vector3 direction;
          math::Vector3 position;

          switch (mode)
          {
          case (SourceAtFiniteDistance):
              //position = math::vector3_0;
              //direction = r_.normalized();
              break;

          case (SourceAtInfinity):
              static const double epsilon = 1e-8;
              const double step = std::tan(_halfsize) / _density;

              math::VectorPair3 chief_component(starget->get_position(*this) -
                                                math::vector3_001 * rlen, math::vector3_001);
              math::Vector3 chief_position = chief_component.pl_ln_intersect(math::VectorPair3(pp, math::vector3_001));
                  
              for (auto&l : this->_spectrum) {
                  {   // chief ray
                      trace::Ray &r = result.new_ray();

                      r.direction() = math::vector3_001;
                      r.origin() = chief_position;

                      r.set_creator(this);
                      r.set_intensity(l.get_intensity()); // FIXME depends on distance from source and pattern density
                      r.set_wavelen(l.get_wavelen());
                      r.set_material(_mat.valid() ? _mat.ptr() : &get_system()->get_environment_proxy());
                  }

                  for (double rad_angle_tan = std::tan(_halfsize); std::atan(rad_angle_tan) > epsilon; rad_angle_tan -= step)
                  {
                      math::Matrix3x3 radial_rot;
                      get_rotation_matrix(radial_rot, 1, std::atan(rad_angle_tan));

                      double astep = (step / rad_angle_tan) * (M_PI / 3);

                      for (double circle_angle = 0; circle_angle < 2 * M_PI - epsilon; circle_angle += astep)
                      {
                          math::Matrix3x3 circle_rot;
                          get_rotation_matrix(circle_rot, 2, circle_angle);

                          trace::Ray &r = result.new_ray();

                          r.direction() = circle_rot * (radial_rot * math::vector3_001);
                          r.origin() = chief_component.pl_ln_intersect(math::VectorPair3(pp, r.direction()));;

                          r.set_creator(this);
                          r.set_intensity(l.get_intensity()); // FIXME depends on distance from source and pattern density
                          r.set_wavelen(l.get_wavelen());
                          r.set_material(_mat.valid() ? _mat.ptr() : &get_system()->get_environment_proxy());
                      }
                  }
              }

              break;
          }

      };

      starget->get_pattern(de, d, result.get_params().get_unobstructed());

    }

    void SourceDisk::generate_rays_simple(trace::Result &result,
                                           const targets_t &entry) const
    {
      for (auto&l : _spectrum) {
        result.add_ray_wavelen(l.get_wavelen());
      }

      switch (_mode)
        {
        case SourceAtFiniteDistance:
          for (auto& target : entry) {
              get_lightrays_<SourceAtFiniteDistance>(result, *target);
          }
          return;

        case SourceAtInfinity:
          for (auto& target : entry) {
              get_lightrays_<SourceAtInfinity>(result, *target);
          }
          return;
        }
    }

    void SourceDisk::generate_rays_intensity(trace::Result &result,
                                              const targets_t &entry) const
    {
      generate_rays_simple(result, entry);
    }

    void SourceDisk::set_limits(const math::Vector2& limit1,
                                const math::Vector2& limit2)
    {
        _limit1 = limit1;
        _limit2 = limit2;
    }

  }

}

