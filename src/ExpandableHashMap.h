#ifndef EX
#define EX
// !EX

// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.
#include<list>
#include<vector>
#include<iostream>
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
		KeyType k;
		ValueType v;
	};
	unsigned int getbucket(const KeyType& k)const;
	void cleanup(std::vector<std::list<Node*>> table);
	double m_maxl;
	int m_buc;
	int m_asso;
	std::vector<std::list<Node*>> m_hash;
};
template<typename KeyType, typename ValueType>
unsigned int ExpandableHashMap<KeyType, ValueType>::getbucket(const KeyType& k) const {
	unsigned int hasher(const KeyType & k);
	unsigned int h = hasher(k);
	//use '%' operator to fit all stuff in a limited number of buckets
	int bucket = h % m_buc;
	return bucket;
}
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::cleanup(std::vector<std::list<Node*>> table) {
	//detele everything in the hash table(pointers to nodes)
	for (int i = 0; i < table.size(); i++) {
		for (typename std::list<Node*>::iterator iter = table[i].begin(); iter != table[i].end(); iter++)
			delete* iter;
	}
}
template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
	m_maxl = maximumLoadFactor;
	m_buc = 8;
	m_asso = 0;
	std::vector<std::list<Node*>> a(8);
	//push back the bucket number of lists for later use
	for (int i = 0; i < 8; i++) {
		std::list<Node*> l;
		a.push_back(l);
	}
	m_hash = a;
}
template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
	//simply call cleanup function to delete all pointers to nodes
	cleanup(m_hash);
}
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
	//call cleanup to destroy the old table and create a completely new one, note that there is no copy constructor or assignment operator
	cleanup(m_hash);
	m_buc = 8;
	m_asso = 0;
	std::vector<std::list<Node*>> a(8);
	for (int i = 0; i < 8; i++) {
		std::list<Node*> l;
		a.push_back(l);
	}
	m_hash = a;
}
template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
	return m_asso;  // Delete this line and implement this function correctly
}
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
	int bucket = getbucket(key);
	//if the key already exists, replace its value
	typename std::list<Node*>::iterator iter = (m_hash[bucket]).begin();
	while (iter != m_hash[bucket].end()) {
		if ((*iter)->k == key) {
			(*iter)->v = value;
			break;
		}
		iter++;
	}
	//if the key is new, create a new node with corresponding key and values and add to the list under the bucket
	if (iter == m_hash[bucket].end()) {
		m_asso++;
		//create the new item
		Node* n = new Node();
		n->k = key;
		n->v = value;
		//if loadfactor exceeds the specified maximum, rehash the table
		if (m_asso / static_cast<double>(m_buc) > m_maxl) {
			//create a new vector with double the bucket number and push new lists into it
			m_buc *= 2;
			std::vector<std::list<Node*>> temp(m_buc);
			for (int i = 0; i < m_buc; i++) {
				std::list<Node*> l;
				temp.push_back(l);
			}
			//calculate new bucket number of items in the old table and add them to the new one
			for (int i = 0; i < m_hash.size(); i++) {
				for (typename std::list<Node*>::iterator iter = m_hash[i].begin(); iter != m_hash[i].end(); iter++) {
					int bucket = getbucket((*iter)->k);
					temp[bucket].push_back(*iter);
				}
			}
			//replace the old table with the new table
			m_hash = temp;
		}
		//add the new item to the new table
		int buc = getbucket(key);
		m_hash[buc].push_back(n);
	}
}
template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
	//iterate through the list in the bucket, if found, return a pointer to the value
	int bucket = getbucket(key);
	typename std::list<Node*>::const_iterator iter = m_hash[bucket].begin();
	while (iter != m_hash[bucket].end()) {
		if ((*iter)->k == key)
			return &(*iter)->v;
		iter++;
	}
	//if not found, return a null pointer
	return nullptr;  // Delete this line and implement this function correctly
}

#endif
