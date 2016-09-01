#include "data/DecisionTreeNode.h"

#include "data/DecisionSubtreeRef.h"
#include "data/DecisionTree.h"

DecisionTreeNode::DecisionTreeNode(uint64_t nodeIdx, float bias,
                                   uint32_t dataSetFeatureIdx,
                                   uint64_t zeroFalseChildIdx,
                                   uint64_t oneTrueChildIdx)
  : NodeIdx(nodeIdx), DataSetFeatureIdx(dataSetFeatureIdx), Bias(bias),
    FalseChildNodeIdx(zeroFalseChildIdx), TrueChildNodeIdx(oneTrueChildIdx) {}

bool operator!=(const DecisionTreeNode &lhs, const DecisionTreeNode &rhs) {
  return !(lhs == rhs);
}

bool operator==(const DecisionTreeNode &lhs, const DecisionTreeNode &rhs) {
  if (lhs.NodeIdx == rhs.NodeIdx) {
    assert(lhs.isImplicit() == rhs.isImplicit());
    assert(lhs.DataSetFeatureIdx == rhs.DataSetFeatureIdx);
    assert(lhs.TrueChildNodeIdx == rhs.TrueChildNodeIdx);
    assert(lhs.FalseChildNodeIdx == rhs.FalseChildNodeIdx);
    return true;
  }

  return false;
}

bool DecisionTreeNode::isImplicit() const {
  bool allDefaulted =
      (std::isnan(Bias) && DataSetFeatureIdx == NoFeatureIdx &&
       TrueChildNodeIdx == NoNodeIdx && FalseChildNodeIdx == NoNodeIdx);

  return allDefaulted && NodeIdx != NoNodeIdx;
}

bool DecisionTreeNode::hasChildFor(NodeEvaluation evaluation) const {
  return (evaluation == NodeEvaluation::ContinueZeroLeft)
         ? this->hasLeftChild()
         : this->hasRightChild();
}

DecisionTreeNode
DecisionTreeNode::getChildFor(NodeEvaluation evaluation,
                              DecisionSubtreeRef subtree) const {
  return subtree.getNode(getChildIdxFor(evaluation));
}

uint64_t DecisionTreeNode::getChildIdxFor(NodeEvaluation evaluation) const {
  return (evaluation == NodeEvaluation::ContinueZeroLeft)
         ? FalseChildNodeIdx
         : TrueChildNodeIdx;
}
