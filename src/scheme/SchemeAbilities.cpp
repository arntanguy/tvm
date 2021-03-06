/* Copyright 2017-2018 CNRS-AIST JRL and CNRS-UM LIRMM
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#include <tvm/scheme/internal/SchemeAbilities.h>

#include <tvm/constraint/abstract/Constraint.h>
#include <tvm/requirements/SolvingRequirements.h>

#include <iostream>
#include <sstream>

namespace tvm
{

namespace scheme
{

namespace internal
{

    static int GeneralLevel = -1;
    static int NoLimit = GeneralLevel;

    LevelAbilities::LevelAbilities(bool inequality, const std::vector<requirements::ViolationEvaluationType>& types)
      : inequalities_(inequality)
      , evaluationTypes_(types)
    {
    }

    void LevelAbilities::check(const ConstraintPtr& c, const SolvingRequirementsPtr& req, bool /*emitWarnings*/) const
    {
      //checking the constraint type
      if (c->type() != constraint::Type::EQUAL && !inequalities_)
        throw std::runtime_error("This level does not handle inequality constraints.");

      //checking the evaluation type
      auto it = std::find(evaluationTypes_.begin(), evaluationTypes_.end(), req->violationEvaluation().value());
      if (it == evaluationTypes_.end())
        throw std::runtime_error("This level does not handle the required violation evaluation value.");
    }

    SchemeAbilities::SchemeAbilities(int numberOfLevels, const std::map<int, LevelAbilities>& abilities, bool scalarization)
      : numberOfLevels_(numberOfLevels)
      , scalarization_(scalarization)
      , abilities_(abilities)
    {
      assert(GeneralLevel < 0 && "Implementation of this class assumes that GeneralLevel is non positive.");
      assert(GeneralLevel == NoLimit && "Implementation of this class assumes that GeneralLevel is equal to NoLimit.");

      if (numberOfLevels < 0 && numberOfLevels != NoLimit)
        throw std::runtime_error("Incorrect number of levels. This number must be nonnegative or equal to NoLimit");

      if (numberOfLevels >= 0)
      {
        // if there is not general entry in abilities, then we check that each level has its own entry.
        if (abilities.count(GeneralLevel) == 0)
        {
          for (int i = 0; i < numberOfLevels; ++i)
          {
            if (abilities.count(i) == 0)
            {
              std::stringstream s;
              s << "No abilities given for level " << i << "." << std::endl;
              throw std::runtime_error(s.str());
            }
          }
        }
      }
      else
      {
        //we check that there is a general entry in abilities
        if (abilities.count(GeneralLevel) == 0)
          throw std::runtime_error("No general level abilities given.");
      }
    }

    void SchemeAbilities::check(const ConstraintPtr& c, const SolvingRequirementsPtr& req, bool emitWarnings) const
    {
      //check priority level value
      int p = req->priorityLevel().value();
      if (numberOfLevels_ > 0 && p >= numberOfLevels_)
      {
        if (scalarization_)
        {
          if (emitWarnings)
          {
            std::cout << "Warning: required priority level (" << p
                      << ") cannot be handled as a strict hierarchy level by the resolution scheme. "
                      << "Using scalarization to revert to weighted approach.";
          }
        }
        else
        {
          std::stringstream s;
          s << "This resolution scheme can handle priorities level up to" << numberOfLevels_ - 1
            << ". It cannot process required level (" << p << ")." << std::endl;
          throw std::runtime_error(s.str());
        }
      }

      //check abilities of the required priority level
      auto it = abilities_.find(p);
      if (it != abilities_.end())
        it->second.check(c, req, emitWarnings);
      else
        abilities_.at(GeneralLevel).check(c, req, emitWarnings);
    }

}  // namespace internal

}  // namespace scheme

}  // namespace tvm
