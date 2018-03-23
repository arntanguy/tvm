#pragma once

/* Copyright 2017 CNRS-UM LIRMM, CNRS-AIST JRL
 *
 * This file is part of TVM.
 *
 * TVM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TVM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with TVM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <tvm/api.h>
#include <tvm/defs.h>

#include <tvm/robot/Frame.h>

namespace tvm
{

namespace robot
{

  /** Represent a contact between two frames
   *
   * The node forwards the signals of each frame.
   *
   * Outputs:
   * - F1Position/F1Jacobian/F1Velocity/F1NormalAcceleration: proxy for the
   *   first frame Position/Jacobian/Velocity/NormalAcceleration signals
   * - F2Position/F2Jacobian/F2Velocity/F2NormalAcceleration: proxy for the
   *   second frame Position/Jacobian/Velocity/NormalAcceleration signals
   *
   */
  class TVM_DLLAPI Contact : public graph::abstract::Node<Contact>
  {
  public:
    /** Uniquely identifies a contact */
    struct Id
    {
      /** First robot name */
      std::string r1;
      /** First robot's frame name */
      std::string f1;
      /** Second robot name */
      std::string r2;
      /** Second robot's frame name */
      std::string f2;
      /** Allow to distinguish the same contact */
      int ambiguityId;
      /** Comparison to use Id in map/set */
      inline bool operator<(const Id & rhs) const
      {
        return r1 < rhs.r1 ||
          ( r1 == rhs.r1 && f1 < rhs.f1 ) ||
          ( r1 == rhs.r1 && f1 == rhs.f1 && r2 < rhs.r2 ) ||
          ( r1 == rhs.r1 && f1 == rhs.f1 && r2 == rhs.r2 && f2 < rhs.f2 ) ||
          ( r1 == rhs.r1 && f1 == rhs.f1 && r2 == rhs.f2 && f2 == rhs.f2 && ambiguityId < rhs.ambiguityId );
      }
      inline bool operator==(const Id & rhs) const
      {
        return ( r1 == rhs.r1 && f1 == rhs.f1 && r2 == rhs.r2 && f2 == rhs.f2 && ambiguityId == rhs.ambiguityId );
      }
    };

    /** Allows to view a contact from one frame perspective */
    struct View
    {
      const Id & id;
      const FramePtr & f;
      const std::vector<sva::PTransformd> & points;
    };

    SET_OUTPUTS(Contact,
                F1Position, F1Jacobian, F1Velocity, F1NormalAcceleration,
                F2Position, F2Jacobian, F2Velocity, F2NormalAcceleration
    )

    /** Constructor
     *
     * Represent a contact between frame f1 and f2, the contacts points are
     * provided in f1 frame. Importantly, it assumes the frame position are
     * already known.
     *
     * \param f1 Contact frame f1
     *
     * \param f2 Contact frame f2
     *
     * \param points Contact points expressed in f1
     *
     * \param ambiguityId If the "same" contact must exist multiple times
     *
     */
    Contact(FramePtr f1, FramePtr f2,
            std::vector<sva::PTransformd> points,
            int ambiguityId = 0);

    /** Return the first frame in the contact */
    inline const Frame & f1() const { return *f1_; }

    /** Return the second frame in the contact */
    inline const Frame & f2() const { return *f2_; }

    /** Return the contact points in f1 */
    inline const std::vector<sva::PTransformd> & f1Points() const { return f1Points_; }

    /** Initial transformation between frame 1 and frame 2 */
    inline const sva::PTransformd & X_f1_f2() const { return X_f1_f2_; }

    /** Initial transformation between frame 2 and frame 1 */
    inline const sva::PTransformd & X_f2_f1() const { return X_f2_f1_; }

    /** Return the contact points in f2 */
    inline const std::vector<sva::PTransformd> & f2Points() const { return f2Points_; }

    /** Return a contact view from f1 perspective */
    inline const View f1View() const { return {id_, f1_, f1Points_}; };

    /** Return a contact view from f2 perspective */
    inline const View f2View() const { return {id_, f2_, f2Points_}; };

    /** Return the contact's unique id */
    inline const Id & id() const { return id_; }
  private:
    FramePtr f1_;
    FramePtr f2_;
    sva::PTransformd X_f1_f2_;
    sva::PTransformd X_f2_f1_;
    std::vector<sva::PTransformd> f1Points_;
    std::vector<sva::PTransformd> f2Points_;
    Id id_;
  };

  using ContactPtr = std::shared_ptr<Contact>;
}

}

/** Output stream operator for Contact::Id */
std::ostream & operator<<(std::ostream & os, const tvm::robot::Contact::Id & c);