#include "octree.h"
#include <queue>
OctreeNode::OctreeNode(Vec3 bound[2])
	:bound_{bound[0],bound[1]}
{
}

OctreeNode::~OctreeNode() {

}

bool OctreeNode::hit(Ray ray, double t0, double t1, HitRecord* rec)
{
	//TODO 除了和包围盒hit之外，还要hit内部元素
	return hit(bound_, ray, t0, t1, rec);
}

ImplicitSurfaceOctreeNode::ImplicitSurfaceOctreeNode(Vec3 bound[2], StdImpicitFunc func)
	:OctreeNode(bound),sfunc_(func)
{
}

void ImplicitSurfaceOctreeNode::addChild(int index, std::shared_ptr<ImplicitSurfaceOctreeNode> child)
{
	impchildren_[index] = child;
	children_[index] = child;
}

std::shared_ptr<ImplicitSurfaceOctreeNode> ImplicitSurfaceOctreeNode::getChild(int index)
{
	return impchildren_[index];
}

ImplicitSurfaceOctree::ImplicitSurfaceOctree(Vec3 bound[2], StdImpicitFunc func)
	:sfunc_(func)
{
	root_.reset(new ImplicitSurfaceOctreeNode(bound, func));
}

bool ImplicitSurfaceOctree::hit(Ray ray, double t0, double t1, HitRecord * rec)
{
	//注意，前面hit到的可能没有hit到表面上，所以只看最后一层cube的hit
	if (!includeSurface_) return false;
	std::queue<std::shared_ptr<ImplicitSurfaceOctreeNode>> nodes;
	nodes.push(std::dynamic_pointer_cast<ImplicitSurfaceOctreeNode>(root_));
	std::vector<double> results;
	while (!nodes.empty())
	{
		auto node = nodes.front();
		nodes.pop();
		HitRecord temprec;
		if (node->hit(ray, t0, t1, &temprec)) { //如果击中node，则将其子节点加入queue，如果其为最后一层，则作为候选结果
			bool empty=true;
			for (int i = 0; i < 8; i++) {
				if (node->getChild(i)) {
					nodes.push(node->getChild(i));
					empty = false;
				}
			}
			if (empty) results.push_back(temprec.t);
		}
		//如果没击中，则直接从队列中去除节点
	}
	if (results.empty()) {
		return false;
	}
	else {
		if (rec) rec->t = *std::min_element(results.begin(), results.end());
		return true;
	}
}

void ImplicitSurfaceOctree::build()
{
	std::queue<std::shared_ptr<ImplicitSurfaceOctreeNode>> nodes;
	nodes.push(std::dynamic_pointer_cast<ImplicitSurfaceOctreeNode>(root_));
	if (!nodes.front()->isIntersectSurface()) {
		//初始的包围盒就不包含曲面
		includeSurface_ = false;
		return;
	}
	while (!nodes.empty())
	{
		auto node = nodes.front();
		nodes.pop();
		if (node->getSize().length() <= minSize_) continue;
		for (int i = 0; i < 8; i++) {
			Vec3 bound[2];
			node->getChildBound(i, bound);
			std::shared_ptr<ImplicitSurfaceOctreeNode> newnode(new ImplicitSurfaceOctreeNode(bound, sfunc_));
			if (newnode->isIntersectSurface()) {
				node->addChild(i, newnode);
				nodes.push(newnode);
			}
		}
	}
}
