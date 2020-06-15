#include "octree.h"
#include <queue>
#include <array>
#include <assert.h>
#include <iostream>
#include "algorithms.h"
#include "Surface.h"
using namespace algorithm;

OctreeNode::OctreeNode(Vec3 bound[2], int maxSize)
	:bound_{bound[0],bound[1]}, maxSize_(maxSize)
{
}

OctreeNode::~OctreeNode() {

}

bool OctreeNode::hit(const Ray& ray, double t0, double t1, HitRecord* rec) const
{
	//�������ӽڵ㣬ֻ���ǽڵ㱾�������object
	//�������ļ����������н���
	double bt0, bt1;
	if (!algorithm::hitBox(bound_, ray, t0, t1, &bt0, &bt1)) return false;
	double tempt = bt1;
	bool flag = false;
	for (const auto& obj : objects_) {
		HitRecord temprec;
		if (obj->calHit(ray, bt0, bt1, &temprec)) {
			flag = true;
			if (temprec.t <= tempt) {
				tempt = temprec.t;
				*rec = temprec;
			}
		}
	}
	return flag;
}

bool OctreeNode::isLeaf() const
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
			int tot = objects_.size();
			int count = 0;
			for (auto it = objects_.begin(); count != tot; count++) {
				std::shared_ptr<AbstructNode> obj(*it);
				it = objects_.erase(it);
				insertObject(obj); //���²���Ԫ�أ�Ԫ�ؿ��ܱ��嵽�ӽڵ��У�Ҳ��������ԭ��
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
		children_[i].reset(new OctreeNode(box, maxSize_));
	}
}

ImplicitSurfaceOctree::ImplicitSurfaceOctree(StdImpicitFunc func, Point_t seedPoints[2], double minSize)
	:Octree(), minSize_(minSize), sfunc_(func), seedPoints_{ seedPoints[0],seedPoints[1] }
{
}

//bool ImplicitSurfaceOctree::hit(const Ray& ray, double t0, double t1, HitRecord * rec)
//{
//	////ע�⣬ǰ��hit���Ŀ���û��hit�������ϣ�����ֻ�����һ��cube��hit
//	//if (!includeSurface_) return false;
//	//std::queue<std::shared_ptr<ImplicitSurfaceOctreeNode>> nodes;
//	//nodes.push(std::dynamic_pointer_cast<ImplicitSurfaceOctreeNode>(root_));
//	//std::vector<double> results;
//	//while (!nodes.empty())
//	//{
//	//	auto node = nodes.front();
//	//	nodes.pop();
//	//	HitRecord temprec;
//	//	if (node->hit(ray, t0, t1, &temprec)) { //�������node�������ӽڵ����queue�������Ϊ���һ�㣬����Ϊ��ѡ���
//	//		bool empty=true;
//	//		for (int i = 0; i < 8; i++) {
//	//			if (node->getChild(i)) {
//	//				nodes.push(node->getChild(i));
//	//				empty = false;
//	//			}
//	//		}
//	//		if (empty) results.push_back(temprec.t);
//	//	}
//	//	//���û���У���ֱ�ӴӶ�����ȥ���ڵ�
//	//}
//	//if (results.empty()) {
//	//	return false;
//	//}
//	//else {
//	//	if (rec) rec->t = *std::min_element(results.begin(), results.end());
//	//	return true;
//	//}
//}

