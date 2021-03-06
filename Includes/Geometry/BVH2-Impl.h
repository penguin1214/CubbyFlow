/*************************************************************************
> File Name: BVH2-Impl.h
> Project Name: CubbyFlow
> Author: Chan-Ho Chris Ohk
> Purpose: Bounding Volume Hierarchy (BVH) in 2D.
> Created Time: 2017/10/17
> Copyright (c) 2018, Chan-Ho Chris Ohk
*************************************************************************/
#ifndef CUBBYFLOW_BVH2_IMPL_H
#define CUBBYFLOW_BVH2_IMPL_H

#include <numeric>

namespace CubbyFlow
{
	template <typename T>
	BVH2<T>::Node::Node() : flags(0)
	{
		child = std::numeric_limits<size_t>::max();
	}

	template <typename T>
	void BVH2<T>::Node::InitLeaf(size_t it, const BoundingBox2D& b)
	{
		flags = 2;
		item = it;
		bound = b;
	}

	template <typename T>
	void BVH2<T>::Node::InitInternal(uint8_t axis, size_t c, const BoundingBox2D& b)
	{
		flags = axis;
		child = c;
		bound = b;
	}

	template <typename T>
	bool BVH2<T>::Node::IsLeaf() const
	{
		return flags == 2;
	}

	template <typename T>
	BVH2<T>::BVH2()
	{
		// Do nothing
	}

	template <typename T>
	void BVH2<T>::Build(const std::vector<T>& items,
		const std::vector<BoundingBox2D>& itemsBounds)
	{
		m_items = items;
		m_itemBounds = itemsBounds;

		if (m_items.empty())
		{
			return;
		}

		m_nodes.clear();

		for (size_t i = 0; i < m_items.size(); ++i)
		{
			m_bound.Merge(m_itemBounds[i]);
		}

		std::vector<size_t> itemIndices(m_items.size());
		std::iota(std::begin(itemIndices), std::end(itemIndices), 0);

		Build(0, itemIndices.data(), m_items.size(), 0);
	}

	template <typename T>
	void BVH2<T>::Clear()
	{
		m_bound = BoundingBox2D();
		m_items.clear();
		m_itemBounds.clear();
		m_nodes.clear();
	}

	template <typename T>
	inline NearestNeighborQueryResult2<T> BVH2<T>::GetNearestNeighbor(
		const Vector2D& pt,
		const NearestNeighborDistanceFunc2<T>& distanceFunc) const
	{
		NearestNeighborQueryResult2<T> best;
		best.distance = std::numeric_limits<double>::max();
		best.item = nullptr;

		// Prepare to traverse BVH
		static const int maxTreeDepth = 8 * sizeof(size_t);
		const Node* todo[maxTreeDepth];
		size_t todoPos = 0;

		// Traverse BVH nodes
		const Node* node = m_nodes.data();
		while (node != nullptr)
		{
			if (node->IsLeaf())
			{
				double dist = distanceFunc(m_items[node->item], pt);
				if (dist < best.distance)
				{
					best.distance = dist;
					best.item = &m_items[node->item];
				}

				// Grab next node to process from todo stack
				if (todoPos > 0)
				{
					// Dequeue
					--todoPos;
					node = todo[todoPos];
				}
				else
				{
					break;
				}
			}
			else
			{
				const double bestDistSqr = best.distance * best.distance;

				const Node* left = node + 1;
				const Node* right = &m_nodes[node->child];

				// If pt is inside the box, then the closestLeft and Right will be
				// identical to pt. This will make distMinLeftSqr and
				// distMinRightSqr zero, meaning that such a box will have higher
				// priority.
				Vector2D closestLeft = left->bound.Clamp(pt);
				Vector2D closestRight = right->bound.Clamp(pt);

				double distMinLeftSqr = closestLeft.DistanceSquaredTo(pt);
				double distMinRightSqr = closestRight.DistanceSquaredTo(pt);

				bool shouldVisitLeft = distMinLeftSqr < bestDistSqr;
				bool shouldVisitRight = distMinRightSqr < bestDistSqr;

				const Node* firstChild;
				const Node* secondChild;

				if (shouldVisitLeft && shouldVisitRight)
				{
					if (distMinLeftSqr < distMinRightSqr)
					{
						firstChild = left;
						secondChild = right;
					}
					else
					{
						firstChild = right;
						secondChild = left;
					}

					// Enqueue secondChild in todo stack
					todo[todoPos] = secondChild;
					++todoPos;
					node = firstChild;
				}
				else if (shouldVisitLeft)
				{
					node = left;
				}
				else if (shouldVisitRight)
				{
					node = right;
				}
				else
				{
					if (todoPos > 0)
					{
						// Dequeue
						--todoPos;
						node = todo[todoPos];
					}
					else
					{
						break;
					}
				}
			}
		}

		return best;
	}

