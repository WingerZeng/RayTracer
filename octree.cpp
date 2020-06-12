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
	//TODO ���˺Ͱ�Χ��hit֮�⣬��Ҫhit�ڲ�Ԫ��
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
	//ע�⣬ǰ��hit���Ŀ���û��hit�������ϣ�����ֻ�����һ��cube��hit
	if (!includeSurface_) return false;
	std::queue<std::shared_ptr<ImplicitSurfaceOctreeNode>> nodes;
	nodes.push(std::dynamic_pointer_cast<ImplicitSurfaceOctreeNode>(root_));
	std::vector<double> results;
	while (!nodes.empty())
	{
		auto node = nodes.front();
		nodes.pop();
		HitRecord temprec;
		if (node->hit(ray, t0, t1, &temprec)) { //�������node�������ӽڵ����queue�������Ϊ���һ�㣬����Ϊ��ѡ���
			bool empty=true;
			for (int i = 0; i < 8; i++) {
				if (node->getChild(i)) {
					nodes.push(node->getChild(i));
					empty = false;
				}
			}
			if (empty) results.push_back(temprec.t);
		}
		//���û���У���ֱ�ӴӶ�����ȥ���ڵ�
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
		//��ʼ�İ�Χ�оͲ���������
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
