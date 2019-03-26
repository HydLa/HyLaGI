#pragma once
#include "Logger.h"
#include "NonlinearFunctionApproximator.h"
#include "NonlinearTwoVariableFunction.h"
#include "Parser.h"
#include <fstream>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <iostream>

#include <boost/accumulators/statistics/sum.hpp>
#include <boost/accumulators/statistics/sum_kahan.hpp>

using namespace boost::accumulators;
int func()
{
  accumulator_set<double, stats<tag::sum_kahan>> acc;
  const double initial = sum_kahan(acc);
  std::cout << initial << std::endl;
  for (size_t i = 0; i < 1e6; ++i)
  {
    acc(1e-6f);
  }
  const double result = sum_kahan(acc);
  std::cout << result << std::endl;
}

namespace hydla
{
namespace interval
{
using parser::double_to_node;

class TwoVariableApproximator
{
 public:
  TwoVariableApproximator() = default;

  void makeVanDerPol(double& theta1_Left, double& theta1_Right, double& theta2_Left, double& theta2_Right, int quality)
  {
    /*
    boost::accumulators::accumulator_set<float, boost::accumulators::stats<boost::accumulators::tag::sum_kahan>> acc;
    acc(1.0f);
    const float val = boost::accumulators::tag::sum_kahan(acc);
    std::cout << val;
    */

    /*accumulator_set<double, stats<tag::sum_kahan>> acc;
    acc(1.0);
    const double val = sum_kahan(acc);
    std::cout << val;*/

    const auto func = [](const afd& p, const afd& q)
    {
      const double K = 0.25;
      return K * (1.0 - q * q) * p - q;
    };

    QuadTree reference(theta1_Left, theta1_Right, theta2_Left, theta2_Right, func);
    {
      const int centiQuality = (quality * 100) / 100;
      afd::maxnum() = 0;
      for (int i = 1; i < quality * 100; ++i)
      {
        reference.stepDivision(func);
        if ((i - 1) / centiQuality != i / centiQuality)
        {
          std::cout << "precalculation1 progress: " << (i / centiQuality) << "%" << std::endl;
        }
      };
    }

    //std::cout << "BREAK 1" << std::endl;
    approxY1 = QuadTree(theta1_Left, theta1_Right, theta2_Left, theta2_Right, func);

    const int centiQuality = quality / 100;
    //std::cout << "BREAK 2" << std::endl;
    afd::maxnum() = 0;
    for (int i = 1; i < quality; ++i)
    {
      //std::cout << "BREAK 3" << std::endl;
      approxY1.stepDivision(func);
      if ((i - 1) / centiQuality != i / centiQuality)
      {
        std::cout << "precalculation2 progress: " << (i / centiQuality) << "%" << std::endl;
      }
    };

    {
      //std::ofstream ofs("dump_quadtree_initial");
      //approxY1.dump(ofs);
    }

    std::cout << "optimize quadtree: " << std::endl;
    OptimizeQuadTree(reference, approxY1);

    {
      //std::ofstream ofs("dump_quadtree_optimized");
      //approxY1.dump(ofs);
    }
  }

