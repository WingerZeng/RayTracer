#include "octree.h"
#include <queue>
#include <array>
#include "algorithms.h"
OctreeNode::OctreeNode(Vec3 bound[2], int maxSize)
	:bound_{bound[0],bound[1]}, maxSize_(maxSize)
{
}

OctreeNode::~OctreeNode() {

}

bool OctreeNode::hit(const Ray& ray, double t0, double t1, HitRecord* rec)
{
	//不计算子节点，只考虑节点本身包含的object
	//整颗树的计算在树类中进行
	double tempt = t1;
	bool flag = false;
	for (const auto& obj : objects_) {
		HitRecord temprec;
		if (obj->calHit(ray, t0, tempt, &temprec)) {
			flag = true;
			if (temprec.t <= tempt) {
				tempt = temprec.t;
				*rec = temprec;
			}
		}
	}
	return flag;
}

bool OctreeNode::isLeaf()
{
	return !full;
}

//ImplicitSurfaceOctreeNode::ImplicitSurfaceOctreeNode(Vec3 bound[2], StdImpicitFunc func)
//	:OctreeNode(bound),sfunc_(func)
//{
//}
//
//void ImplicitSurfaceOctreeNode::addChild(int index, std::shared_ptr<ImplicitSurfaceOctreeNode> child)
//{
//	impchildren_[index] = child;
//	children_[index] = child;
//}
//
//std::shared_ptr<ImplicitSurfaceOctreeNode> ImplicitSurfaceOctreeNode::getChild(int index)
//{
//	return impchildren_[index];
//}

void OctreeNode::insertObject(std::shared_ptr<AbstructNode> object)
{
	if (!full) {
		objects_.push_back(object);
		if (objects_.size() == maxSize_) { //刚好达到max，建立子节点
			full = true;
			createChildren();
			for (auto it = objects_.begin(); it != objects_.end(); it++) {
				std::shared_ptr<AbstructNode> obj(*it);
				it = objects_.erase(it);
				insertObject(object); //重新插入元素，元素可能被插到子节点中，也可能留在原地
			}
		}
	}
	else {
		bool a[8];
		int count = 0;
		for (int i = 0; i < 8; i++) {
			BoundBox_t box1, box2;
			object->getBoundBox(box1);
			getChildBound(i, box2);
			if (algorithm::isBoundBoxOverLap(box1, box2)) {
				a[i] = true;
				count++;
			}
			else a[i] = false;
		}
		if (count > OverlapTolerance) {
			objects_.push_back(object);
		}
		else {
			for (int i = 0; i < 8; i++) {
				if (a[i]) children_[i]->insertObject(object);
			}
		}
	}
}

void OctreeNode::createChildren()
{
	for (int i = 0; i < 8; i++) {
		BoundBox_t box;
		getChildBound(i, box);
		children_[0].reset(new OctreeNode(box, maxSize_));
	}
}

ImplicitSurfaceOctree::ImplicitSurfaceOctree(Vec3 bound[2], StdImpicitFunc func)
	:sfunc_(func)
{
	//root_.reset(new ImplicitSurfaceOctreeNode(bound, func));
}

bool ImplicitSurfaceOctree::hit(const Ray& ray, double t0, double t1, HitRecord * rec)
{
	////注意，前面hit到的可能没有hit到表面上，所以只看最后一层cube的hit
	//if (!includeSurface_) return false;
	//std::queue<std::shared_ptr<ImplicitSurfaceOctreeNode>> nodes;
	//nodes.push(std::dynamic_pointer_cast<ImplicitSurfaceOctreeNode>(root_));
	//std::vector<double> results;
	//while (!nodes.empty())
	//{
	//	auto node = nodes.front();
	//	nodes.pop();
	//	HitRecord temprec;
	//	if (node->hit(ray, t0, t1, &temprec)) { //如果击中node，则将其子节点加入queue，如果其为最后一层，则作为候选结果
	//		bool empty=true;
	//		for (int i = 0; i < 8; i++) {
	//			if (node->getChild(i)) {
	//				nodes.push(node->getChild(i));
	//				empty = false;
	//			}
	//		}
	//		if (empty) results.push_back(temprec.t);
	//	}
	//	//如果没击中，则直接从队列中去除节点
	//}
	//if (results.empty()) {
	//	return false;
	//}
	//else {
	//	if (rec) rec->t = *std::min_element(results.begin(), results.end());
	//	return true;
	//}
}

