//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include <cassert>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <math.h>

#include <SDL.h>
#include <SDL_image.h>

#include "gameconfig.hpp"
#include "physfs/physfs_sdl.hpp"
#include "video/surface.hpp"
#include "video/drawing_context.hpp"
#include "video/color.hpp"
#include "image_texture.hpp"
#include "texture_manager.hpp"

Surface::Surface(const std::string& file)
{
  texture = texture_manager->get(file);
  texture->ref();
  uv_left = 0.5 / texture->get_width();
  uv_top = 0.5 / texture->get_height();
  uv_right = texture->get_uv_right();
  uv_bottom = texture->get_uv_bottom();

  width = texture->get_image_width();
  height = texture->get_image_height();
}

Surface::Surface(const std::string& file, int x, int y, int w, int h)
{
  texture = texture_manager->get(file);
  texture->ref();

  float tex_w = static_cast<float> (texture->get_width());
  float tex_h = static_cast<float> (texture->get_height());
  uv_left = static_cast<float>(x+0.5) / tex_w;
  uv_top = static_cast<float>(y+0.5) / tex_h;
  uv_right = static_cast<float>(x+w-0.5) / tex_w;
  uv_bottom = static_cast<float>(y+h-0.5) / tex_h;

  width = w;
  height = h;
}

Surface::Surface(const Surface& other)
{
  texture = other.texture;
  texture->ref();

  uv_left = other.uv_left;
  uv_top = other.uv_top;
  uv_right = other.uv_right;
  uv_bottom = other.uv_bottom;
  width = other.width;
  height = other.height;
}

const Surface&
Surface::operator= (const Surface& other)
{
  other.texture->ref();
  texture->unref();
  texture = other.texture;

  uv_left = other.uv_left;
  uv_top = other.uv_top;
  uv_right = other.uv_right;
  uv_bottom = other.uv_bottom;
  width = other.width;
  height = other.height;

  return *this;
}

Surface::~Surface()
{
  texture->unref();
}

void
Surface::hflip()
{
  std::swap(uv_left, uv_right);
}

static inline void intern_draw(float left, float top, float right, float bottom,                               float uv_left, float uv_top,
                               float uv_right, float uv_bottom,
                               DrawingEffect effect)
{
  if(effect & HORIZONTAL_FLIP)
    std::swap(uv_left, uv_right);
  if(effect & VERTICAL_FLIP) {
    std::swap(uv_top, uv_bottom);
  }

  glBegin(GL_QUADS);
  glTexCoord2f(uv_left, uv_top);
  glVertex2f(left, top);

  glTexCoord2f(uv_right, uv_top);
  glVertex2f(right, top);

  glTexCoord2f(uv_right, uv_bottom);
  glVertex2f(right, bottom);

  glTexCoord2f(uv_left, uv_bottom);
  glVertex2f(left, bottom);
  glEnd();
}

static inline void intern_draw2(float left, float top, float right, float bottom,
                                float uv_left, float uv_top,
                                float uv_right, float uv_bottom,
                                float angle,
                                const Color& color,
                                const Blend& blend,
                                DrawingEffect effect)
{
  if(effect & HORIZONTAL_FLIP)
    std::swap(uv_left, uv_right);
  if(effect & VERTICAL_FLIP) {
    std::swap(uv_top, uv_bottom);
  }

  float center_x = (left + right) / 2;
  float center_y = (top + bottom) / 2;

  float sa = sinf(angle/180.0f*M_PI);
  float ca = cosf(angle/180.0f*M_PI);

  left  -= center_x;
  right -= center_x;

  top    -= center_y;
  bottom -= center_y;

  glBlendFunc(blend.sfactor, blend.dfactor);
  glColor4f(color.red, color.green, color.blue, color.alpha);
  glBegin(GL_QUADS);
  glTexCoord2f(uv_left, uv_top);
  glVertex2f(left*ca - top*sa + center_x,
             left*sa + top*ca + center_y);

  glTexCoord2f(uv_right, uv_top);
  glVertex2f(right*ca - top*sa + center_x,
             right*sa + top*ca + center_y);

  glTexCoord2f(uv_right, uv_bottom);
  glVertex2f(right*ca - bottom*sa + center_x,
             right*sa + bottom*ca + center_y);

  glTexCoord2f(uv_left, uv_bottom);
  glVertex2f(left*ca - bottom*sa + center_x,
             left*sa + bottom*ca + center_y);
  glEnd();

  // FIXME: find a better way to restore the blend mode
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
Surface::draw(float x, float y, float alpha, float angle, const Color& color, const Blend& blend, DrawingEffect effect) const
{
  glColor4f(1.0f, 1.0f, 1.0f, alpha);
  glBindTexture(GL_TEXTURE_2D, texture->get_handle());

  intern_draw2(x, y,
               x + width, y + height,
               uv_left, uv_top, uv_right, uv_bottom,
               angle,
               color,
               blend,
               effect);
}

void
Surface::draw(float x, float y, float alpha, DrawingEffect effect) const
{
  glColor4f(1.0f, 1.0f, 1.0f, alpha);
  glBindTexture(GL_TEXTURE_2D, texture->get_handle());

  intern_draw(x, y,
              x + width, y + height,
              uv_left, uv_top, uv_right, uv_bottom, effect);
}

void
Surface::draw_part(float src_x, float src_y, float dst_x, float dst_y,
                   float width, float height, float alpha,
                   DrawingEffect effect) const
{
  float uv_width = uv_right - uv_left;
  float uv_height = uv_bottom - uv_top;

  float uv_left = this->uv_left + (uv_width * src_x) / this->width;
  float uv_top = this->uv_top + (uv_height * src_y) / this->height;
  float uv_right = this->uv_left + (uv_width * (src_x + width)) / this->width;
  float uv_bottom = this->uv_top + (uv_height * (src_y + height)) / this->height;

  glColor4f(1.0f, 1.0f, 1.0f, alpha);
  glBindTexture(GL_TEXTURE_2D, texture->get_handle());

  intern_draw(dst_x, dst_y,
              dst_x + width, dst_y + height,
              uv_left, uv_top, uv_right, uv_bottom, effect);
}