  std::vector<DynamicModule> makeUpdateConstraints() const
  {
    std::vector<DynamicModule> result;
    if (approxY1.isLeaf())
    {
      return result;
    }

    std::cout << "BREAK makeUpdateConstraints" << std::endl;

    const auto indices1 = approxY1.getAllElements();

    const std::string positionVar = "position1";
    const std::string widthVar = "w";
    const std::string var1 = "theta1";
    const std::string var2 = "theta2";

    const int centiIndices = indices1.size() / 100;

    for (size_t i = 0; i < indices1.size(); ++i)
    {
      const QuadTree& currentElem = approxY1.access(indices1[i]);

      simulator::constraint_t currentConstraint = NewConstraint(NewAlways(
          NewAsk(
              //position1- = i
              NewEqual(NewPrev(NewVar(positionVar)), NewNum(std::to_string(i))),
              //var2' = var1*currentElem.m_a1 + var2*currentElem.m_a2 + currentElem.m_b.lower() + width(currentElem.m_b)*w
              NewEqual(
                  NewD(NewVar(var2)),
                  NewAdd(
                      NewAdd(
                          //var1 * currentElem.m_a1
                          //NewMul(NewVar(var1), NewNum(std::to_string(currentElem.m_a1))),
                          NewMul(NewVar(var1), double_to_node(currentElem.m_a1)),
                          //var2 * currentElem.m_a2
                          //NewMul(NewVar(var2), NewNum(std::to_string(currentElem.m_a2)))),
                          NewMul(NewVar(var2), double_to_node(currentElem.m_a2))),
                      NewAdd(
                          //currentElem.m_b.lower()
                          //NewNum(std::to_string(currentElem.m_b.lower())),
                          double_to_node(currentElem.m_b.lower()),
                          //width(currentElem.m_b) * w
                          //NewMul(NewNum(std::to_string(width(currentElem.m_b))), NewVar(widthVar))))))));
                          NewMul(double_to_node(width(currentElem.m_b)), NewVar(widthVar))))))));
      HYDLA_LOGGER_DEBUG("BREAK NonlinearUpdate ", i, " : ", get_infix_string(currentConstraint));
      result.emplace_back(std::string("NonlinearUpdate") + std::to_string(i), currentConstraint);

      if ((i - 1) / centiIndices != i / centiIndices)
      {
        std::cout << "make updates progress: " << (i / centiIndices) << "%" << std::endl;
      }
    }

    return result;
  }

