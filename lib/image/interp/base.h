/*
   Copyright 2008 Brain Research Institute, Melbourne, Australia

   Written by J-Donald Tournier, 27/06/08.

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

#ifndef __image_interp_base_h__
#define __image_interp_base_h__

#include "point.h"
#include "image/transform.h"

namespace MR
{
  namespace Image
  {

    /** \defgroup interp Interpolation
     @{ */

    namespace Interp
    {

      //! Base class for interpolator classes

      template <class InfoType> 
        class Base : public InfoType
      {
        public:

          using InfoType::dim;
          using InfoType::vox;
          using InfoType::transform;

          //! construct an Base object to obtain interpolated values using the
          // parent DataSet class
          Base (const InfoType& parent) :
            InfoType (parent), 
            out_of_bounds (true)
          {
            bounds[0] = dim(0) - 0.5;
            bounds[1] = dim(1) - 0.5;
            bounds[2] = dim(2) - 0.5;

            Math::Matrix<float> M (4,4);
            set_matrix (S2V, Transform::scanner2voxel (M, *this));
            set_matrix (V2S, Transform::voxel2scanner (M, *this));
            set_matrix (I2S, Transform::image2scanner (M, *this));
            set_matrix (S2I, Transform::scanner2image (M, *this));
          }

          //! test whether current position is within bounds.
          /*! \return true if the current position is out of bounds, false otherwise */
          bool  operator! () const {
            return out_of_bounds;
          }

          //! Transform the position \p r from scanner-space to voxel-space
          Point<float> scanner2voxel (const Point<float>& r) const {
            return transform (S2V, r);
          }
          //! Transform the position \p r from voxel-space to scanner-space
          Point<float> voxel2scanner (const Point<float>& r) const {
            return transform (V2S, r);
          }
          //! Transform the position \p r from image-space to voxel-space
          Point<float> image2voxel (const Point<float>& r) const {
            return Point<float> (r[0]/vox(0), r[1]/vox(1), r[2]/vox(2));
          }
          //! Transform the position \p r from voxel-space to image-space
          Point<float> voxel2image (const Point<float>& r) const {
            return Point<float> (r[0]*vox(0), r[1]*vox(1), r[2]*vox(2));
          }
          //! Transform the position \p r from image-space to scanner-space
          Point<float> image2scanner (const Point<float>& r) const {
            return transform (I2S, r);
          }
          //! Transform the position \p r from scanner-space to image-space
          Point<float> scanner2image (const Point<float>& r) const {
            return transform (S2I, r);
          }

          //! Transform the orientation \p r from scanner-space to voxel-space
          Point<float> scanner2voxel_dir (const Point<float>& r) const {
            return transform_vector (S2V, r);
          }
          //! Transform the orientation \p r from voxel-space to scanner-space
          Point<float> voxel2scanner_dir (const Point<float>& r) const {
            return transform_vector (V2S, r);
          }

          const float* image2scanner_matrix () const {
            return *I2S;
          }
          const float* scanner2image_matrix () const {
            return *S2I;
          }
          const float* voxel2scanner_matrix () const {
            return *V2S;
          }
          const float* scanner2voxel_matrix () const {
            return *S2V;
          }

        protected:
          float  S2V[3][4], V2S[3][4], I2S[3][4], S2I[3][4];
          float  bounds[3];
          bool   out_of_bounds;

          Point<float> transform (const float M[3][4], const Point<float>& p) const {
            return Point<float> (
                M[0][0]*p[0] + M[0][1]*p[1] + M[0][2]*p[2] + M[0][3],
                M[1][0]*p[0] + M[1][1]*p[1] + M[1][2]*p[2] + M[1][3],
                M[2][0]*p[0] + M[2][1]*p[1] + M[2][2]*p[2] + M[2][3]);
          }

          Point<float> transform_vector (const float M[3][4], const Point<float>& p) const {
            return Point<float> (
                M[0][0]*p[0] + M[0][1]*p[1] + M[0][2]*p[2],
                M[1][0]*p[0] + M[1][1]*p[1] + M[1][2]*p[2],
                M[2][0]*p[0] + M[2][1]*p[1] + M[2][2]*p[2]);
          }

          template <class U> void set_matrix (float M[3][4], const Math::Matrix<U>& MV) {
            M[0][0] = MV (0,0);
            M[0][1] = MV (0,1);
            M[0][2] = MV (0,2);
            M[0][3] = MV (0,3);
            M[1][0] = MV (1,0);
            M[1][1] = MV (1,1);
            M[1][2] = MV (1,2);
            M[1][3] = MV (1,3);
            M[2][0] = MV (2,0);
            M[2][1] = MV (2,1);
            M[2][2] = MV (2,2);
            M[2][3] = MV (2,3);
          }

          Point<float> set (const Point<float>& pos) {
            if (pos[0] < -0.5 || pos[0] > bounds[0] ||
                pos[1] < -0.5 || pos[1] > bounds[1] ||
                pos[2] < -0.5 || pos[2] > bounds[2]) {
              out_of_bounds = true;
              return Point<float> (NAN, NAN, NAN);
            }

            out_of_bounds = false;

            return Point<float> (pos[0]-Math::floor (pos[0]), pos[1]-Math::floor (pos[1]), pos[2]-Math::floor (pos[2]));
          }
      };

    }
    //! @}
  }
}

#endif