void ImplicitSurfaceOctree::build()
{
	//std::queue<std::shared_ptr<ImplicitSurfaceOctreeNode>> nodes;
	//nodes.push(std::dynamic_pointer_cast<ImplicitSurfaceOctreeNode>(root_));
	//if (!nodes.front()->isIntersectSurface()) {
	//	//初始的包围盒就不包含曲面
	//	includeSurface_ = false;
	//	return;
	//}
	//while (!nodes.empty())
	//{
	//	auto node = nodes.front();
	//	nodes.pop();
	//	if (node->getSize().length() <= minSize_) continue;
	//	for (int i = 0; i < 8; i++) {
	//		Vec3 bound[2];
	//		node->getChildBound(i, bound);
	//		std::shared_ptr<ImplicitSurfaceOctreeNode> newnode(new ImplicitSurfaceOctreeNode(bound, sfunc_));
	//		if (newnode->isIntersectSurface()) {
	//			node->addChild(i, newnode);
	//			nodes.push(newnode);
	//		}
	//	}
	//}
}

Octree::Octree(int maxSize)
	:maxSize_(maxSize)
{
}

bool Octree::hit(const Ray& ray, double t0, double t1, HitRecord * rec)
{
	//TODO 除了和包围盒hit之外，还要hit内部元素
	//DFS 用栈实现
	struct ChildHit {
		int childindex;
		double t;
	};
	auto cmp = [](const ChildHit& lhs, const ChildHit& rhs) {
		return lhs.t < rhs.t;
	};
	std::vector<std::shared_ptr<OctreeNode>> stack;
	std::vector<std::vector<ChildHit>> childlog; //记录当前节点有哪些child还需遍历
	std::vector<int> index;
	stack.push_back(root_);
	childlog.push_back(std::vector<ChildHit>());
	index.push_back(-1);
	HitRecord minrec;
	minrec.t = t1 + 1;
	while (!stack.empty()) {
		if (childlog.back().empty()) {  //当前节点还未处理
			//先对节点本身求交
			HitRecord temprec;
			bool flag = false;
			if (stack.back()->hit(ray, t0, t1, &temprec)) {
				if (temprec.t < minrec.t) {
					minrec = temprec;
					flag = true;
				}
			}
			if (stack.back()->isLeaf()) { 
				if (flag) {//如果是叶结点且t最小，则说明t最小的就是该节点
					break;
				}
				else {//否则说明有更小t的祖先节点，放弃该节点
					index.pop_back();
					childlog.pop_back();
					stack.pop_back();
				}
			}
			else { //如果不是叶结点，则要继续向子节点遍历
				for (int i = 0; i < 8; i++) { //判断哪些子节点被hit
					BoundBox_t box;
					stack.back()->getChildBound(i, box);
					HitRecord temprec;
					if (algorithm::hitBox(box, ray, t0, t1, &temprec)) {
						childlog.back().push_back({ i,temprec.t });
					}
				}
				std::sort(childlog.back().begin(), childlog.back().end(), cmp);
				assert(childlog.back().empty());
			}
		}
		else { //从子节点上来，或者刚开始遍历子节点
			//index.back() + 1 是下一个要遍历的子节点在 childlog.back() 中的序号
			index.back()++;
			ChildHit next = childlog.back[index.back()];
			if (index.back() == childlog.back().size()) { //向上返回
				index.pop_back();
				childlog.pop_back();
				stack.pop_back();
			}
			else if (minrec.t < next.t) {  //下一个遍历子节点的包围盒t值已经大于最小t值，没有继续必要
				break;
			}
			else { //遍历下一个子节点
				stack.push_back(stack.back()->getChild(next.childindex));
				childlog.push_back(std::vector<ChildHit>());
				index.push_back(-1);
			}
		}
	}
	if (minrec.t <= t1) {
		if (rec) *rec = minrec;
		return true;
	}
	return false;
}
