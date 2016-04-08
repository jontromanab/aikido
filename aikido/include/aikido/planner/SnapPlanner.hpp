#ifndef AIKIDO_PLANNER_SNAP_PLANNER_HPP_
#define AIKIDO_PLANNER_SNAP_PLANNER_HPP_

#include <memory>
#include <exception>
#include <stdexcept>
#include <vector>
#include "Result.hpp"
#include "../constraint/TestableConstraint.hpp"
#include "../statespace/StateSpace.hpp"
#include "../util/VanDerCorput.hpp"

namespace aikido
{
namespace planner
{

using Trajectory = std::vector<aikido::statespace::StateSpace::ScopedState>;

static std::unique_ptr<Trajectory> planSnap(
    const aikido::statespace::StateSpace::State *startState,
    const aikido::statespace::StateSpace::State *goalState,
    const std::shared_ptr<aikido::statespace::StateSpace> stateSpace,
    const std::shared_ptr<aikido::constraint::TestableConstraint> constraint,
    aikido::planner::Result* planningResult)
{
  if (stateSpace != constraint->getStateSpace()) {
    throw std::invalid_argument(
        "StateSpace of constraint not equal to StateSpace of planning space");
  }
  aikido::util::VanDerCorput vdc{1, true, 0.02};  // TODO junk resolution
  auto testState = stateSpace->createState();
  
  for (double alpha : vdc) {
    stateSpace->interpolate(startState, goalState, alpha, testState);
    if (!constraint->isSatisfied(testState)) {
      planningResult->message = "Collision detected";
      return nullptr;  // TODO return optional?
    }
  }

  std::unique_ptr<Trajectory> trajectory(new Trajectory());
  auto startCopy = stateSpace->createState();
  auto goalCopy = stateSpace->createState();
  stateSpace->copyState(startState, startCopy);
  stateSpace->copyState(goalState, goalCopy);

  trajectory->emplace_back(std::move(startCopy));
  trajectory->emplace_back(std::move(goalCopy));
  return trajectory;
}

}  // planner
}  // aikido

#endif  // AIKIDO_PLANNER_SNAP_PLANNER_HPP_