	template <typename T>
	inline bool BVH2<T>::IsIntersects(const BoundingBox2D& box,
		const BoxIntersectionTestFunc2<T>& testFunc) const
	{
		if (!m_bound.Overlaps(box))
		{
			return false;
		}

		// prepare to traverse BVH for box
		static const int kMaxTreeDepth = 8 * sizeof(size_t);
		const Node* todo[kMaxTreeDepth];
		size_t todoPos = 0;

		// traverse BVH nodes for box
		const Node* node = m_nodes.data();

		while (node != nullptr)
		{
			if (node->IsLeaf())
			{
				if (testFunc(m_items[node->item], box))
				{
					return true;
				}

				// grab next node to process from todo stack
				if (todoPos > 0)
				{
					// Dequeue
					--todoPos;
					node = todo[todoPos];
				}
				else
				{
					break;
				}
			}
			else
			{
				// get node children pointers for box
				const Node* firstChild = node + 1;
				const Node* secondChild = const_cast<Node*>(&m_nodes[node->child]);

				// advance to next child node, possibly enqueue other child
				if (!firstChild->bound.Overlaps(box))
				{
					node = secondChild;
				}
				else if (!secondChild->bound.Overlaps(box))
				{
					node = firstChild;
				}
				else
				{
					// enqueue secondChild in todo stack
					todo[todoPos] = secondChild;
					++todoPos;
					node = firstChild;
				}
			}
		}

		return false;
	}

	template <typename T>
	inline bool BVH2<T>::IsIntersects(const Ray2D& ray,
		const RayIntersectionTestFunc2<T>& testFunc) const
	{
		if (!m_bound.Intersects(ray))
		{
			return false;
		}

		// prepare to traverse BVH for ray
		static const int maxTreeDepth = 8 * sizeof(size_t);
		const Node* todo[maxTreeDepth];
		size_t todoPos = 0;

		// traverse BVH nodes for ray
		const Node* node = m_nodes.data();

		while (node != nullptr)
		{
			if (node->IsLeaf())
			{
				if (testFunc(m_items[node->item], ray))
				{
					return true;
				}

				// grab next node to process from todo stack
				if (todoPos > 0)
				{
					// Dequeue
					--todoPos;
					node = todo[todoPos];
				}
				else
				{
					break;
				}
			}
			else
			{
				// get node children pointers for ray
				const Node* firstChild;
				const Node* secondChild;
				
				if (ray.direction[node->flags] > 0.0)
				{
					firstChild = node + 1;
					secondChild = const_cast<Node*>(&m_nodes[node->child]);
				}
				else
				{
					firstChild = const_cast<Node*>(&m_nodes[node->child]);
					secondChild = node + 1;
				}

				// advance to next child node, possibly enqueue other child
				if (!firstChild->bound.Intersects(ray))
				{
					node = secondChild;
				}
				else if (!secondChild->bound.Intersects(ray))
				{
					node = firstChild;
				}
				else
				{
					// enqueue secondChild in todo stack
					todo[todoPos] = secondChild;
					++todoPos;
					node = firstChild;
				}
			}
		}

		return false;
	}