  std::pair<std::vector<FunctionBounds>, std::vector<std::vector<node_sptr>>> makeJumpConstraints() const
  {
    std::vector<FunctionBounds> result;
    std::vector<std::vector<node_sptr>> resultGuards;
    if (approxY1.isLeaf())
    {
      return {result, resultGuards};
    }

    /*
    const auto getIndex = [](const std::vector<Accessor>& indices, const Accessor& elem) -> size_t {
      for (size_t i = 0; i < indices.size(); ++i)
      {
        if (IsSame(elem, indices[i]))
        {
          return i;
        }
      }
      exit(1);
    };
*/
    const auto indices1 = approxY1.getAllElements();
    const int centiIndices = indices1.size() / 100;

    std::unordered_map<Accessor, size_t> indexTable;
    for (size_t i = 0; i < indices1.size(); ++i)
    {
      indexTable[indices1[i]] = i;
      if ((i - 1) / centiIndices != i / centiIndices)
      {
        std::cout << "make indices map: " << (i / centiIndices) << "%" << std::endl;
      }
    }

    const std::string positionVar = "position1";
    const std::string widthVar = "w";
    const std::string var1 = "theta1";
    const std::string var2 = "theta2";

    for (size_t i = 0; i < indices1.size(); ++i)
    {
      const QuadTree& currentElem = approxY1.access(indices1[i]);

      FunctionBounds currentModules;
      std::vector<node_sptr> currentGuards;

      //theta2 == bottom
      {
        // std::cout << "BottomAdjacent" << std::endl;
        const auto b = approxY1.BottomAdjacent(indices1[i]);
        // std::cout << "ok" << std::endl;
        if (!b.empty())
        {
          //std::cout << "interpretAccessor" << std::endl;
          const auto sideAccessors = approxY1.interpretAccessor(b, QuadTree::TopSide);
          //std::cout << "ok" << std::endl;
          const bool hasBranch = 2 <= sideAccessors.size();
          for (const auto& accessor : sideAccessors)
          {
            //const size_t sideIndex = getIndex(indices1, accessor);
            //std::cout << "indexTable" << std::endl;
            const size_t sideIndex = indexTable[accessor];
            //std::cout << "ok" << std::endl;
            const auto& sideElem = approxY1.access(accessor);

            /*simulator::constraint_t constraint =
                hasBranch ? NewConstraint(NewAlways(
                                NewAsk(
                                    NewAnd(
                                        NewAnd(
                                            //position1 -= i
                                            NewEqual(NewPrev(NewVar(positionVar)), NewNum(std::to_string(i))),
                                            //theta2- = currentElem.v2Lower
                                            NewEqual(NewPrev(NewVar(var2)), NewNum(std::to_string(currentElem.v2Lower)))),
                                        NewAnd(
                                            //theta1- >= sideElem.v1Lower
                                            NewGreaterEq(NewPrev(NewVar(var1)), NewNum(std::to_string(sideElem.v1Lower))),
                                            //theta1- <= sideElem.v1Upper
                                            NewLessEq(NewPrev(NewVar(var1)), NewNum(std::to_string(sideElem.v1Upper))))),
                                    //position1 = sideIndex
                                    NewEqual(
                                        NewVar(positionVar),
                                        NewNum(std::to_string(sideIndex))))))
                          : NewConstraint(NewAlways(
                                NewAsk(
                                    NewAnd(
                                        //position1 -= i
                                        NewEqual(NewPrev(NewVar(positionVar)), NewNum(std::to_string(i))),
                                        //theta2- = currentElem.v2Lower
                                        NewEqual(NewPrev(NewVar(var2)), NewNum(std::to_string(currentElem.v2Lower)))),
                                    //position1 = sideIndex
                                    NewEqual(
                                        NewVar(positionVar),
                                        NewNum(std::to_string(sideIndex))))));*/

            //position1 -= i
            auto atomicGuard1 = NewEqual(NewPrev(NewVar(positionVar)), NewNum(std::to_string(i)));
            //theta2- = currentElem.v2Lower
            //auto atomicGuard2 = NewEqual(NewPrev(NewVar(var2)), NewNum(std::to_string(currentElem.v2Lower)));
            auto atomicGuard2 = NewEqual(NewPrev(NewVar(var2)), double_to_node(currentElem.v2Lower));

            //theta1- >= sideElem.v1Lower
            //auto atomicGuard3 = NewGreaterEq(NewPrev(NewVar(var1)), NewNum(std::to_string(sideElem.v1Lower)));
            auto atomicGuard3 = NewGreaterEq(NewPrev(NewVar(var1)), double_to_node(sideElem.v1Lower));
            //theta1- <= sideElem.v1Upper
            //auto atomicGuard4 = NewLessEq(NewPrev(NewVar(var1)), NewNum(std::to_string(sideElem.v1Upper)));
            auto atomicGuard4 = NewLessEq(NewPrev(NewVar(var1)), double_to_node(sideElem.v1Upper));

            simulator::constraint_t guard =
                hasBranch ? NewAnd(NewAnd(atomicGuard1, atomicGuard2), NewAnd(atomicGuard3, atomicGuard4))
                          : NewAnd(atomicGuard1, atomicGuard2);

            simulator::constraint_t constraint =
                NewConstraint(NewAlways(
                    NewAsk(
                        guard,
                        //position1 = sideIndex
                        NewEqual(
                            NewVar(positionVar),
                            NewNum(std::to_string(sideIndex))))));

            currentModules.add(DynamicModule(std::string("NonlinearJump") + std::to_string(i) + "_TO_" + std::to_string(sideIndex), constraint));

            currentGuards.push_back(atomicGuard2);
            if (hasBranch)
            {
              currentGuards.push_back(atomicGuard3);
              currentGuards.push_back(atomicGuard4);
            }

            HYDLA_LOGGER_DEBUG("BREAK NonlinearJump(B) ", i, " : ", get_infix_string(constraint));
          }
        }
      }

      //theta2 == top
      {
        //std::cout << "TopAdjacent" << std::endl;
        const auto t = approxY1.TopAdjacent(indices1[i]);
        //std::cout << "ok" << std::endl;
        if (!t.empty())
        {
          //std::cout << "interpretAccessor" << std::endl;
          const auto sideAccessors = approxY1.interpretAccessor(t, QuadTree::BottomSide);
          //std::cout << "ok" << std::endl;
          const bool hasBranch = 2 <= sideAccessors.size();
          for (const auto& accessor : sideAccessors)
          {
            //const size_t sideIndex = getIndex(indices1, accessor);
            //std::cout << "indexTable" << std::endl;
            const size_t sideIndex = indexTable[accessor];
            //std::cout << "ok" << std::endl;
            const auto& sideElem = approxY1.access(accessor);

            /*simulator::constraint_t constraint =
                hasBranch ? NewConstraint(NewAlways(
                                NewAsk(
                                    NewAnd(
                                        NewAnd(
                                            //position1 -= i
                                            NewEqual(NewPrev(NewVar(positionVar)), NewNum(std::to_string(i))),
                                            //theta2- = currentElem.v2Upper
                                            NewEqual(NewPrev(NewVar(var2)), NewNum(std::to_string(currentElem.v2Upper)))),
                                        NewAnd(
                                            //theta1- >= sideElem.v1Lower
                                            NewGreaterEq(NewPrev(NewVar(var1)), NewNum(std::to_string(sideElem.v1Lower))),
                                            //theta1- <= sideElem.v1Upper
                                            NewLessEq(NewPrev(NewVar(var1)), NewNum(std::to_string(sideElem.v1Upper))))),
                                    //position1 = sideIndex
                                    NewEqual(
                                        NewVar(positionVar),
                                        NewNum(std::to_string(sideIndex))))))
                          : NewConstraint(NewAlways(
                                NewAsk(
                                    NewAnd(
                                        //position1 -= i
                                        NewEqual(NewPrev(NewVar(positionVar)), NewNum(std::to_string(i))),
                                        //theta2- = currentElem.v2Upper
                                        NewEqual(NewPrev(NewVar(var2)), NewNum(std::to_string(currentElem.v2Upper)))),
                                    //position1 = sideIndex
                                    NewEqual(
                                        NewVar(positionVar),
                                        NewNum(std::to_string(sideIndex))))));*/

            //position1 -= i
            auto atomicGuard1 = NewEqual(NewPrev(NewVar(positionVar)), NewNum(std::to_string(i)));
            //theta2- = currentElem.v2Upper
            //        auto atomicGuard2 = NewEqual(NewPrev(NewVar(var2)), NewNum(std::to_string(currentElem.v2Upper)));

            //theta1- >= sideElem.v1Lower
            //          auto atomicGuard3 = NewGreaterEq(NewPrev(NewVar(var1)), NewNum(std::to_string(sideElem.v1Lower)));
            //theta1- <= sideElem.v1Upper
            //            auto atomicGuard4 = NewLessEq(NewPrev(NewVar(var1)), NewNum(std::to_string(sideElem.v1Upper)));

            //theta2- = currentElem.v2Upper
            auto atomicGuard2 = NewEqual(NewPrev(NewVar(var2)), double_to_node(currentElem.v2Upper));

            //theta1- >= sideElem.v1Lower
            auto atomicGuard3 = NewGreaterEq(NewPrev(NewVar(var1)), double_to_node(sideElem.v1Lower));
            //theta1- <= sideElem.v1Upper
            auto atomicGuard4 = NewLessEq(NewPrev(NewVar(var1)), double_to_node(sideElem.v1Upper));

            simulator::constraint_t guard =
                hasBranch ? NewAnd(NewAnd(atomicGuard1, atomicGuard2), NewAnd(atomicGuard3, atomicGuard4))
                          : NewAnd(atomicGuard1, atomicGuard2);

            simulator::constraint_t constraint =
                NewConstraint(NewAlways(
                    NewAsk(
                        guard,
                        //position1 = sideIndex
                        NewEqual(
                            NewVar(positionVar),
                            NewNum(std::to_string(sideIndex))))));

            currentModules.add(DynamicModule(std::string("NonlinearJump") + std::to_string(i) + "_TO_" + std::to_string(sideIndex), constraint));

            currentGuards.push_back(atomicGuard2);
            if (hasBranch)
            {
              currentGuards.push_back(atomicGuard3);
              currentGuards.push_back(atomicGuard4);
            }

            HYDLA_LOGGER_DEBUG("BREAK NonlinearJump(T) ", i, " : ", get_infix_string(constraint));
          }
        }
      }

      //theta1 == left
      {
        //std::cout << "LeftAdjacent" << std::endl;
        const auto l = approxY1.LeftAdjacent(indices1[i]);
        //std::cout << "ok" << std::endl;
        if (!l.empty())
        {
          //std::cout << "interpretAccessor" << std::endl;
          const auto sideAccessors = approxY1.interpretAccessor(l, QuadTree::RightSide);
          //std::cout << "ok" << std::endl;
          const bool hasBranch = 2 <= sideAccessors.size();
          for (const auto& accessor : sideAccessors)
          {
            //const size_t sideIndex = getIndex(indices1, accessor);
            //std::cout << "indexTable" << std::endl;
            const size_t sideIndex = indexTable[accessor];
            //std::cout << "ok" << std::endl;
            const auto& sideElem = approxY1.access(accessor);

            /*simulator::constraint_t constraint =
                hasBranch ? NewConstraint(NewAlways(
                                NewAsk(
                                    NewAnd(
                                        NewAnd(
                                            //position1 -= i
                                            NewEqual(NewPrev(NewVar(positionVar)), NewNum(std::to_string(i))),
                                            //theta1- = currentElem.v1Lower
                                            NewEqual(NewPrev(NewVar(var1)), NewNum(std::to_string(currentElem.v1Lower)))),
                                        NewAnd(
                                            //theta2- >= sideElem.v2Lower
                                            NewGreaterEq(NewPrev(NewVar(var2)), NewNum(std::to_string(sideElem.v2Lower))),
                                            //theta2- <= sideElem.v2Upper
                                            NewLessEq(NewPrev(NewVar(var2)), NewNum(std::to_string(sideElem.v2Upper))))),
                                    //position1 = sideIndex
                                    NewEqual(
                                        NewVar(positionVar),
                                        NewNum(std::to_string(sideIndex))))))
                          : NewConstraint(NewAlways(
                                NewAsk(
                                    NewAnd(
                                        //position1 -= i
                                        NewEqual(NewPrev(NewVar(positionVar)), NewNum(std::to_string(i))),
                                        //theta1- = currentElem.v1Lower
                                        NewEqual(NewPrev(NewVar(var1)), NewNum(std::to_string(currentElem.v1Lower)))),
                                    //position1 = sideIndex
                                    NewEqual(
                                        NewVar(positionVar),
                                        NewNum(std::to_string(sideIndex))))));*/

            //position1 -= i
            auto atomicGuard1 = NewEqual(NewPrev(NewVar(positionVar)), NewNum(std::to_string(i)));
            //theta1- = currentElem.v1Lower
            //        auto atomicGuard2 = NewEqual(NewPrev(NewVar(var1)), NewNum(std::to_string(currentElem.v1Lower)));

            //theta2- >= sideElem.v2Lower
            //          auto atomicGuard3 = NewGreaterEq(NewPrev(NewVar(var2)), NewNum(std::to_string(sideElem.v2Lower)));
            //theta2- <= sideElem.v2Upper
            //            auto atomicGuard4 = NewLessEq(NewPrev(NewVar(var2)), NewNum(std::to_string(sideElem.v2Upper)));

            //theta1- = currentElem.v1Lower
            auto atomicGuard2 = NewEqual(NewPrev(NewVar(var1)), double_to_node(currentElem.v1Lower));

            //theta2- >= sideElem.v2Lower
            auto atomicGuard3 = NewGreaterEq(NewPrev(NewVar(var2)), double_to_node(sideElem.v2Lower));
            //theta2- <= sideElem.v2Upper
            auto atomicGuard4 = NewLessEq(NewPrev(NewVar(var2)), double_to_node(sideElem.v2Upper));

            simulator::constraint_t guard =
                hasBranch ? NewAnd(NewAnd(atomicGuard1, atomicGuard2), NewAnd(atomicGuard3, atomicGuard4))
                          : NewAnd(atomicGuard1, atomicGuard2);

            simulator::constraint_t constraint =
                NewConstraint(NewAlways(
                    NewAsk(
                        guard,
                        //position1 = sideIndex
                        NewEqual(
                            NewVar(positionVar),
                            NewNum(std::to_string(sideIndex))))));

            currentModules.add(DynamicModule(std::string("NonlinearJump") + std::to_string(i) + "_TO_" + std::to_string(sideIndex), constraint));

            currentGuards.push_back(atomicGuard2);
            if (hasBranch)
            {
              currentGuards.push_back(atomicGuard3);
              currentGuards.push_back(atomicGuard4);
            }

            HYDLA_LOGGER_DEBUG("BREAK NonlinearJump(L) ", i, " : ", get_infix_string(constraint));
          }
        }
      }

      //theta1 == right
      {
        //std::cout << "RightAdjacent" << std::endl;
        const auto r = approxY1.RightAdjacent(indices1[i]);
        //std::cout << "ok" << std::endl;
        if (!r.empty())
        {
          //std::cout << "interpretAccessor" << std::endl;
          const auto sideAccessors = approxY1.interpretAccessor(r, QuadTree::LeftSide);
          //std::cout << "ok" << std::endl;
          const bool hasBranch = 2 <= sideAccessors.size();
          for (const auto& accessor : sideAccessors)
          {
            //const size_t sideIndex = getIndex(indices1, accessor);
            //std::cout << "indexTable" << std::endl;
            const size_t sideIndex = indexTable[accessor];
            //std::cout << "ok" << std::endl;
            const auto& sideElem = approxY1.access(accessor);

            /*simulator::constraint_t constraint =
                hasBranch ? NewConstraint(NewAlways(
                                NewAsk(
                                    NewAnd(
                                        NewAnd(
                                            //position1 -= i
                                            NewEqual(NewPrev(NewVar(positionVar)), NewNum(std::to_string(i))),
                                            //theta1- = currentElem.v1Upper
                                            NewEqual(NewPrev(NewVar(var1)), NewNum(std::to_string(currentElem.v1Upper)))),
                                        NewAnd(
                                            //theta2- >= sideElem.v2Lower
                                            NewGreaterEq(NewPrev(NewVar(var2)), NewNum(std::to_string(sideElem.v2Lower))),
                                            //theta2- <= sideElem.v2Upper
                                            NewLessEq(NewPrev(NewVar(var2)), NewNum(std::to_string(sideElem.v2Upper))))),
                                    //position1 = sideIndex
                                    NewEqual(
                                        NewVar(positionVar),
                                        NewNum(std::to_string(sideIndex))))))
                          : NewConstraint(NewAlways(
                                NewAsk(
                                    NewAnd(
                                        //position1 -= i
                                        NewEqual(NewPrev(NewVar(positionVar)), NewNum(std::to_string(i))),
                                        //theta1- = currentElem.v1Upper
                                        NewEqual(NewPrev(NewVar(var1)), NewNum(std::to_string(currentElem.v1Upper)))),
                                    //position1 = sideIndex
                                    NewEqual(
                                        NewVar(positionVar),
                                        NewNum(std::to_string(sideIndex))))));*/

            //position1 -= i
            auto atomicGuard1 = NewEqual(NewPrev(NewVar(positionVar)), NewNum(std::to_string(i)));
            //theta1- = currentElem.v1Upper
            //auto atomicGuard2 = NewEqual(NewPrev(NewVar(var1)), NewNum(std::to_string(currentElem.v1Upper)));

            //theta2- >= sideElem.v2Lower
            //auto atomicGuard3 = NewGreaterEq(NewPrev(NewVar(var2)), NewNum(std::to_string(sideElem.v2Lower)));
            //theta2- <= sideElem.v2Upper
            //auto atomicGuard4 = NewLessEq(NewPrev(NewVar(var2)), NewNum(std::to_string(sideElem.v2Upper)));

            //theta1- = currentElem.v1Upper
            auto atomicGuard2 = NewEqual(NewPrev(NewVar(var1)), double_to_node(currentElem.v1Upper));

            //theta2- >= sideElem.v2Lower
            auto atomicGuard3 = NewGreaterEq(NewPrev(NewVar(var2)), double_to_node(sideElem.v2Lower));
            //theta2- <= sideElem.v2Upper
            auto atomicGuard4 = NewLessEq(NewPrev(NewVar(var2)), double_to_node(sideElem.v2Upper));

            simulator::constraint_t guard =
                hasBranch ? NewAnd(NewAnd(atomicGuard1, atomicGuard2), NewAnd(atomicGuard3, atomicGuard4))
                          : NewAnd(atomicGuard1, atomicGuard2);

            simulator::constraint_t constraint =
                NewConstraint(NewAlways(
                    NewAsk(
                        guard,
                        //position1 = sideIndex
                        NewEqual(
                            NewVar(positionVar),
                            NewNum(std::to_string(sideIndex))))));

            currentModules.add(DynamicModule(std::string("NonlinearJump") + std::to_string(i) + "_TO_" + std::to_string(sideIndex), constraint));

            currentGuards.push_back(atomicGuard2);
            if (hasBranch)
            {
              currentGuards.push_back(atomicGuard3);
              currentGuards.push_back(atomicGuard4);
            }

            HYDLA_LOGGER_DEBUG("BREAK NonlinearJump(R) ", i, " : ", get_infix_string(constraint));
          }
        }
      }

      result.push_back(currentModules);
      resultGuards.push_back(currentGuards);

      if ((i - 1) / centiIndices != i / centiIndices)
      {
        std::cout << "make jumps progress: " << (i / centiIndices) << "%" << std::endl;
      }
    }

    return {result, resultGuards};
  }

