/*
   Copyright 2012 Brain Research Institute, Melbourne, Australia

   Written by David Raffelt, 24/02/2012

   This file is part of MRtrix.

   MRtrix is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   MRtrix is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MRtrix.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef __registration_metric_difference_robust_h__
#define __registration_metric_difference_robust_h__

#include "registration/metric/m_estimators.h"

namespace MR
{
  namespace Registration
  {
    namespace Metric
    {
      template<class Estimator = L2>
        class DifferenceRobust {
          public:
            DifferenceRobust(Estimator est) : estimator(est) {}

            template <class Params>
              default_type operator() (Params& params,
                                       const Eigen::Vector3 im1_point,
                                       const Eigen::Vector3 im2_point,
                                       const Eigen::Vector3 midway_point,
                                       Eigen::Matrix<default_type, Eigen::Dynamic, 1>& gradient) {

                typename Params::Im1ValueType im1_value;
                typename Params::Im2ValueType im2_value;
                Eigen::Matrix<typename Params::Im1ValueType, 1, 3> im1_grad;
                Eigen::Matrix<typename Params::Im2ValueType, 1, 3> im2_grad;

                params.im1_image_interp->value_and_gradient (im1_value, im1_grad);
                if (isnan (default_type (im1_value)))
                  return 0.0;
                params.im2_image_interp->value_and_gradient (im2_value, im2_grad);
                if (isnan (default_type (im2_value)))
                  return 0.0;

                default_type residual, grad;
                estimator((default_type) im1_value - (default_type) im2_value, residual, grad);
                const auto jacobian_vec = params.transformation.get_jacobian_vector_wrt_params (midway_point);
                const  Eigen::Vector3d g = grad * (im1_grad + im2_grad);
                gradient.segment<4>(0) += g(0) * jacobian_vec;
                gradient.segment<4>(4) += g(1) * jacobian_vec;
                gradient.segment<4>(8) += g(2) * jacobian_vec;

                return residual;
            }

            Estimator estimator;
        };
    }
  }
}
#endif