	template <typename T>
	inline void BVH2<T>::ForEachIntersectingItem(
		const BoundingBox2D& box, const BoxIntersectionTestFunc2<T>& testFunc,
		const IntersectionVisitorFunc2<T>& visitorFunc) const
	{
		if (!m_bound.Overlaps(box))
		{
			return;
		}

		// prepare to traverse BVH for box
		static const int maxTreeDepth = 8 * sizeof(size_t);
		const Node* todo[maxTreeDepth];
		size_t todoPos = 0;

		// traverse BVH nodes for box
		const Node* node = m_nodes.data();

		while (node != nullptr)
		{
			if (node->IsLeaf())
			{
				if (testFunc(m_items[node->item], box))
				{
					visitorFunc(m_items[node->item]);
				}

				// grab next node to process from todo stack
				if (todoPos > 0)
				{
					// Dequeue
					--todoPos;
					node = todo[todoPos];
				}
				else
				{
					break;
				}
			}
			else
			{
				// get node children pointers for box
				const Node* firstChild = node + 1;
				const Node* secondChild = const_cast<Node*>(&m_nodes[node->child]);

				// advance to next child node, possibly enqueue other child
				if (!firstChild->bound.Overlaps(box))
				{
					node = secondChild;
				}
				else if (!secondChild->bound.Overlaps(box))
				{
					node = firstChild;
				}
				else
				{
					// enqueue secondChild in todo stack
					todo[todoPos] = secondChild;
					++todoPos;
					node = firstChild;
				}
			}
		}
	}

	template <typename T>
	inline void BVH2<T>::ForEachIntersectingItem(
		const Ray2D& ray, const RayIntersectionTestFunc2<T>& testFunc,
		const IntersectionVisitorFunc2<T>& visitorFunc) const
	{
		if (!m_bound.Intersects(ray))
		{
			return;
		}

		// prepare to traverse BVH for ray
		static const int maxTreeDepth = 8 * sizeof(size_t);
		const Node* todo[maxTreeDepth];
		size_t todoPos = 0;

		// traverse BVH nodes for ray
		const Node* node = m_nodes.data();

		while (node != nullptr)
		{
			if (node->IsLeaf())
			{
				if (testFunc(m_items[node->item], ray))
				{
					visitorFunc(m_items[node->item]);
				}

				// grab next node to process from todo stack
				if (todoPos > 0)
				{
					// Dequeue
					--todoPos;
					node = todo[todoPos];
				}
				else
				{
					break;
				}
			}
			else
			{
				// get node children pointers for ray
				const Node* firstChild;
				const Node* secondChild;

				if (ray.direction[node->flags] > 0.0)
				{
					firstChild = node + 1;
					secondChild = const_cast<Node*>(&m_nodes[node->child]);
				}
				else
				{
					firstChild = const_cast<Node*>(&m_nodes[node->child]);
					secondChild = node + 1;
				}

				// advance to next child node, possibly enqueue other child
				if (!firstChild->bound.Intersects(ray))
				{
					node = secondChild;
				}
				else if (!secondChild->bound.Intersects(ray))
				{
					node = firstChild;
				}
				else
				{
					// enqueue secondChild in todo stack
					todo[todoPos] = secondChild;
					++todoPos;
					node = firstChild;
				}
			}
		}
	}

	template <typename T>
	inline ClosestIntersectionQueryResult2<T> BVH2<T>::GetClosestIntersection(
		const Ray2D& ray, const GetRayIntersectionFunc2<T>& testFunc) const
	{
		ClosestIntersectionQueryResult2<T> best;
		best.distance = std::numeric_limits<double>::max();
		best.item = nullptr;

		if (!m_bound.Intersects(ray))
		{
			return best;
		}

		// prepare to traverse BVH for ray
		static const int maxTreeDepth = 8 * sizeof(size_t);
		const Node* todo[maxTreeDepth];
		size_t todoPos = 0;

		// traverse BVH nodes for ray
		const Node* node = m_nodes.data();

		while (node != nullptr)
		{
			if (node->IsLeaf())
			{
				double dist = testFunc(m_items[node->item], ray);
				if (dist < best.distance)
				{
					best.distance = dist;
					best.item = m_items.data() + node->item;
				}

				// grab next node to process from todo stack
				if (todoPos > 0)
				{
					// Dequeue
					--todoPos;
					node = todo[todoPos];
				}
				else
				{
					break;
				}
			}
			else
			{
				// get node children pointers for ray
				const Node* firstChild;
				const Node* secondChild;

				if (ray.direction[node->flags] > 0.0)
				{
					firstChild = node + 1;
					secondChild = const_cast<Node*>(&m_nodes[node->child]);
				}
				else
				{
					firstChild = const_cast<Node*>(&m_nodes[node->child]);
					secondChild = node + 1;
				}

				// advance to next child node, possibly enqueue other child
				if (!firstChild->bound.Intersects(ray))
				{
					node = secondChild;
				}
				else if (!secondChild->bound.Intersects(ray))
				{
					node = firstChild;
				}
				else
				{
					// enqueue secondChild in todo stack
					todo[todoPos] = secondChild;
					++todoPos;
					node = firstChild;
				}
			}
		}

		return best;
	}