  int getModuleIndex(double v1, double v2) const
  {
    const auto allElements = approxY1.getAllElements();
    const auto regionAccessor = approxY1.getRegion(v1, v2);

    for (int i = 0; i < allElements.size(); ++i)
    {
      if (IsSame(allElements[i], regionAccessor))
      {
        return i;
      }
    }

    exit(1);
  }

 private:
  QuadTree approxY1;
};

int NonlinearFunctionApproximator::init(double init_v1, double init_v2)
{
  /*
  double v1_l = 2 - 0.1;
  double v1_u = 2 + 0.12;
  double v2_l = 2 - 0.1;
  double v2_u = 2 + 0.12;*/

  double v1_l = -3;
  double v1_u = 3.1237;
  double v2_l = -3;
  double v2_u = 3.1237;

  std::cout << std::setprecision(10);

  HYDLA_LOGGER_DEBUG("Make parallelotopes");
  TwoVariableApproximator approx;
  //approx.makeVanDerPol(v1_l, v1_u, v2_l, v2_u, 10);
  //approx.makeVanDerPol(v1_l, v1_u, v2_l, v2_u, 10000);
  approx.makeVanDerPol(v1_l, v1_u, v2_l, v2_u, 100000);

  HYDLA_LOGGER_DEBUG("Make update constraints");
  updates = approx.makeUpdateConstraints();

  HYDLA_LOGGER_DEBUG("Make jump constraints");
  std::tie(jumps, guards) = approx.makeJumpConstraints();

  return approx.getModuleIndex(init_v1, init_v2);
}

}  //namespace interval
}  //namespace hydla
