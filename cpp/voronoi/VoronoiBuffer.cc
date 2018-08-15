// Copyright (c) 2010-2018 The Regents of the University of Michigan
// This file is from the freud project, released under the BSD 3-Clause License.

#include <memory>
#include <stdexcept>
#include <vector>

#include "VoronoiBuffer.h"

using namespace std;

/*! \file VoronoiBuffer.cc
    \brief Computes a buffer of particles to support wrapped positions in qhull
*/

namespace freud { namespace voronoi {

void VoronoiBuffer::compute(const vec3<float> *points,
                            const unsigned int Np,
                            const float buff)
    {
    assert(points);

    m_buffer_particles = std::shared_ptr<std::vector< vec3<float> > >(
            new std::vector< vec3<float> >());
    m_buffer_ids = std::shared_ptr<std::vector< unsigned int > >(
            new std::vector< unsigned int >());
    std::vector< vec3<float> >& buffer_parts = *m_buffer_particles;
    std::vector< unsigned int >& buffer_ids = *m_buffer_ids;

    // Get the box dimensions
    float lx = m_box.getLx();
    float ly = m_box.getLy();
    float lz = m_box.getLz();
    float xy = m_box.getTiltFactorXY();
    float xz = m_box.getTiltFactorXZ();
    float yz = m_box.getTiltFactorYZ();
  
    float lx_2_buff = 0.5*lx + buff;
    float ly_2_buff = 0.5*ly + buff;
    float lz_2_buff = 0.5*lz + buff;
    float ix = ceil(buff / lx);
    float iy = ceil(buff / ly);
    float iz = ceil(buff / lz);


    vec3<float> img;
    buffer_parts.clear();
    buffer_ids.clear();

    // for each particle
    for (unsigned int particle = 0; particle < Np; particle++)
        {
        for (int i=-ix; i<=ix; i++)
            {
            for (int j=-iy; j<=iy; j++)
                {
                if (m_box.is2D())
                    {
                    if(i != 0 || j != 0)
                        {
                        img.x = points[particle].x + i*lx + j*ly*xy;
                        img.y = points[particle].y + j*ly;
                        img.z = 0.0;
                        // Check to see if this image is within the buffer
                        float xadj = img.y*xy;
                        if(img.x < lx_2_buff + xadj && img.x > -lx_2_buff + xadj &&
                           img.y < ly_2_buff && img.y > -ly_2_buff)
                            {
                            buffer_parts.push_back(img);
                            buffer_ids.push_back(particle);
                            }
                        }
                    }
                else
                    {
                    for (int k=-iz; k<=iz; k++)
                        {
                        if(!(i==0 && j==0 && k==0))
                            {
                            img.x = points[particle].x + i*lx + j*ly*xy + k*lz*xz;
                            img.y = points[particle].y + j*ly + k*lz*yz;
                            img.z = points[particle].z + k*lz;
                            // Check to see if this image is within the buffer
                            float xadj = img.y*xy + img.z*xz;
                            float yadj = img.z*yz;
                            if(img.x < lx_2_buff  + xadj && img.x > -lx_2_buff + xadj &&
                               img.y < ly_2_buff  + yadj && img.y > -ly_2_buff + yadj &&
                               img.z < lz_2_buff && img.z > -lz_2_buff)
                                {
                                buffer_parts.push_back(img);
                                buffer_ids.push_back(particle);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}; }; // end namespace freud::voronoi
