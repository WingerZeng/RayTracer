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
	//�������ӽڵ㣬ֻ���ǽڵ㱾�������object
	//�������ļ����������н���
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
		if (objects_.size() == maxSize_) { //�պôﵽmax�������ӽڵ�
			full = true;
			createChildren();
			for (auto it = objects_.begin(); it != objects_.end(); it++) {
				std::shared_ptr<AbstructNode> obj(*it);
				it = objects_.erase(it);
				insertObject(object); //���²���Ԫ�أ�Ԫ�ؿ��ܱ��嵽�ӽڵ��У�Ҳ��������ԭ��
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
	////ע�⣬ǰ��hit���Ŀ���û��hit�������ϣ�����ֻ�����һ��cube��hit
	//if (!includeSurface_) return false;
	//std::queue<std::shared_ptr<ImplicitSurfaceOctreeNode>> nodes;
	//nodes.push(std::dynamic_pointer_cast<ImplicitSurfaceOctreeNode>(root_));
	//std::vector<double> results;
	//while (!nodes.empty())
	//{
	//	auto node = nodes.front();
	//	nodes.pop();
	//	HitRecord temprec;
	//	if (node->hit(ray, t0, t1, &temprec)) { //�������node�������ӽڵ����queue�������Ϊ���һ�㣬����Ϊ��ѡ���
	//		bool empty=true;
	//		for (int i = 0; i < 8; i++) {
	//			if (node->getChild(i)) {
	//				nodes.push(node->getChild(i));
	//				empty = false;
	//			}
	//		}
	//		if (empty) results.push_back(temprec.t);
	//	}
	//	//���û���У���ֱ�ӴӶ�����ȥ���ڵ�
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
	//	//��ʼ�İ�Χ�оͲ���������
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
	//TODO ���˺Ͱ�Χ��hit֮�⣬��Ҫhit�ڲ�Ԫ��
	//DFS ��ջʵ��
	struct ChildHit {
		int childindex;
		double t;
	};
	auto cmp = [](const ChildHit& lhs, const ChildHit& rhs) {
		return lhs.t < rhs.t;
	};
	std::vector<std::shared_ptr<OctreeNode>> stack;
	std::vector<std::vector<ChildHit>> childlog; //��¼��ǰ�ڵ�����Щchild�������
	std::vector<int> index;
	stack.push_back(root_);
	childlog.push_back(std::vector<ChildHit>());
	index.push_back(-1);
	HitRecord minrec;
	minrec.t = t1 + 1;
	while (!stack.empty()) {
		if (childlog.back().empty()) {  //��ǰ�ڵ㻹δ����
			//�ȶԽڵ㱾����
			HitRecord temprec;
			bool flag = false;
			if (stack.back()->hit(ray, t0, t1, &temprec)) {
				if (temprec.t < minrec.t) {
					minrec = temprec;
					flag = true;
				}
			}
			if (stack.back()->isLeaf()) { 
				if (flag) {//�����Ҷ�����t��С����˵��t��С�ľ��Ǹýڵ�
					break;
				}
				else {//����˵���и�Сt�����Ƚڵ㣬�����ýڵ�
					index.pop_back();
					childlog.pop_back();
					stack.pop_back();
				}
			}
			else { //�������Ҷ��㣬��Ҫ�������ӽڵ����
				for (int i = 0; i < 8; i++) { //�ж���Щ�ӽڵ㱻hit
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
		else { //���ӽڵ����������߸տ�ʼ�����ӽڵ�
			//index.back() + 1 ����һ��Ҫ�������ӽڵ��� childlog.back() �е����
			index.back()++;
			ChildHit next = childlog.back[index.back()];
			if (index.back() == childlog.back().size()) { //���Ϸ���
				index.pop_back();
				childlog.pop_back();
				stack.pop_back();
			}
			else if (minrec.t < next.t) {  //��һ�������ӽڵ�İ�Χ��tֵ�Ѿ�������Сtֵ��û�м�����Ҫ
				break;
			}
			else { //������һ���ӽڵ�
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