	template <typename T>
	const BoundingBox2D& BVH2<T>::GetBoundingBox() const
	{
		return m_bound;
	}

	template <typename T>
	typename BVH2<T>::Iterator BVH2<T>::begin()
	{
		return m_items.begin();
	}

	template <typename T>
	typename BVH2<T>::Iterator BVH2<T>::end()
	{
		return m_items.end();
	}

	template <typename T>
	typename BVH2<T>::ConstIterator BVH2<T>::begin() const
	{
		return m_items.begin();
	}

	template <typename T>
	typename BVH2<T>::ConstIterator BVH2<T>::end() const
	{
		return m_items.end();
	}

	template <typename T>
	size_t BVH2<T>::GetNumberOfItems() const
	{
		return m_items.size();
	}

	template <typename T>
	const T& BVH2<T>::GetItem(size_t i) const
	{
		return m_items[i];
	}

	template <typename T>
	size_t BVH2<T>::Build(size_t nodeIndex, size_t* itemIndices, size_t nItems, size_t currentDepth)
	{
		// add a node
		m_nodes.push_back(Node());

		// initialize leaf node if termination criteria met
		if (nItems == 1)
		{
			m_nodes[nodeIndex].InitLeaf(itemIndices[0], m_itemBounds[itemIndices[0]]);
			return currentDepth + 1;
		}

		// find the mid-point of the bounding box to use as a qsplit pivot
		BoundingBox2D nodeBound;
		for (size_t i = 0; i < nItems; ++i)
		{
			nodeBound.Merge(m_itemBounds[itemIndices[i]]);
		}

		Vector2D d = nodeBound.upperCorner - nodeBound.lowerCorner;

		// choose which axis to split along
		uint8_t axis;
		if (d.x > d.y)
		{
			axis = 0;
		}
		else
		{
			axis = 1;
		}

		double pivot = 0.5 * (nodeBound.upperCorner[axis] + nodeBound.lowerCorner[axis]);

		// classify primitives with respect to split
		size_t midPoint = QSplit(itemIndices, nItems, pivot, axis);

		// recursively initialize children m_nodes
		size_t d0 = Build(nodeIndex + 1, itemIndices, midPoint, currentDepth + 1);
		m_nodes[nodeIndex].InitInternal(axis, m_nodes.size(), nodeBound);
		size_t d1 = Build(m_nodes[nodeIndex].child, itemIndices + midPoint, nItems - midPoint, currentDepth + 1);

		return std::max(d0, d1);
	}

	template <typename T>
	size_t BVH2<T>::QSplit(size_t* itemIndices, size_t numItems, double pivot, uint8_t axis)
	{
		size_t ret = 0;

		for (size_t i = 0; i < numItems; ++i)
		{
			BoundingBox2D b = m_itemBounds[itemIndices[i]];
			double centroid = 0.5f * (b.lowerCorner[axis] + b.upperCorner[axis]);
			
			if (centroid < pivot)
			{
				std::swap(itemIndices[i], itemIndices[ret]);
				ret++;
			}
		}

		if (ret == 0 || ret == numItems)
		{
			ret = numItems >> 1;
		}

		return ret;
	}
}

#endif