int ImplicitSurfaceOctree::build()
{
	Vec3 unit = (seedPoints_[1] - seedPoints_[0]).normalize();
	Vec3 sp = seedPoints_[0];
	auto sfun = sfunc_;
	auto fun = [sp,unit, sfun](double t) {
		Vec3 pt = unit * t + sp;
		return sfun(pt);
	};
	double root;
	int ret = algorithm::calSingleRoot(fun, 0, (seedPoints_[1] - seedPoints_[0]).length(), &root);
	if (ret) return -1;

	//ȷ����һ�����ӵ�Ԫ
	Vec3 midp = unit * root + sp;
	BoundBox_t originBox;
	originBox[0] = midp - Vec3(minSize_,minSize_,minSize_);
	originBox[1] = midp + Vec3(minSize_, minSize_, minSize_);

	struct cmp {
		bool operator()(std::shared_ptr<BoxNode> lhs, std::shared_ptr<BoxNode> rhs) const{
			return *lhs < *rhs;
		}
	};

	std::set<std::shared_ptr<BoxNode>, cmp> boxv;
	std::queue<std::shared_ptr<BoxNode>> boxq;
	std::queue<std::array<double,8>> pointValue; //��¼��Χ�нڵ��8����ĺ���ֵ

	auto poriginbox = std::shared_ptr<BoxNode>(new BoxNode(originBox));
	boxv.insert(poriginbox);
	boxq.push(poriginbox);
	pointValue.push(std::array<double, 8>());

	BoundBox_t box{Vec3(INFINITE,INFINITE,INFINITE),Vec3(-INFINITE,-INFINITE,-INFINITE) };

	auto isSameSign = [](std::array<double, 8> a) {
		bool pflag = 1;
		bool nflag = 1;
		for (int i = 0; i < 8; i++) {
			if (a[i] > 0) nflag = 0;
			if (a[i] < 0) pflag = 0;
		}
		return pflag || nflag;
	};
	//�����װ�Χ�нڵ��8����ĺ���ֵ
	for (int i = 0; i < 8; i++) {
		pointValue.back()[i] = sfunc_(algorithm::getBoundPoint(i, originBox));
	}
	
	assert(!isSameSign(pointValue.back()));

	while (!boxq.empty())
	{
		std::shared_ptr<BoxNode> node = boxq.front();
		BoundBox_t nodebox;
		node->getBoundBox(nodebox);

		//��������İ�Χ��  //TODO y�İ�Χ�к���������
		box[0].x_ = std::min(box[0].x_, nodebox[0].x_);
		box[0].y_ = std::min(box[0].y_, nodebox[0].y_);
		box[0].z_ = std::min(box[0].z_, nodebox[0].z_);
		box[1].x_ = std::max(box[1].x_, nodebox[1].x_);
		box[1].y_ = std::max(box[1].y_, nodebox[1].y_);
		box[1].z_ = std::max(box[1].z_, nodebox[1].z_);

		std::array<double, 8> pv(std::move(pointValue.back()));
		pointValue.pop();
		boxq.pop();
		//TODO �Ż�����ÿ��������һ����ά�������������й�ϣ������ÿ���ڵ�ĺ���ֵ�������ظ�����ڵ㺯��ֵ
		//����ÿ�����ڰ�Χ�У������û�б����ϰ���������������������棬����Ҫ������кͼ���
		for (int i = 0; i < 6; i++) {
			BoundBox_t adjbox; 
			getAdjBox(nodebox,BoxFace(i),adjbox);
			std::shared_ptr<BoxNode> pbox(new BoxNode(adjbox));
			if (boxv.find(pbox) != boxv.end()) continue;
			std::array<double, 8> pointv;
			for (int i = 0; i < 8; i++) {
				pointv[i] = sfunc_(algorithm::getBoundPoint(i, adjbox));
			}
			if (!isSameSign(pointv)) {  //�������������棬��Ҫ���뼯�������
				int s1 = boxv.size();
				//std::cout << adjbox[0] << std::endl;
				//std::cout << std::endl;
				auto it = boxv.insert( pbox);
				//std::cout << std::endl;
				cmp cmp;
				//if ((boxv.find(pbox) == boxv.end())) {
				//	std::cout << std::endl;
				//	boxv.find(pbox);
				//	std::cout << std::endl;
				//}
				//std::cout << std::endl << std::endl << (boxv.find(pbox) != boxv.end())  << std::endl;
				assert(s1 != boxv.size());
				boxq.push(pbox);
				pointValue.push(std::move(pointv));
			}
		}
	}

	setBoundBox(box);
	Octree::build();
	for (const auto& node : boxv) {
		insertObject(node);
	}
	return 0;
}

Octree::Octree(int maxSize, BoundBox_t bound)
	:maxSize_(maxSize)
{
	if (bound) {
		bound_[0] = bound[0]; 
		bound_[1] = bound[1];
	}
}

int Octree::build()
{
	root_.reset(new OctreeNode(bound_, maxSize_));
	return 0;
}

void Octree::insertObject(std::shared_ptr<AbstructNode> objects_)
{
	root_->insertObject(objects_);
}

int Octree::setBoundBox(BoundBox_t bound)
{
	if (bound) {
		bound_[0] = bound[0];
		bound_[1] = bound[1];
	}
	return 0;
}

bool Octree::hit(const Ray& ray, double t0, double t1, HitRecord * rec) const
{
	//TODO ���˺Ͱ�Χ��hit֮�⣬��Ҫhit�ڲ�Ԫ��
	//DFS ��ջʵ��
	BoundBox_t rootbox;
	root_->getBound(rootbox);
	if (!algorithm::hitBox(rootbox, ray, t0, t1, nullptr)) return false;
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

	//TODO �����10Ӧ���滻Ϊ������
	stack.reserve(10);
	childlog.reserve(10);
	index.reserve(10);

	stack.push_back(root_);
	childlog.push_back(std::vector<ChildHit>());
	index.push_back(-1);

	HitRecord minrec;
	minrec.t = t1 + 1;
	int countt = 0;
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
					if (algorithm::hitBox(box, ray, t0, t1, &temprec)) {
						childlog.back().push_back({ i,temprec.t });
					}
				}
				std::sort(childlog.back().begin(), childlog.back().end(), cmp);
				//assert(!(childlog.back().empty()));
				//std::cout << ++countt << std::endl;
			}
		}
		else { //���ӽڵ����������߸տ�ʼ�����ӽڵ�
			//index.back() + 1 ����һ��Ҫ�������ӽڵ��� childlog.back() �е����
			index.back()++;
			if (index.back() == childlog.back().size()) { //���Ϸ���
				index.pop_back();
				childlog.pop_back();
				stack.pop_back();
			}
			else{
				auto next = childlog.back()[index.back()];
				if (minrec.t < next.t) {  //��һ�������ӽڵ�İ�Χ��tֵ�Ѿ�������Сtֵ��û�м�����Ҫ
					break;
				}
				else { //������һ���ӽڵ�
					stack.push_back(stack.back()->getChild(next.childindex));
					childlog.push_back(std::vector<ChildHit>());
					childlog.back().reserve(8);
					index.push_back(-1);
				}
			}
		}
	}
	if (minrec.t <= t1) {
		if (rec) *rec = minrec;
		return true;
	}
	return false;
}