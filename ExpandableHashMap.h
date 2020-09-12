// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.
#ifndef EXPANDABLEHASHMAP_H
#define EXPANDABLEHASHMAP_H

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5);
	~ExpandableHashMap();
	void reset();
	int size() const;
	void associate(const KeyType& key, const ValueType& value);

	// for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	// for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	// C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
	struct Node {
		KeyType key;
		ValueType value;
		Node* next;
	};
	int m_size; // Number of Nodes in m_buckets
	int m_numBuckets; // Number of allocated Node pointers in m_buckets
	double m_maximumLoadFactor; // Maximum load factor allowed before rehashing
	Node** m_buckets; // Array of pointers to Nodes, each one holding a linked list

	void rehash(int nNodes); // Rehashes the hasp map with nNodes Nodes
	void deleteAll(Node** map, int n); // Deletes all dynamically-allocated data in the class
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
	m_size = 0; // Default size is 0
	m_numBuckets = 8; // Default number of allocated Node pointers is 8
	m_maximumLoadFactor = maximumLoadFactor;
	m_buckets = new Node * [m_numBuckets];
	// Sets all the node pointers to a default value of nullptr
	for (int i = 0; i < m_numBuckets; i++)
		m_buckets[i] = nullptr;
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
	deleteAll(m_buckets, m_numBuckets);
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
	// Deletes all the nodes, setting everything to the default values
	deleteAll(m_buckets, m_numBuckets);
	m_numBuckets = 8;
	m_size = 0;
	m_buckets = new Node * [m_numBuckets];
	for (int i = 0; i < m_numBuckets; i++)
		m_buckets[i] = nullptr;
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
	return m_size;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
	// Gets the value found at key
	ValueType* val = find(key);

	// If no key-value pair exists, then add a new one
	if (val == nullptr) {
		// If adding a new key-value pair would result in exceeding the maximum load factor,
		// rehash with twice the number of allocated Node pointers
		if ((m_size * 1.0 + 1) / m_numBuckets > m_maximumLoadFactor) {
			rehash(m_numBuckets * 2);
		}
		unsigned int hasher(const KeyType & key);
		unsigned int h = hasher(key);
		// Hash and adjust the key so it fits into one of the linked lists
		Node* b = m_buckets[h % m_numBuckets];

		// Either create a new linked list or add to an existing linked list
		if (b == nullptr) {
			m_buckets[h % m_numBuckets] = new Node;
			b = m_buckets[h % m_numBuckets];
		}
		else {
			while (b->next != nullptr)
				b = b->next;
			b->next = new Node;
			b = b->next;
		}

		b->key = key;
		b->value = value;
		b->next = nullptr;
		m_size++;
	}
	// If key-value pair already exists, alter the value
	else
		*val = value;
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
	// Gets the bucket, then loops through the linked list to find the key
	unsigned int hasher(const KeyType & key);
	unsigned int h = hasher(key);
	Node* b = m_buckets[h % m_numBuckets];
	while (b != nullptr) {
		if (b->key == key)
			return &b->value;
		b = b->next;
	}
	// If not found, returns nullptr
	return nullptr;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::rehash(int nNodes)
{
	// Reassigns member variables to new values
	Node** oldBuckets = m_buckets;
	int oldNumBuckets = m_numBuckets;
	m_buckets = new Node * [nNodes];
	m_numBuckets = nNodes;
	m_size = 0;
	// Sets the new Node pointer array's values to nullptr
	for (int i = 0; i < m_numBuckets; i++)
		m_buckets[i] = nullptr;

	// Loops through the old array, associating all its values into the new array
	for (int i = 0; i < oldNumBuckets; i++) {
		Node* ob = oldBuckets[i];
		while (ob != nullptr) {
			associate(ob->key, ob->value);
			ob = ob->next;
		}
	}

	// Deallocates the old array
	deleteAll(oldBuckets, oldNumBuckets);
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::deleteAll(Node** map, int n)
{
	// Loops through the m_buckets array, deallocating all Nodes, and then the whole array
	for (int i = 0; i < n; i++) {
		Node* b = map[i];
		while (b != nullptr) {
			Node* temp = b->next;
			delete b;
			b = temp;
		}
	}
	delete[] map;
}

#endif