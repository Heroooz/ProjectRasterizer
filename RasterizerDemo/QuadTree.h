#pragma once
#include <memory>
#include <vector>
#include <DirectXCollision.h>

template<typename T>
class QuadTree
{
	struct Node
	{
		std::vector<const T*> elements;
		std::unique_ptr<Node> children[4];
		DirectX::BoundingBox boundingRegion;
		int depth;
	};

	std::unique_ptr<Node> root;

	void PrintTree(std::unique_ptr<Node>& nodeToProcess);
	void AddToNode(const T* elementAddress, const DirectX::BoundingBox& boundingbox, 
		std::unique_ptr<Node>& node);
	void SubdivideNode(std::unique_ptr<Node>& node);
	void CheckNode(const DirectX::BoundingFrustum& frustum, 
		const std::unique_ptr<Node>&node, std::vector<const T*>& foundObjects);

public:
	QuadTree();
	~QuadTree() = default;

	void PrintTree();
	void AddElement(const T* elementAddress, const DirectX::BoundingBox& boundingbox);
	std::vector<const T*> CheckTree(const DirectX::BoundingFrustum& frustum);
};

// Private Functions:

template<typename T>
inline void QuadTree<T>::PrintTree(std::unique_ptr<Node>& nodeToProcess)
{
	if (!nodeToProcess) return;
	// If all nodes, parent and child, contain information to be printed then do that here
	if (nodeToProcess->children[0] == nullptr)
	{
		// If leaf nodes are the only nodes to contain information to be printed then do so here
		std::cout << &nodeToProcess->elements << "\t";
	}
	else // The node is not a leaf node, it has children
	{
		// For each child, call this function with the child node pointer as the argument
		for (int c = 0; c < 4; c++)
		{
			PrintTree(nodeToProcess->children[c]);
		}
	}
}

template<typename T>
inline void QuadTree<T>::AddToNode(const T* elementAddress, const DirectX::BoundingBox& boundingbox, 
	std::unique_ptr<Node>& node)
{
	static const int MAX_DEPTH = 8;
	bool collision = node->boundingRegion.Intersects(boundingbox);

	if (!collision) // No collison found, node and potential children not relevant
		return;

	if (!node->children[0]) // Check if node is leaf here
	{
		if (node->elements.size() < 4) // Node is not full
		{
			// Add the object and volume to the node
			node->elements.push_back(elementAddress);
			return;
		}
		else  // The node is a leaf node, but there is no more room for elements
		{
			// Add child nodes to the node based on this node's covered volume
			// For each of the currently stored elements in this node, attempt 
			// to add them to the new child nodes

			// If passed max_depth, pushing it anyway to avoid infinity-loop
			if (node->depth >= MAX_DEPTH)
			{
				node->elements.push_back(elementAddress); 
				return;
			}

			SubdivideNode(node);

			std::vector<const T*> oldElements = node->elements;
			node->elements.clear();

			for (auto& element : oldElements)
			{
				for (auto& child : node->children)
				{
					AddToNode(element, element->GetBoundingBox(), child);
				}

			}	
			for (auto& child : node->children)
			{
				AddToNode(elementAddress, boundingbox, child);
			}
			return;
		}
	}

	// The current node either was a parent node all along, or it was a full leaf 
	// node and turned into a parent node
	// For each of the child nodes of this node, recursively call this function 
	// with the same element and volume that were received by this function call
	for (auto& child : node->children)
	{
		AddToNode(elementAddress, boundingbox, child);
	}
}

template<typename T>
inline void QuadTree<T>::SubdivideNode(std::unique_ptr<Node>& node)
{
	float width = node->boundingRegion.Extents.x * 0.5f;
	float depth = node->boundingRegion.Extents.z * 0.5f;
	DirectX::XMFLOAT3 center = node->boundingRegion.Center;

	for (size_t i = 0; i < 4; i++)
	{
		node->children[i] = std::make_unique<Node>();
	}

	node->children[0]->boundingRegion = DirectX::BoundingBox(
		{ center.x + width, 0, center.z + width }, { width, 100, depth });
	node->children[1]->boundingRegion = DirectX::BoundingBox(
		{ center.x - width, 0, center.z + width }, { width, 100, depth });
	node->children[2]->boundingRegion = DirectX::BoundingBox(
		{ center.x + width, 0, center.z - width }, { width, 100, depth });
	node->children[3]->boundingRegion = DirectX::BoundingBox(
		{ center.x - width, 0, center.z - width }, { width, 100, depth });

	for (auto& child : node->children)
	{
		child->depth = node->depth + 1;
	}
}

template<typename T>
inline void QuadTree<T>::CheckNode(const DirectX::BoundingFrustum& frustum,
	const std::unique_ptr<Node>& node, std::vector<const T*>& foundObjects)
{
	bool collision = frustum.Intersects(node->boundingRegion);
	if (!collision)	// No collison found, node and potential children not relevant
		return;

	if (node->children[0] == nullptr)	// Check if node is leaf
	{
		// For each element in node
		for (auto& element : node->elements)
		{
			// Check if there is a collision between the frustum and the object's 
			// bounding volume
			collision = frustum.Intersects(element->GetBoundingBox());
			if (collision)
			{
				// Check if the object is already present in the return vector,
				// add if not
				if (std::find(foundObjects.begin(), foundObjects.end(), element) == foundObjects.end())
					foundObjects.push_back(element);
			}
		}
	}
	else
	{
		// Recursively run this function for each of the child nodes of this node
		for (auto& child : node->children)
		{
			CheckNode(frustum, child, foundObjects);
		}
	}
}



// Public: 

template<typename T>
inline QuadTree<T>::QuadTree() 
{ 
	this->root = std::make_unique<Node>(); 
	this->root->boundingRegion = DirectX::BoundingBox({ 0.0f,0.0f,0.0f }, { 100.0f,100.0f,100.0f });
	this->root->depth = 0;
}

template<typename T>
inline void QuadTree<T>::PrintTree()
{
	PrintTree(this->root);
}

template<typename T>
inline void QuadTree<T>::AddElement(const T* elementAddress, const DirectX::BoundingBox& boundingbox)
{
	AddToNode(elementAddress, boundingbox, root);
}

template<typename T>
inline std::vector<const T*> QuadTree<T>::CheckTree(const DirectX::BoundingFrustum& frustum)
{
	std::vector<const T*> toReturn;

	CheckNode(frustum, root, toReturn);

	return toReturn;
}
