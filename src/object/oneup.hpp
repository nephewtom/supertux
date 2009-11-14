//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __ONEUP_H__
#define __ONEUP_H__

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"
#include "supertux/direction.hpp"

class OneUp : public MovingSprite, private UsesPhysic
{
public:
  OneUp(const Vector& pos, Direction direction = RIGHT);
  virtual OneUp* clone() const { return new OneUp(*this); }

  virtual void update(float elapsed_time);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
};

#endif